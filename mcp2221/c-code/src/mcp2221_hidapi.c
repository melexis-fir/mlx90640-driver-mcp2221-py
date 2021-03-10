#include "mcp2221_hidapi.h"
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include <sys/time.h>

//
// Yet another lib for MCP2221 https://www.microchip.com/wwwproducts/en/MCP2221A
//
// Inspired by:
// - https://github.com/nonNoise/PyMCP2221A
// - https://github.com/zkemble/libmcp2221
// - https://elixir.bootlin.com/linux/latest/source/drivers/hid/hid-mcp2221.c
//


/* Response codes in a raw input report */
enum {
  MCP2221_SUCCESS = 0x00,
  MCP2221_I2C_ENG_BUSY = 0x01,
  MCP2221_I2C_START_TOUT = 0x12,
  MCP2221_I2C_STOP_TOUT = 0x62,
  MCP2221_I2C_WRADDRL_TOUT = 0x23,
  MCP2221_I2C_WRDATA_TOUT = 0x44,
  MCP2221_I2C_WRADDRL_NACK = 0x25,
  MCP2221_I2C_MASK_ADDR_NACK = 0x40,
  MCP2221_I2C_WRADDRL_SEND = 0x21,
  MCP2221_I2C_ADDR_NACK = 0x25,
  MCP2221_I2C_READ_COMPL = 0x55,
  MCP2221_ALT_F_NOT_GPIOV = 0xEE,
  MCP2221_ALT_F_NOT_GPIOD = 0xEF,
};


static uint64_t
get_time_in_us(void)
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return (uint64_t)1000000*tv.tv_sec + tv.tv_usec;
}


void
mcp2221_hidapi_print_in_report(struct MCP2221_t *handle)
{
  printf ("\nin_report_:\n");
  for (int i = 0; i < 16; i++)
  {
    printf ("%02X, ", handle->in_report_[i]);
  }
  printf("\n");
  for (int i = 16; i < 32; i++)
  {
    printf ("%02X, ", handle->in_report_[i]);
  }
  printf("\n");
  for (int i = 32; i < 48; i++)
  {
    printf ("%02X, ", handle->in_report_[i]);
  }
  printf("\n");
  for (int i = 48; i < 64; i++)
  {
    printf ("%02X, ", handle->in_report_[i]);
  }
  printf("\n");

}


