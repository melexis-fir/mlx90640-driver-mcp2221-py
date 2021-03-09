#include <stdio.h>
#include <string.h>

#include "mcp2221_hidapi.h"
#include <stdlib.h>

#include "MLX90640_I2C_Driver_mcp2221.h"


static struct MCP2221_t *g_handle;


struct MLX90640DriverRegister_t *
MLX90640_get_register_mcp2221(void)
{
  static struct MLX90640DriverRegister_t reg;

  strcpy(reg.name_, "mcp://mcp:2221/");
  reg.MLX90640_get_i2c_handle_  = MLX90640_get_i2c_handle_mcp2221;
  reg.MLX90640_set_i2c_handle_  = MLX90640_set_i2c_handle_mcp2221;
  reg.MLX90640_I2CInit_         = MLX90640_I2CInit_mcp2221;
  reg.MLX90640_I2CClose_        = MLX90640_I2CClose_mcp2221;
  reg.MLX90640_I2CRead_         = MLX90640_I2CRead_mcp2221;
  reg.MLX90640_I2CFreqSet_      = MLX90640_I2CFreqSet_mcp2221;
  reg.MLX90640_I2CGeneralReset_ = MLX90640_I2CGeneralReset_mcp2221;
  reg.MLX90640_I2CWrite_        = MLX90640_I2CWrite_mcp2221;
  return &reg;
}


void *
MLX90640_get_i2c_handle_mcp2221(void)
{
  return (void *)g_handle;
}


void
MLX90640_set_i2c_handle_mcp2221(void *handle)
{
  g_handle = (struct MCP2221_t *)handle;
}


void MLX90640_I2CInit_mcp2221(const char *port) 
{
  const char *start = "mcp://mcp:2221/";
  if (strncmp(port, start, strlen(start)) != 0)
  {
    printf("ERROR: '%s' is not a valid port\n", port);
    return;
  }

  int index = atoi(&port[strlen(start)]);
  g_handle = mcp2221_hidapi_init_by_index(index);
  if (g_handle == NULL)
  {
    printf("MLX90640 MCP2221 ERROR: not able to open USB link\n");
    exit(1);
  }
  mcp2221_hidapi_i2c_set_frequency(g_handle, 400000);
}


void MLX90640_I2CClose_mcp2221(void)
{
  mcp2221_hidapi_tear_down(g_handle);
  g_handle = NULL;
}


int MLX90640_I2CRead_mcp2221(uint8_t slaveAddr, uint16_t startAddr, uint16_t nMemAddressRead, uint16_t *data)
{
  return mcp2221_hidapi_i2c_memory_read_uint16(g_handle, slaveAddr, startAddr, data, nMemAddressRead);
}


void MLX90640_I2CFreqSet_mcp2221(int freq)
{
  mcp2221_hidapi_i2c_set_frequency(g_handle, freq);
}


int MLX90640_I2CGeneralReset_mcp2221(void)
{
  return mcp2221_hidapi_i2c_sent_general_reset(g_handle);
}


int MLX90640_I2CWrite_mcp2221(uint8_t slaveAddr, uint16_t writeAddress, uint16_t data)
{
  int delay_ms = 0;
  if ((writeAddress & 0xFF00) == 0x2400) delay_ms = 10; // 10ms for EEPROM write!
  return mcp2221_hidapi_i2c_memory_write(g_handle, slaveAddr, writeAddress, data, delay_ms);
}
