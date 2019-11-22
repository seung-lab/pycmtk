import os
from shutil import copyfile
from setuptools import setup, Distribution
import subprocess
import sys

if not os.path.isdir('pycmtk'):
    os.mkdir('pycmtk')

# c++ -O3 -Wall -shared -std=c++11 -fPIC `python3 -m pybind11 --includes` example.cpp -o example`python3-config --extension-suffix`

cmd = 'python{}.{}-config --extension-suffix'.format(sys.version_info.major, sys.version_info.minor)

extension = str(subprocess.check_output(cmd, shell=True))[2:-3]
sharedlib = 'pycmtk' + extension

destlib = 'pycmtk/' + sharedlib

copyfile('build/bin/' + sharedlib, destlib)

class BinaryDistribution(Distribution):
    def has_ext_modules(foo):
        return True

setup(
    name='pycmtk',
    version='0.1',
    description='Unofficial Python bindings for the Computational Morphology Toolkit (CMTK) (https://www.nitrc.org/projects/cmtk)',
    packages=['pycmtk'],
    package_data={
        'pycmtk': [ destlib ],
    },
    distclass=BinaryDistribution,
    license = "GPLv3+",
    author="William Silversmith",
    author_email="ws9@princeton.edu",
    keywords = "volumetric-data numpy connectomics image-processing biomedical-image-processing skeleton skeletons registration",
    url = "https://github.com/seung-lab/pycmtk/",
    classifiers=[
        "Intended Audience :: Developers",
        "Development Status :: 4 - Beta",
        "License :: OSI Approved :: BSD License",
        "Programming Language :: Python",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.5",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Topic :: Scientific/Engineering",
        "Intended Audience :: Science/Research",
        "Operating System :: POSIX",
        "Operating System :: MacOS",
        "Topic :: Utilities",
    ],
)