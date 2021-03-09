from setuptools import setup
import codecs
import os.path


def read(rel_path):
    here = os.path.abspath(os.path.dirname(__file__))
    with codecs.open(os.path.join(here, rel_path), 'r') as fp:
        return fp.read()


def get_version(rel_path):
    for line in read(rel_path).splitlines():
        if line.startswith('__version__'):
            delim = '"' if '"' in line else "'"
            return line.split(delim)[1]
    else:
        raise RuntimeError("Unable to find version string.")


# Metadata goes in setup.cfg. These are here for GitHub's dependency graph.
setup(
    name="mlx90640-driver-mcp2221",
    version=get_version('mcp2221/__init__.py'),
    install_requires=[
        'mlx90640-driver>=1.1.2'
    ]
)
