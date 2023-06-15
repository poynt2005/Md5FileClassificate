#include <Windows.h>
#include <Python.h>
#include <string.h>

typedef unsigned char *(*LPMd5Hash)(const unsigned char *, const unsigned int);
typedef unsigned char *(*LPMd5FileHash)(const char *);
typedef void (*LPReleaseHashString)(unsigned char **);
typedef struct _md5Context
{
    LPMd5Hash Md5Hash;
    LPMd5FileHash Md5FileHash;
    LPReleaseHashString ReleaseHashString;
    int isLoaded;
} Md5Context;

static Md5Context md5Context = {NULL, NULL, 0};

static PyObject *PyCFunc_LoadLibrary(PyObject *self, PyObject *args)
{
    if (md5Context.isLoaded)
    {
        PyErr_SetString(PyExc_BaseException, "Dynamic library has been loaded");
        return NULL;
    }

    char *md5DllPath = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char) * MAX_PATH);

    DWORD isDirectoryGet = GetCurrentDirectory(MAX_PATH, md5DllPath);

    if (!isDirectoryGet)
    {
        PyErr_SetString(PyExc_IOError, "Cannot get the target dll path, make sure md5.dll is in the same path");
        return NULL;
    }

    memset(md5DllPath + strlen(md5DllPath), '\\', sizeof(char));

    const char *md5str = "md5.dll";
    memcpy(md5DllPath + strlen(md5DllPath), md5str, strlen(md5str) * sizeof(char));

    HMODULE Md5Lib = LoadLibrary(md5DllPath);
    HeapFree(GetProcessHeap(), 0, md5DllPath);

    if (!Md5Lib)
    {
        PyErr_SetString(PyExc_IOError, "Cannot get the target dll path, make sure md5.dll is in the same path");
        return NULL;
    }

    md5Context.Md5Hash = (LPMd5Hash)GetProcAddress(Md5Lib, "Md5Hash");
    md5Context.Md5FileHash = (LPMd5FileHash)GetProcAddress(Md5Lib, "Md5FileHash");
    md5Context.ReleaseHashString = (LPReleaseHashString)GetProcAddress(Md5Lib, "ReleaseHashString");

    if (md5Context.Md5Hash == NULL || md5Context.Md5FileHash == NULL || md5Context.ReleaseHashString == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Cannot load dll functions, make sure md5.dll is a correct dll");
        return NULL;
    }

    md5Context.isLoaded = 1;

    return Py_None;
}

static PyObject *PyCFunc_Md5Hash(PyObject *self, PyObject *args)
{
    if (!md5Context.isLoaded)
    {
        PyErr_SetString(PyExc_BaseException, "Dynamic library has not loaded");
        return NULL;
    }

    PyObject *pyStr = NULL;

    if (!PyArg_ParseTuple(args, "O", &pyStr))
    {
        PyErr_SetString(PyExc_TypeError, "Wrong Argument");
        return NULL;
    }

    if (!PyUnicode_Check(pyStr))
    {
        PyErr_SetString(PyExc_TypeError, "Wrong Argument, position 0 must be a string type");
        return NULL;
    }

    PyObject *pyRawByteStr = PyUnicode_AsEncodedString(pyStr, "utf-8", "ignore");

    if (!PyBytes_Check(pyRawByteStr))
    {
        PyErr_SetString(PyExc_BaseException, "Convert to Python bytes object failed");
        return NULL;
    }

    Py_ssize_t byteLength = PyBytes_Size(pyRawByteStr);
    unsigned char *bytes = (unsigned char *)PyBytes_AsString(pyRawByteStr);

    unsigned char *hash = md5Context.Md5Hash(bytes, byteLength);

    if (hash == NULL)
    {
        PyErr_SetString(PyExc_BaseException, "Calculate hash failed");
        return NULL;
    }

    Py_XDECREF(pyStr);
    Py_XDECREF(pyRawByteStr);

    PyObject *pyHashStr = PyUnicode_FromString((char *)hash);

    md5Context.ReleaseHashString(&hash);

    return pyHashStr;
}

static PyObject *PyCFunc_Md5FileHash(PyObject *self, PyObject *args)
{
    if (!md5Context.isLoaded)
    {
        PyErr_SetString(PyExc_BaseException, "Dynamic library has not loaded");
        return NULL;
    }

    const char *filePath = NULL;

    if (!PyArg_ParseTuple(args, "s", &filePath))
    {
        PyErr_SetString(PyExc_TypeError, "Wrong Argument, position 0 must be a string");
        return NULL;
    }

    if (GetFileAttributes(filePath) == INVALID_FILE_ATTRIBUTES)
    {
        PyErr_SetString(PyExc_BaseException, "Target file is not exists");
        return NULL;
    }

    unsigned char *hash = md5Context.Md5FileHash(filePath);

    if (hash == NULL)
    {
        PyErr_SetString(PyExc_BaseException, "Calculate hash failed");
        return NULL;
    }

    PyObject *pyHashStr = PyUnicode_FromString((char *)hash);
    md5Context.ReleaseHashString(&hash);
    return pyHashStr;
}

static PyMethodDef Methods[] = {
    {"LoadLibrary", PyCFunc_LoadLibrary, METH_VARARGS, "Load Md5 dynamic library"},
    {"Md5Hash", PyCFunc_Md5Hash, METH_VARARGS, "Get a md5 hash from one string"},
    {"Md5FileHash", PyCFunc_Md5FileHash, METH_VARARGS, "Get a md5 hash from one file"},
    {NULL, NULL, 0, NULL}
    // py method
};

static struct PyModuleDef Module = {
    PyModuleDef_HEAD_INIT,
    "Md5Module",
    NULL,
    -1,
    Methods};

PyMODINIT_FUNC
PyInit_Md5Module()
{
    return PyModule_Create(&Module);
}