struct MCP2221_t *
mcp2221_hidapi_init_(int16_t index, const char *path,  int vid, int pid)
{
  if ((path == NULL) && (index < 0))
  {
    printf ("mcp2221_hidapi_init: ERROR, specify at least index or path!\n");
    return NULL;
  }

  struct hid_device_info* all_devices = hid_enumerate(vid, pid);

  if (all_devices == NULL)
  {
    printf ("mcp2221_hidapi_init: ERROR, No MCP2221(A) found!\n");
    return NULL;
  }

  int i = 0;
  for (struct hid_device_info *cur_dev = all_devices; cur_dev != NULL; cur_dev = cur_dev->next)
  {
    printf("dev[%i](%04X %04X) => '%s'\n", i, cur_dev->vendor_id, cur_dev->product_id, cur_dev->path);
    i++;
  }    

  struct MCP2221_t *handle = malloc(sizeof(struct MCP2221_t));
  static int8_t is_init = 0;
  memset(handle, 0, sizeof (struct MCP2221_t));
  handle->i2c_frequency_hz_ = 100000; // defaulting to 100kHz.

  if (!is_init)
  {
    if (hid_init() < 0)
    {
      printf("mcp2221_hidapi_init: ERROR, HIDAPI Init failed\n");
      return NULL;
    }
    is_init = 1;
  }

  if (index >= 0)
  {
    int i = 0;
    for (struct hid_device_info *cur_dev = all_devices; cur_dev != NULL; cur_dev = cur_dev->next, i++)
    {
      if (i == index)
      {
        handle->hid_ = hid_open_path(cur_dev->path);
        hid_free_enumeration(all_devices);

        if (handle->hid_ == NULL)
        {
          printf ("mcp2221_hidapi_init: ERROR, no handle!\n");
          free(handle);
          return NULL;
        }
        mcp2221_hidapi_i2c_cancel(handle);
        mcp2221_hidapi_i2c_set_frequency(handle, handle->i2c_frequency_hz_);
        mcp2221_hidapi_i2c_test_lines(handle);
        if (handle->in_report_[8] != 0)
        {
          printf ("mcp2221_hidapi_init: ERROR: internal I2C state error => reset!\n");
          mcp2221_hidapi_reset(handle);
          return NULL;
        }
        return handle;
      }
    }

    printf ("mcp2221_hidapi_init: ERROR, index not found!\n");
    hid_free_enumeration(all_devices);
    free(handle);
    return NULL;
  }

  // index is <0, so path must be specified; null pointer is checked above.
  for (struct hid_device_info *cur_dev = all_devices; cur_dev != NULL; cur_dev = cur_dev->next)
  {
    if (!strcmp(cur_dev->path, path))
    {
      handle->hid_ = hid_open_path(cur_dev->path);
      hid_free_enumeration(all_devices);

      if (handle->hid_ == NULL)
      {
        printf ("mcp2221_hidapi_init: ERROR, no handle!\n");
        free(handle);
        return NULL;
      }

      mcp2221_hidapi_i2c_test_lines(handle);
      if (handle->in_report_[8] != 0)
      {
        mcp2221_hidapi_reset(handle);
        return NULL;
      }
      mcp2221_hidapi_i2c_set_frequency(handle, handle->i2c_frequency_hz_);
      return handle;        
    }
  }
  printf ("mcp2221_hidapi_init: ERROR, path not found!\n");
  hid_free_enumeration(all_devices);
  free(handle);
  return NULL;
}


struct MCP2221_t *
mcp2221_hidapi_init_by_path(const char *path)
{
  return mcp2221_hidapi_init_(-1, path, 0x04d8, 0x00dd);
}


struct MCP2221_t *
mcp2221_hidapi_init_by_index(uint8_t index)
{
  return mcp2221_hidapi_init_(index, NULL, 0x04d8, 0x00dd);
}


struct MCP2221_t *
mcp2221_hidapi_init()
{
  return mcp2221_hidapi_init_(0, NULL, 0x04d8, 0x00dd);
}


void
mcp2221_hidapi_tear_down(struct MCP2221_t *handle)
{
  if (handle != NULL)
  {
    if (handle->hid_ != NULL)
    {
      hid_close(handle->hid_);
    }
    free(handle);
  }
  return;
}



int16_t
mcp2221_hidapi_i2c_smb(struct MCP2221_t *handle, uint8_t use_pec)
{
 if (handle == NULL)
  {
    printf("%s: ERROR: No handle\n", __func__);
    return -9999;
  }
  handle->use_pec_ = use_pec;
  return 0;
}


int16_t
mcp2221_hidapi_i2c_test_lines(struct MCP2221_t *handle)
{
  if (handle == NULL)
  {
    printf("%s: ERROR: No handle\n", __func__);
    return -9999;
  }
  mcp2221_hidapi_clear_reports(handle);
  handle->out_report_[0] = 0x10; // CMD Status / Set param
  mcp2221_hidapi_sent_report(handle);
  mcp2221_hidapi_receive_report(handle);

  if (handle->in_report_[23] != 1)
  {
    printf("mcp2221_hidapi_i2c_test_lines: SCL is stuck to low!\n");
    return -1;
  }
  if (handle->in_report_[22] != 1)
  {
    printf("mcp2221_hidapi_i2c_test_lines: SDA is stuck to low!\n");
    return -2;
  }

  return 0;
}


