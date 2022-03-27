#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma ide diagnostic ignored "UnusedParameter"
#pragma comment(lib, "crypt32.lib")

#include <stdio.h>
#include <Python.h>
#include <string.h>
#include <Windows.h>
#include <wincrypt.h>

static PyObject *PyDpapiError;


void PyHandleError(char *s) {
    char buf[1024];
    snprintf(
            buf,
            sizeof(buf),
            "%s %s%x",
            s,
            "Error number: ",
            GetLastError());
    PyErr_SetString(PyDpapiError, buf);
}


static PyObject *encrypt(PyObject *self, PyObject *args) {
    // parse inputs
    Py_buffer buf;

    if (!PyArg_ParseTuple(args, "y*", &buf)) {
        return NULL;
    }

    char *input = (char *) buf.buf;
    size_t inputLen = buf.len;

    PyBuffer_Release(&buf);

    // initialize
    DATA_BLOB DataIn;
    DATA_BLOB DataOut;
    BYTE *pbDataInput = (BYTE *) input;
    DWORD cbDataInput = (DWORD) inputLen;
    DataIn.pbData = pbDataInput;
    DataIn.cbData = cbDataInput;

    // do the encrypt
    if (!CryptProtectData(
            &DataIn,
            L"",
            NULL,
            NULL,
            NULL,
            0,
            &DataOut
    )) {
        PyHandleError("Encryption failed.");
        LocalFree(DataOut.pbData);
        return NULL;
    }

    // construct output and cleanup
    PyObject *result = PyByteArray_FromStringAndSize(
            (char *) DataOut.pbData, DataOut.cbData);

    LocalFree(DataOut.pbData);
    memset(input, 0, inputLen);

    return result;
}

PyDoc_STRVAR(
        encrypt_docs,
        "encrypt(value)\n"
        "--\n"
        "\n"
        "Encrypts input bytesarray\n"
        "Arguments: (data).");

static PyObject *decrypt(PyObject *self, PyObject *args) {
    // parse inputs
    Py_buffer buf;

    if (!PyArg_ParseTuple(args, "y*", &buf)) {
        return NULL;
    }

    char *input = (char *) buf.buf;
    size_t inputLen = buf.len;

    PyBuffer_Release(&buf);

    // initialize
    DATA_BLOB DataIn;
    DATA_BLOB DataOut;
    BYTE *pbDataInput = (BYTE *) input;
    DWORD cbDataInput = inputLen;
    DataIn.pbData = pbDataInput;
    DataIn.cbData = cbDataInput;
    LPWSTR pDescrOut = NULL;

    // do the decrypt
    if (!CryptUnprotectData(
            &DataIn,
            &pDescrOut,
            NULL,
            NULL,
            NULL,
            0,
            &DataOut
    )) {
        PyHandleError("Decryption failed.");
        LocalFree(DataOut.pbData);
        LocalFree(pDescrOut);
        return NULL;
    }

    // construct output and cleanup
    PyObject *result = PyByteArray_FromStringAndSize(
            (char *) DataOut.pbData, DataOut.cbData);

    LocalFree(DataOut.pbData);
    LocalFree(pDescrOut);

    return result;
}

PyDoc_STRVAR(
        decrypt_docs,
        "decrypt(data)\n"
        "--\n"
        "\n"
        "Decrypts input bytesarray\n"
        "Arguments: (data).");


static PyObject *input(PyObject *self, PyObject *args) {
    char *prompt;

    if (!PyArg_ParseTuple(args, "s", &prompt)) {
        return NULL;
    }

    // write prompt to screen
    char *s;
    for (s = prompt; *s != '\0'; s++) {
        _putwch(*s);
    }

    int c;
    int bufSize = 1024;
    char userInputBuffer[1024] = "";
    if (!VirtualLock(userInputBuffer, bufSize)) {
        PyHandleError("Failed to obtain memory lock.");
        return NULL;
    }
    int inputLen = 0;
    DATA_BLOB DataIn;
    DATA_BLOB DataOut;

    // read user input
    while (1) {
        c = _getwch();

        if (c == '\n' || c == '\r') {
            break;
        }

        // backspace
        if (c == '\b') {
            if (inputLen == 0) {
                continue;
            } else {
                // go back and zero out the previously entered character
                inputLen--;
                userInputBuffer[inputLen] = 0;
                continue;
            }
        }

        userInputBuffer[inputLen] = (char) c;
        inputLen++;

        if (inputLen == bufSize) {
            PyHandleError("Exceeded buffer size");
        }
    }

    _putwch('\r');
    _putwch('\n');

    DataIn.pbData = (BYTE *) userInputBuffer;
    DataIn.cbData = inputLen;

    if (!CryptProtectData(
            &DataIn,
            L"User input",
            NULL,
            NULL,
            NULL,
            0,
            &DataOut)) {
        PyHandleError("Encryption failed.");
        LocalFree(DataOut.pbData);
        return NULL;
    }

    // DataIn and userInputBuffer are pointing to the same memory, we wipe them
    // both here and release the memory block
    memset(userInputBuffer, 0, bufSize);
    if (!VirtualUnlock(userInputBuffer, bufSize)) {
        PyHandleError("Failed to release memory lock.");
        return NULL;
    }

    // construct output and cleanup
    PyObject *result = PyByteArray_FromStringAndSize(
            (char *) DataOut.pbData, DataOut.cbData);

    LocalFree(DataOut.pbData);

    return result;
}

PyDoc_STRVAR(
        input_docs,
        "input(prompt)\n"
        "--\n"
        "\n"
        "Reads user input into an encrypted bytearray.\n"
        "Arguments: (prompt).");


static PyMethodDef module_methods[] = {
        {"encrypt", (PyCFunction) encrypt, METH_VARARGS, encrypt_docs},
        {"decrypt", (PyCFunction) decrypt, METH_VARARGS, decrypt_docs},
        {"input",   (PyCFunction) input,   METH_VARARGS, input_docs},
        {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef PyDpapi =
        {
                PyModuleDef_HEAD_INIT,
                "pydpapi", /* name of module */
                "Python C-Extension wrapper around the Windows DPAPI CryptProtect methods.\n",
                -1, /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
                module_methods
        };


PyMODINIT_FUNC PyInit_pydpapi(void) {
    PyObject *m;

    m = PyModule_Create(&PyDpapi);
    if (m == NULL)
        return NULL;

    PyDpapiError = PyErr_NewException("pydpapi.error", NULL, NULL);
    Py_XINCREF(PyDpapiError);
    if (PyModule_AddObject(m, "error", PyDpapiError) < 0) {
        Py_XDECREF(PyDpapiError);
        Py_CLEAR(PyDpapiError);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}
