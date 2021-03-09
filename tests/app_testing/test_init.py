import pytest
import mlx90640_mcp2221
import mlx90640


order=0000
dev = None


@pytest.mark.run(order=order+0)
def test_version():
    print ('mlx90640-driver version: {}'.format(mlx90640.__version__))
    print ('mlx90640-driver-mcp2221 version: {}'.format(mlx90640_mcp2221.__version__))


@pytest.mark.run(order=order+1)
def test_class_declared():
    global dev
    dev = mlx90640.MLX90640()


@pytest.mark.run(order=order+2)
def test_i2c_init(metadata):
    global dev
    
    # make sure we have the metadata variable set up!
    assert 'i2c_port' in metadata

    i2c_port = metadata['i2c_port']
    if i2c_port == 'auto':
        i2c_port = "mcp://mcp:2221/0"

    r = dev.i2c_init(i2c_port)
    assert r == None


@pytest.mark.run(order=order+3)
def test_refresh_rate_setting():
    global dev

    assert dev.set_refresh_rate(1) == 0
    assert dev.get_refresh_rate() == 1

    assert dev.set_refresh_rate(4) == 0
    assert dev.get_refresh_rate() == 4


@pytest.mark.run(order=order+4)
def test_dump_frame():
    global dev

    dev.dump_eeprom()
    dev.extract_parameters()

    for i in range(0, 5):
        dev.get_frame_data()
        ta = dev.get_ta() - 8.0
        emissivity = 1

        to = dev.calculate_to(emissivity, ta)

    print("{:02d}: {}".format(i, ','.join(format(x, ".2f") for x in to)))

    assert 10 <= ta <= 40

    n = len(to)
    assert n == (32*24)

    average_to = 0
    for t in to:
    	average_to += t
    average_to /= n
    assert 10 <= average_to <= 40
