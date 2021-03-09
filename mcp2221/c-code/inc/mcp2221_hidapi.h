#ifndef __MCP2221_HIDAPI_H__
#define __MCP2221_HIDAPI_H__

#define _DEFAULT_SOURCE

#include "hidapi/hidapi.h"
#include <stdint.h>

struct MCP2221_t
{
  uint8_t in_report_[64];
  uint8_t out_report_buffer_[65];
  uint8_t use_pec_;
  uint32_t i2c_frequency_hz_;
  hid_device* hid_;
  uint8_t *out_report_;
};


struct MCP2221_t *mcp2221_hidapi_init_(int16_t index, const char *path, int vid, int pid); // internal
struct MCP2221_t *mcp2221_hidapi_init_by_path(const char *path);
struct MCP2221_t *mcp2221_hidapi_init_by_index(uint8_t index);
struct MCP2221_t *mcp2221_hidapi_init(void);
void mcp2221_hidapi_tear_down(struct MCP2221_t *handle);
int16_t mcp2221_hidapi_reset(struct MCP2221_t *handle);

int16_t mcp2221_hidapi_clear_out_report(struct MCP2221_t *handle);
int16_t mcp2221_hidapi_clear_in_report(struct MCP2221_t *handle);
int16_t mcp2221_hidapi_clear_reports(struct MCP2221_t *handle);
int16_t mcp2221_hidapi_sent_report(struct MCP2221_t *handle);
int16_t mcp2221_hidapi_receive_report(struct MCP2221_t *handle);

int16_t mcp2221_hidapi_read_factory_serial_number(struct MCP2221_t *handle, char *serial, uint8_t max_size);
int16_t mcp2221_hidapi_read_usb_serial_number(struct MCP2221_t *handle, char *serial, uint8_t max_size);


int16_t mcp2221_hidapi_i2c_smb(struct MCP2221_t *handle, uint8_t use_pec);

int16_t mcp2221_hidapi_i2c_test_lines(struct MCP2221_t *handle);
int16_t mcp2221_hidapi_i2c_set_frequency(struct MCP2221_t *handle, uint32_t frequency_hz);
int16_t mcp2221_hidapi_i2c_cancel(struct MCP2221_t *handle);
int16_t mcp2221_hidapi_i2c_state_check(struct MCP2221_t *handle);
int16_t mcp2221_hidapi_i2c_write_(struct MCP2221_t *handle, uint8_t cmd, uint8_t slave_address, const uint8_t *data, uint16_t size); // internal
int16_t mcp2221_hidapi_i2c_write(struct MCP2221_t *handle, uint8_t slave_address, const uint8_t *data, uint16_t size);
int16_t mcp2221_hidapi_i2c_write_repeated(struct MCP2221_t *handle, uint8_t slave_address, const uint8_t *data, uint16_t size);
int16_t mcp2221_hidapi_i2c_write_no_stop(struct MCP2221_t *handle, uint8_t slave_address, const uint8_t *data, uint16_t size);

int16_t mcp2221_hidapi_i2c_read_(struct MCP2221_t *handle, uint8_t cmd, uint8_t slave_address, uint8_t *data, uint16_t size); // internal
int16_t mcp2221_hidapi_i2c_read(struct MCP2221_t *handle, uint8_t slave_address, uint8_t *data, uint16_t size);
int16_t mcp2221_hidapi_i2c_read_repeated(struct MCP2221_t *handle, uint8_t slave_address, uint8_t *data, uint16_t size);

int16_t mcp2221_hidapi_i2c_slave_available(struct MCP2221_t *handle, uint8_t slave_address);
int16_t mcp2221_hidapi_i2c_sent_general_reset(struct MCP2221_t *handle);

int16_t mcp2221_hidapi_i2c_memory_write(struct MCP2221_t *handle, uint8_t slave_address, uint16_t memory_address, uint16_t data, uint16_t delay_ms);
int16_t mcp2221_hidapi_i2c_memory_read(struct MCP2221_t *handle, uint8_t slave_address, uint16_t memory_address, uint8_t *data, uint16_t size);

int16_t mcp2221_hidapi_i2c_memory_read_uint16(struct MCP2221_t *handle, uint8_t slave_address, uint16_t memory_address, uint16_t *data, uint16_t size);

int16_t mcp2221_hidapi_i2c_write_byte(struct MCP2221_t *handle, uint8_t slave_address, uint8_t data);
int16_t mcp2221_hidapi_i2c_write_word(struct MCP2221_t *handle, uint8_t slave_address, uint16_t data);
int16_t mcp2221_hidapi_i2c_read_byte(struct MCP2221_t *handle, uint8_t slave_address, uint8_t *data);
int16_t mcp2221_hidapi_i2c_read_word(struct MCP2221_t *handle, uint8_t slave_address, uint16_t *data);


#endif // __MCP2221_HIDAPI_H__