static int mcp_get_i2c_engine_state(struct MCP2221_t *handle, uint8_t idx)
{
  int ret;

  switch (handle->in_report_[idx])
  {
    case MCP2221_I2C_WRADDRL_NACK:
    case MCP2221_I2C_WRADDRL_SEND:
      ret = -ENXIO;
      break;
    case MCP2221_I2C_START_TOUT:
    case MCP2221_I2C_STOP_TOUT:
    case MCP2221_I2C_WRADDRL_TOUT:
    case MCP2221_I2C_WRDATA_TOUT:
      ret = -ETIMEDOUT;
      break;
    case MCP2221_I2C_ENG_BUSY:
      ret = -EAGAIN;
      break;
    case MCP2221_SUCCESS:
      ret = 0x00;
      break;
    default:
      ret = -EIO;
  }

  return ret;
}


int16_t
mcp2221_hidapi_i2c_set_frequency(struct MCP2221_t *handle, uint32_t frequency_hz)
{
  if (handle == NULL)
  {
    printf("%s: ERROR: No handle\n", __func__);
    return -9999;
  }
  if (frequency_hz == 0) // use 0 to resend last used frequency!
  {
    frequency_hz = handle->i2c_frequency_hz_;
  }
  mcp2221_hidapi_clear_reports(handle);
  handle->out_report_[0] = 0x10; // CMD Status / Set param
  handle->out_report_[1] = 0x00; // don't care
  handle->out_report_[2] = 0x00; // 0x10 ==> cancel current transaction
  handle->out_report_[3] = 0x20; // when 0x20 ==> next is clock divider
  handle->out_report_[4] = (12000000 / frequency_hz) - 3; // clock speed.
  mcp2221_hidapi_sent_report(handle);
  mcp2221_hidapi_receive_report(handle);

  uint8_t state = mcp_get_i2c_engine_state(handle, 8);
  if (state != MCP2221_SUCCESS)
  {
    usleep(1000);    /* Small delay is needed here */
    mcp2221_hidapi_i2c_cancel(handle);
  }

  handle->i2c_frequency_hz_ = frequency_hz;

  return 0;
}


int16_t
mcp2221_hidapi_i2c_cancel(struct MCP2221_t *handle)
{
  if (handle == NULL)
  {
    printf("%s: ERROR: No handle\n", __func__);
    return -9999;
  }
  mcp2221_hidapi_clear_reports(handle);
  handle->out_report_[0] = 0x10; // CMD Status / Set param
  mcp2221_hidapi_sent_report(handle);
  mcp2221_hidapi_receive_report(handle);

  if (handle->in_report_[8] != MCP2221_SUCCESS)
  {
    mcp2221_hidapi_clear_reports(handle);
    handle->out_report_[0] = 0x10; // CMD Status / Set param
    handle->out_report_[2] = 0x10; // 0x10 ==> cancel current transaction
    mcp2221_hidapi_sent_report(handle);
    return mcp2221_hidapi_receive_report(handle);
  }
  return 0;
}


int16_t
mcp2221_hidapi_reset(struct MCP2221_t *handle)
{
  if (handle == NULL)
  {
    printf("%s: ERROR: No handle\n", __func__);
    return -9999;
  }

  mcp2221_hidapi_clear_reports(handle);
  handle->out_report_[0] = 0x70;
  handle->out_report_[1] = 0xAB;
  handle->out_report_[2] = 0xCD;
  handle->out_report_[3] = 0xEF;
  mcp2221_hidapi_sent_report(handle);

  usleep(1000000);
  // no answer is expected!

  mcp2221_hidapi_tear_down(handle); // re-enumeration is needed!

  return 0;
}


int16_t
mcp2221_hidapi_i2c_state_check(struct MCP2221_t *handle)
{
  if (handle == NULL)
  {
    printf("%s: ERROR: No handle\n", __func__);
    return -9999;
  }

  mcp2221_hidapi_clear_reports(handle);
  handle->out_report_[0] = 0x10; // CMD Status / Set param
  mcp2221_hidapi_sent_report(handle);
  mcp2221_hidapi_receive_report(handle);

  return handle->in_report_[8];
}


