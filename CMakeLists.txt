cmake_minimum_required (VERSION 3.21)
option(PythonExe "PythonExe")
project(PyDpapi)


find_package(Python COMPONENTS Interpreter Development REQUIRED)

if( PYTHON_FOUND )
    message(STATUS "python executable: " ${Python_EXECUTABLE})
    execute_process(
            COMMAND ${PythonExe} -c "from distutils import sysconfig; print(sysconfig.get_config_var('INCLUDEPY'))"
            OUTPUT_VARIABLE PYTHON_INCLUDE_DIRECTORY
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS "python include directory: " ${PYTHON_INCLUDE_DIRECTORY})
else()
    message( FATAL_ERROR "A valid Python interpreter is required for finding Python's site-packages directory!" )
endif()

message(STATUS "Python version: ${PYTHONLIBS_VERSION_STRING}")
include_directories(${PYTHON_INCLUDE_DIRECTORY})

add_library(pydpapi pydpapi.c)
set_target_properties(
    pydpapi
    PROPERTIES
    PREFIX ""
    OUTPUT_NAME "pydpapi"
    LINKER_LANGUAGE C
)
