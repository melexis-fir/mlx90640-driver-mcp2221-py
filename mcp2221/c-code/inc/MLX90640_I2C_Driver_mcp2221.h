#ifndef __MLX90640_I2C_DRIVER_MCP2221_H__
#define __MLX90640_I2C_DRIVER_MCP2221_H__

#include <stdint.h>

#include "mlx90640_driver_register.h"

#ifdef __cplusplus
extern "C" {
#endif

struct MLX90640DriverRegister_t *MLX90640_get_register_mcp2221(void);

void *MLX90640_get_i2c_handle_mcp2221(void);
void MLX90640_set_i2c_handle_mcp2221(void *handle);

void MLX90640_I2CInit_mcp2221(const char *port);
void MLX90640_I2CClose_mcp2221(void);
int MLX90640_I2CRead_mcp2221(uint8_t slaveAddr, uint16_t startAddr, uint16_t nMemAddressRead, uint16_t *data);
void MLX90640_I2CFreqSet_mcp2221(int freq);
int MLX90640_I2CGeneralReset_mcp2221(void);
int MLX90640_I2CWrite_mcp2221(uint8_t slaveAddr, uint16_t writeAddress, uint16_t data);

#ifdef __cplusplus
}
#endif

#endif // __MLX90640_I2C_DRIVER_MCP2221_H__
