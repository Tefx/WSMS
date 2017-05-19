from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize


extensions = [Extension(name = "*",
                        sources = ["WSMS/c/*.pyx"],
                        extra_compile_args=['-Ofast', '-march=native', '-ffast-math'],
                        libraries = ['wsms', "wsms_extra"],
                        library_dirs = ['./lib'],
                        include_dirs=["./lib/src"])]

setup(ext_modules = cythonize(extensions, 
                              compiler_directives={
                                  "profile": False,
                                  "cdivision": True,
                                  "boundscheck": False,
                                  "initializedcheck": False,
                              }))
