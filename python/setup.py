from setuptools import setup, Extension
from Cython.Distutils import build_ext

from distutils.core import setup
from Cython.Build import cythonize

import numpy as np

REQUIRES = ['numpy', 'cython']

ext_1 = Extension("run_cython",
                  sources=["run_cython.pyx"],
                  depends=['cfunc.cpp'],
                  language="c++",
                  include_dirs=[np.get_include()])
                  
EXTENSIONS = [ext_1]

if __name__ == "__main__":
    setup(install_requires=REQUIRES,
          zip_safe=False,
          cmdclass={"build_ext": build_ext},
          ext_modules=EXTENSIONS
          )