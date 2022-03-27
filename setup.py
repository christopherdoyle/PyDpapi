from distutils.core import setup, Extension

setup(
    name="PyDpapi",
    version="1.0",
    author="Christopher Doyle",
    ext_modules=[Extension("pydpapi", ["pydpapi.c"])],
)