int16_t
mcp2221_hidapi_i2c_write_(struct MCP2221_t *handle, uint8_t cmd, uint8_t slave_address, const uint8_t *data, uint16_t size)
{
  if (handle == NULL)
  {
    printf("%s: ERROR: No handle\n", __func__);
    return -9999;
  }
  mcp2221_hidapi_clear_reports(handle);
  handle->out_report_[0] = cmd;
  handle->out_report_[1] = size & 0x00FF;
  handle->out_report_[2] = (size>>8) & 0x00FF;
  handle->out_report_[3] = (slave_address << 1);
  if (size > 60) size = 60;
  for (uint8_t i=0; i<size; i++)
  {
    handle->out_report_[4+i] = data[i];
  }
  mcp2221_hidapi_sent_report(handle);
  return mcp2221_hidapi_receive_report(handle);
}


int16_t
mcp2221_hidapi_i2c_write(struct MCP2221_t *handle, uint8_t slave_address, const uint8_t *data, uint16_t size)
{
  return mcp2221_hidapi_i2c_write_(handle, 0x90, slave_address, data, size);
}


int16_t
mcp2221_hidapi_i2c_write_repeated(struct MCP2221_t *handle, uint8_t slave_address, const uint8_t *data, uint16_t size)
{
  return mcp2221_hidapi_i2c_write_(handle, 0x92, slave_address, data, size);
}


int16_t
mcp2221_hidapi_i2c_write_no_stop(struct MCP2221_t *handle, uint8_t slave_address, const uint8_t *data, uint16_t size)
{
  return mcp2221_hidapi_i2c_write_(handle, 0x94, slave_address, data, size);
}


int16_t
mcp2221_hidapi_i2c_read_(struct MCP2221_t *handle, uint8_t cmd, uint8_t slave_address, uint8_t *data, uint16_t size)
{
  if (handle == NULL)
  {
    printf("%s: ERROR: No handle\n", __func__);
    return -9999;
  }

  mcp2221_hidapi_clear_reports(handle);
  handle->out_report_[0] = cmd;
  handle->out_report_[1] = size & 0x00FF;
  handle->out_report_[2] = (size>>8) & 0x00FF;
  handle->out_report_[3] = (slave_address << 1) | 0x01;
  mcp2221_hidapi_sent_report(handle);
  mcp2221_hidapi_receive_report(handle);

  if (handle->in_report_[1] != 0x00)
  { // no ACK => cancel current operation...
    mcp2221_hidapi_i2c_cancel(handle);
    return -1;
  }

  for (int chunk_i = 0; chunk_i * 60 < size; chunk_i++)
  {
    int16_t chunk_size = size;
    chunk_size -= chunk_i * 60;
    if (chunk_size > 60) chunk_size = 60;
    if (chunk_size <= 0) break;

    usleep(1100);
    // uint64_t t_start = get_time_in_us();
    // while ((get_time_in_us() - t_start) < 100000)
    // {
    //   if (mcp2221_hidapi_i2c_state_check(handle) == 0x55)
    //   { // data is available!
    //     break;
    //   }
    // }

    mcp2221_hidapi_clear_reports(handle);
    handle->out_report_[0] = 0x40;
    mcp2221_hidapi_sent_report(handle);
    mcp2221_hidapi_receive_report(handle);

    if (handle->in_report_[1] != 0)
    {
      mcp2221_hidapi_i2c_cancel(handle);
      return -1;
    }

    if (handle->in_report_[3] != chunk_size) // more than 60 bytes can be read in sequence...
    {
      mcp2221_hidapi_i2c_cancel(handle);
      return -2;
    }

    for (int i=0; i<chunk_size; i++)
    {
      data[(chunk_i * 60) + i] = handle->in_report_[i+4];
    }
    if (mcp2221_hidapi_i2c_state_check(handle) == 0x00)
    {
      return 0;
    }
  }
  return 0;
}


int16_t
mcp2221_hidapi_i2c_read(struct MCP2221_t *handle, uint8_t slave_address, uint8_t *data, uint16_t size)
{
  return mcp2221_hidapi_i2c_read_(handle, 0x91, slave_address, data, size);
}


