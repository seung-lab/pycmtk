# pycmtk

Unofficial Python bindings for cmtk functions.

Get the original CMTK here: https://www.nitrc.org/projects/cmtk/  
Get the R version here: https://github.com/jefferis/cmtkr  

## Compiling

Currently the compilation toolchain is a bit of a hack. The cmake files 
work really well for creating the core cmtk command line tools. We are using
pybind11 to wrap the C++ functions to avoid syscalls and filesystem IO.  

We've included pybind11 inside the `core/` directory and pycmtk.cpp as the interface
to python. 

```bash
0. mkdir build && cd build
1. cmake -DBUILD_SHARED_LIBS=on -DPYTHON_EXECUTABLE:FILEPATH=`which python` ../core

  # This generates the .so files for each compiled subunit like Base, IO, Numerics, etc
  # which are by default generated as .a files (static libraries instead of shared .so libs).
  # We also need to specify which version of python to compile against or it will auto-detect
  # the highest version present on your system.

2. make -j 8 # for 8 cores
3. edit core/CMakeLists.txt to include the python bindings
4. cmake -DBUILD_SHARED_LIBS=on -DPYTHON_EXECUTABLE:FILEPATH=`which python` ../core # same as line 1
5. make pycmtk

  # Generates the python library in build/bin/
```

Currently, this compilation generates a series of shared libraries (.so files) that are linked to 
e.g. `build/bin/pycmtk.cpython-36m-x86_64-linux-gnu.so`. This makes it somewhat difficult to distribute, 
so additional work will be needed to create a combined shared library.
