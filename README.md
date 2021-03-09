# MLX90640 driver for MCP2221 USB I2C hub

[![GitHub release (latest by date)](https://img.shields.io/github/v/release/melexis-fir/mlx90640-driver-mcp2221-py?label=github-latest-release-tag)](https://github.com/melexis-fir/mlx90640-driver-mcp2221-py/releases) ![Lines of code](https://img.shields.io/tokei/lines/github/melexis-fir/mlx90640-driver-mcp2221-py)  

[![PyPI](https://img.shields.io/pypi/v/mlx90640-driver-mcp2221)](https://pypi.org/project/mlx90640-driver-mcp2221) ![PyPI - Python Version](https://img.shields.io/pypi/pyversions/mlx90640-driver-mcp2221) ![PyPI - License](https://img.shields.io/pypi/l/mlx90640-driver-mcp2221)  

![platform](https://img.shields.io/badge/platform-Win10%20PC%20%7C%20linux%20PC%20%7C%20rasberry%20pi%204%20%7C%20Jetson%20Nano%20%7C%20beagle%20bone-lightgrey)  

MLX90640 is a thermal camera (32x24 pixels) using Far InfraRed radiation from objects to measure the object temperature.  
https://www.melexis.com/mlx90640  
The python package "[mlx90640-driver](https://github.com/melexis-fir/mlx90640-driver-py)" driver interfaces the MLX90640 and aims to facilitate rapid prototyping.

This package provide the I2C low level routines.
It uses the I2C hub from MCP2221 chip which is connected via the USB cable to the computer.  
https://www.microchip.com/wwwproducts/en/mcp2221  
https://www.adafruit.com/product/4471  

## Getting started

### Installation


```bash
pip install mlx90640-driver-mcp2221
```

https://pypi.org/project/mlx90640-driver-mcp2221  
https://pypistats.org/packages/mlx90640-driver-mcp2221

#### Extra installation for linux based OS.

1. install udev, libusb and libhidapi:
```sh
sudo apt update
sudo apt-get install libudev-dev libusb-1.0-0-dev libhidapi-dev
```

2. Configure such that non-root users have access.

Place a file in `/etc/udev/rules.d/20-microchip.rules`:

```cfg
KERNEL=="hidraw*", ATTRS{idVendor}=="04d8", MODE="0666"
```

You might use this command line to create that file:
```sh
echo 'KERNEL=="hidraw*", ATTRS{idVendor}=="04d8", MODE="0666"' | sudo tee /etc/udev/rules.d/20-microchip.rules >/dev/null
```

__Note:__
Make sure to (re-)plug the MCP2221 after this file is written!


### Running the driver demo

* Connect the MLX90640 to the MCP2221 I2C port
* Connect the MCP2221 to your PC with the USB cable.  
* Open a terminal and run following command:  

```bash
mlx90640-mcp2221-dump mcp://mcp:2221/0
```

This program takes 1 optional argument.

```bash
mlx90640-mcp2221-dump <communication-port>
```

Note: this dump command is not yet available!
