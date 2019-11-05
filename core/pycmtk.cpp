#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

namespace py = pybind11;

// #include <cmtk_sqlite3_mangle.h>
#include <Base/cmtkXform.h>
#include <Base/cmtkXformList.h>

#include <IO/cmtkXformIO.h>
#include <IO/cmtkXformListIO.h>

// #include <cmtk_NrrdIO_mangle.h>

#include <algorithm>
#include <iterator>
#include <vector>
#include <string>
#include <limits>
#include <iostream>

// xform_brain(query_skt, sample = "FAFB14",  reference = "FCWB")
// Mirror skt
// mirror_brain(query_skt, FCWB)
// Move back to FAFB14
// xform_brain(query_skt, sample = "FCWB",  reference = "FAFB14")
// The R functions use functions from the registration package CMTK

std::vector<float> xformpoints(
    const std::vector<float> &points,
    const std::vector<std::string> &xform_paths,
    bool affine_only = false,
    float inversion_tolerance = 1e-8
  ) {

  cmtk::XformList xformList = cmtk::XformListIO::MakeFromStringList(xform_paths);

  xformList.SetEpsilon(inversion_tolerance);

  if (affine_only) {
    xformList = xformList.MakeAllAffine();
  }
  
  const size_t N = points.size();
  // std::cout <<" hello world" << std::endl;
  std::vector<float> output;
  output.reserve(N);

  cmtk::Xform::SpaceVectorType xyz;
  bool valid = true;

  for (size_t i = 0; i < N; i += 3) {
    xyz[0] = points[i + 0];
    xyz[1] = points[i + 1];
    xyz[2] = points[i + 2];

    valid = xformList.ApplyInPlace(xyz);

    if (valid) {
      output[i + 0] = xyz[0];
      output[i + 1] = xyz[1];
      output[i + 2] = xyz[2];
    }
    else {
      output[i + 0] = std::numeric_limits<float>::quiet_NaN();
      output[i + 1] = std::numeric_limits<float>::quiet_NaN();
      output[i + 2] = std::numeric_limits<float>::quiet_NaN();
    }
  }

  return points;  
}

PYBIND11_MODULE(pycmtk, m) {
  m.doc() = 
  "Python bindings for a subset of the Computational Morphology Toolkit (CMTK).\n"
  "https://www.nitrc.org/projects/cmtk\n"
  "\n"
  "This is just a wrapper around the much more sophisticated code by the CMTK authors.\n"
  "\n"
  "pycmtk Copyright (C) 2019 William Silversmith\n"
  "This program comes with ABSOLUTELY NO WARRANTY\n"
  "This is free software, and you are welcome to redistribute it\n"
  "under certain conditions; see github.... for details.";

  m.def("xformpoints", &xformpoints, 
    "Apply a sequence of coordinate transformations to a list of xyz coordinates.\n"
    "You can apply the inverse transformation by specifying '-i $SPACE'.\n"
    "\n"
    "Note: This code was derived from the `xformstream` command line tool.\n"
    "\n"
    "Example:\n"
    "  out_points = xformpoints(points=[0,0,0], xform_paths=['FAFB14', 'FCWB'])\n"
    "\n"
    "Required:\n"
    "  points: list of floating xyz triples.\n"
    "  xform_paths: sequence of known spaces\n"
    "\n"
    "Optional:\n"
    " affine_only: Apply only the affine component of each transformation \n"
    "    (or its inverse, if specified) in the given series, even if the \n"
    "    actual transformation is nonrigid.\n"
    "  inversion_tolerance: Numerical tolerance of B-spline inversion in mm. \n"
    "    Smaller values will lead to more accurate inversion but may increase \n"
    "    failure rate.\n"
    "\n"
    "Returns: list of transformed xyz triples\n",

    py::arg("points"), py::arg("xform_paths"),
    py::arg("affine_only") = false, py::arg("inversion_tolerance") = 1e-8
  );
}


