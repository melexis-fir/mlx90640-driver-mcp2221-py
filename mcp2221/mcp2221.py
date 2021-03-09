import os
import ctypes
from mlx90640 import *


def load_driver():
    dev = MLX90640()

    cfp = os.path.dirname(os.path.realpath(__file__))
    machine = 'windows'
    shared_lib_file = 'mlx90640_driver_mcp2221.dll'
    if os.environ.get('OS','') != 'Windows_NT':
        import platform
        machine = platform.machine()
        shared_lib_file = 'libmlx90640_driver_mcp2221.so'

    lib = ctypes.CDLL(os.path.join(cfp, 'libs', machine, shared_lib_file), mode=ctypes.RTLD_GLOBAL)

    # struct MLX90640DriverRegister_t *MLX90640_get_register_mcp2221(void);
    _get_register_mcp2221 = lib.MLX90640_get_register_mcp2221
    _get_register_mcp2221.restype = ctypes.POINTER(ctypes.c_uint16)
    _get_register_mcp2221.argtypes = []
    dev._register_driver(_get_register_mcp2221())
    pass
