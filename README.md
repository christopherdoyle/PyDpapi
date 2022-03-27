# pydpapi: Python DPAPI C Extension

## Description

A Python C extension are Windows DPAPI methods. Implemented as a learning
exercise in C extensions and offers no particular advantage over `ctypes.windll`
methods.

This package implements an `input` method that aims to securely read user input
from the console into an encrypted blob. The implementation mimicks the that of
`getpass`, but additionally uses VirtualLock to prevent paging, and encrypts as
soon as possible then zeroes out the input. This way, a user can input a string
to a Python application without Python ever having access to the plaintext.

Little thought has been given to safe handling of user inputs, to encodings, or 
to exceptions.

## Usage

```python
import pydpapi

user_input_encrypted = pydpapi.input("Enter password: ")
print(pydpapi.decrypt(user_input_encrypted))
```

```python
import pydpapi

s = bytearray(b"Hello World")
print(f"Input: '{s}'")
encrypted = pydpapi.encrypt(s)
print(f"Encrypted: {encrypted}")
print(f"Input: '{s}'")  # input is zeroed
decrypted = pydpapi.decrypt(encrypted)
print(f"Decrypted: {decrypted}")
print(f"Encrypted: {encrypted}")
```

## Installing

Package intended to be compiled on Windows with VS build tools or similar.
Compiling Python C extensions on Windows can be difficult, and the packages
required seem to change every couple years.

The easiest is to install Visual Studio Community with the "Python Development"
workflow ticked and "Python native development tools" option. Then compile in
the shell with appropriate environment variables. E.g. open a "Developer 
PowerShell", or invoke appropriate build prep scripts:

```commandline
& 'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat'
```

Package can then be installed with pip

```commandline
python -m pip install .
python -m pip install pytest
pytest tests/
```

I have only tested Python 3.10 64-bit but there is nothing specific to the
python version in the implementation other than Python 3-specific C hooks and
type hints in the pyi file.

### CMakeLists.txt

A `CMakeLists.txt` exists purely for type inference in CLion, it requires the
following option to cmake in order to include `Python.h`:
`-DPythonExe="\path\to\python.exe"`. CMake should be configured for VS in order
to recognize all the Windows headers.