int16_t
mcp2221_hidapi_i2c_read_repeated(struct MCP2221_t *handle, uint8_t slave_address, uint8_t *data, uint16_t size)
{
  return mcp2221_hidapi_i2c_read_(handle, 0x93, slave_address, data, size);
}


int16_t
mcp2221_hidapi_i2c_slave_available(struct MCP2221_t *handle, uint8_t slave_address)
{
  uint8_t data[2];

  mcp2221_hidapi_i2c_write(handle, slave_address, data, 0);
  if (mcp2221_hidapi_i2c_state_check(handle) != 0x00)
  {
    mcp2221_hidapi_i2c_cancel(handle);
    return -1; // not available
  } 
  return 0; // available!
}


int16_t
mcp2221_hidapi_i2c_sent_general_reset(struct MCP2221_t *handle)
{ // See I2C Spec => 3.1.14 Software reset
  // http://www.nxp.com/documents/user_manual/UM10204.pdf
  uint8_t data[1];
  data[0] = 0x06;
  uint8_t slave_address = 0x00;
  return mcp2221_hidapi_i2c_write(handle, slave_address, data, 1);
}


int16_t
mcp2221_hidapi_i2c_memory_write(struct MCP2221_t *handle, uint8_t slave_address, uint16_t memory_address, uint16_t data, uint16_t delay_ms)
{
  uint8_t buf[4];
  memset(buf, 0, sizeof(buf));
  buf[0] = (memory_address >> 8) & 0xFF;
  buf[1] = memory_address & 0xFF;
  buf[2] = (data >> 8) & 0xFF;
  buf[3] = data & 0xFF;

  mcp2221_hidapi_i2c_set_frequency(handle, 0);
  int16_t r = mcp2221_hidapi_i2c_write(handle, slave_address, buf, 4);
  if (delay_ms > 0)
  {
    usleep(delay_ms*1000);
  }
  return r;
}


int16_t
mcp2221_hidapi_i2c_memory_read(struct MCP2221_t *handle, uint8_t slave_address, uint16_t memory_address, uint8_t *data, uint16_t size)
{
  uint8_t buf[2];
  memset(buf, 0, sizeof(buf));  
  buf[0] = (memory_address >> 8) & 0xFF;
  buf[1] = memory_address & 0xFF;

  mcp2221_hidapi_i2c_set_frequency(handle, 0);
  int16_t r = mcp2221_hidapi_i2c_write_no_stop(handle, slave_address, buf, 2);
  if (r != 0)
  {
    mcp2221_hidapi_i2c_cancel(handle);
    return r;
  }
  return mcp2221_hidapi_i2c_read_repeated(handle, slave_address, data, size);
}


int16_t
mcp2221_hidapi_i2c_memory_read_uint16(struct MCP2221_t *handle, uint8_t slave_address, uint16_t memory_address, uint16_t *data, uint16_t size)
{
  uint8_t *data8 = (uint8_t *)data;

  mcp2221_hidapi_i2c_set_frequency(handle, 0);
  int16_t r = mcp2221_hidapi_i2c_memory_read(handle, slave_address, memory_address, data8, size*2);
  for (uint16_t i=0; i<size; i++)
  { // swap bytes: 
    data[i] = (256 * data8[i*2 + 0] + data8[i*2 + 1]);
  }
  return r;
}


int16_t
mcp2221_hidapi_i2c_write_byte(struct MCP2221_t *handle, uint8_t slave_address, uint8_t data)
{
  uint8_t buf[1];
  buf[0] = data;

  mcp2221_hidapi_i2c_set_frequency(handle, 0);
  return mcp2221_hidapi_i2c_write(handle, slave_address, buf, 1);
}


int16_t
mcp2221_hidapi_i2c_write_word(struct MCP2221_t *handle, uint8_t slave_address, uint16_t data)
{
  uint8_t buf[2];
  buf[0] = data & 0xFF;
  buf[1] = (data >> 8) & 0xFF;

  mcp2221_hidapi_i2c_set_frequency(handle, 0);
  return mcp2221_hidapi_i2c_write(handle, slave_address, buf, 2);
}


