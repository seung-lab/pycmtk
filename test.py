import pycmtk
import numpy as np

# streamxform --inversion-tolerance 1e-03 --precision 5 --inverse registration input.txt output.txt

pts = np.array([[0,0,0], [1,1,1], [2,2,2]], dtype=np.float32)

xpts = pycmtk.xformpoints(pts.flatten('C'), ["registration"], affine_only=True, inversion_tolerance=0.001)
print(xpts)