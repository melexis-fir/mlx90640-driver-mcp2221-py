#ifndef __MLX90640_DRIVER_REGISTER_H__
#define __MLX90640_DRIVER_REGISTER_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


struct MLX90640DriverRegister_t
{
  char name_[64];
  void *(*MLX90640_get_i2c_handle_) (void);
  void (*MLX90640_set_i2c_handle_) (void *handle);

  void (*MLX90640_I2CInit_) (const char *port);
  void (*MLX90640_I2CClose_) (void);
  int  (*MLX90640_I2CRead_) (uint8_t slaveAddr, uint16_t startAddr, uint16_t nMemAddressRead, uint16_t *data);
  void (*MLX90640_I2CFreqSet_) (int freq);
  int  (*MLX90640_I2CGeneralReset_) (void);
  int  (*MLX90640_I2CWrite_) (uint8_t slaveAddr, uint16_t writeAddress, uint16_t data);
};

void mlx90640_register_driver(struct MLX90640DriverRegister_t *driver);
struct MLX90640DriverRegister_t *mlx90640_get_driver(const char *name);
struct MLX90640DriverRegister_t *mlx90640_get_active_driver(void);
int mlx90640_activate_driver(const char *name);

#ifdef __cplusplus
}
#endif

#endif // __MLX90640_DRIVER_REGISTER_H__