int16_t
mcp2221_hidapi_i2c_read_byte(struct MCP2221_t *handle, uint8_t slave_address, uint8_t *data)
{
  mcp2221_hidapi_i2c_set_frequency(handle, 0);
  return mcp2221_hidapi_i2c_read(handle, slave_address, data, 1);
}


int16_t
mcp2221_hidapi_i2c_read_word(struct MCP2221_t *handle, uint8_t slave_address, uint16_t *data)
{
  uint8_t *data8 = (uint8_t *)data;

  mcp2221_hidapi_i2c_set_frequency(handle, 0);
  int16_t r = mcp2221_hidapi_i2c_read(handle, slave_address, data8, 2);

  // swap bytes: 
  *data = (256 * data8[0] + data8[1]);
  return r;
}


int16_t
mcp2221_hidapi_clear_out_report(struct MCP2221_t *handle)
{
  if (handle == NULL)
  {
    printf("%s: ERROR: No handle\n", __func__);
    return -9999;
  }
  memset(handle->out_report_buffer_, 0, sizeof(handle->out_report_buffer_));
  handle->out_report_ = &handle->out_report_buffer_[1];
  return 0;
}


int16_t
mcp2221_hidapi_clear_in_report(struct MCP2221_t *handle)
{
  if (handle == NULL)
  {
    printf("%s: ERROR: No handle\n", __func__);
    return -9999;
  }
  memset(handle->in_report_, 0, sizeof(handle->in_report_));
  return 0;
}


int16_t
mcp2221_hidapi_clear_reports(struct MCP2221_t *handle)
{
  int16_t r = mcp2221_hidapi_clear_out_report(handle);
  if (r != 0) return r;
  r = mcp2221_hidapi_clear_in_report(handle);
  return r;
}


int16_t
mcp2221_hidapi_sent_report(struct MCP2221_t *handle)
{
  int len = hid_write(handle->hid_, handle->out_report_buffer_, 65);
  if (len != 65)
  {
    printf("%s: ERROR, sending result\n", __func__);
    return -1;
  }
  return 0;
}


int16_t
mcp2221_hidapi_receive_report(struct MCP2221_t *handle)
{
  int len = hid_read(handle->hid_, handle->in_report_, 64);
  if (len != 64)
  {
    printf("%s: ERROR, reading result\n", __func__);
    return -1;
  }
  return 0;
}


int16_t
mcp2221_hidapi_read_factory_serial_number(struct MCP2221_t *handle, char *serial, uint8_t max_size)
{
  if (handle == NULL)
  {
    printf("%s: ERROR: No handle\n", __func__);
    return -9999;
  }
  mcp2221_hidapi_clear_reports(handle);
  handle->out_report_[0] = 0xB0; // CMD Status / Set param
  handle->out_report_[1] = 0x05;
  mcp2221_hidapi_sent_report(handle);
  mcp2221_hidapi_receive_report(handle);

  int len = handle->in_report_[2];
  for (int i=0; i<len; i++)
  {
    if (i >= max_size)
    {
      return -1;
    }
    serial[i] = handle->in_report_[4+2*i];
  }

  return 0;
}


int16_t
mcp2221_hidapi_read_usb_serial_number(struct MCP2221_t *handle, char *serial, uint8_t max_size)
{
  if (handle == NULL)
  {
    printf("%s: ERROR: No handle\n", __func__);
    return -9999;
  }
  mcp2221_hidapi_clear_reports(handle);
  handle->out_report_[0] = 0xB0; // CMD Status / Set param
  handle->out_report_[1] = 0x04;
  mcp2221_hidapi_sent_report(handle);
  mcp2221_hidapi_receive_report(handle);

  int len = handle->in_report_[2];
  for (int i=0; i<len; i++)
  {
    if (i >= max_size)
    {
      return -1;
    }
    serial[i] = handle->in_report_[4+2*i];
  }

  return 0;
}
