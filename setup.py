import os
from shutil import copyfile
from setuptools import setup, Distribution

if not os.path.isdir('pycmtk'):
    os.mkdir('pycmtk')

copyfile('build/bin/pycmtk.cpython-36m-x86_64-linux-gnu.so', 'pycmtk/pycmtk.cpython-36m-x86_64-linux-gnu.so')

class BinaryDistribution(Distribution):
    def has_ext_modules(foo):
        return True

setup(
    name='pycmtk',
    version='0.1',
    description='Unofficial Python bindings for the Computational Morphology Toolkit (CMTK) (https://www.nitrc.org/projects/cmtk)',
    packages=['pycmtk'],
    package_data={
        'pycmtk': ['pycmtk.cpython-36m-x86_64-linux-gnu.so'],
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