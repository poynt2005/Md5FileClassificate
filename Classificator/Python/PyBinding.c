#include <Windows.h>
#include <Python.h>
#include <string.h>

#include "Map.h"

typedef void (*ProgressTeller)(int, int, int, const char *, const char *, void *);

typedef int (*LPClassificator)(const char *, const char *, char ***, int *, ProgressTeller, void *);
typedef void (*LPReleaseFilenameBuffer2D)(char ***, const int);
typedef int (*LPGetClassificatorError)();

typedef struct _classificatorContext
{
    LPClassificator Classificator;
    LPReleaseFilenameBuffer2D ReleaseFilenameBuffer2D;
    LPGetClassificatorError GetClassificatorError;
    int isLoaded;
} ClassificatorContext;

static Map TellerMapping = NULL;

static inline char *GetIdByPtr(void *obj)
{
    unsigned long objPtr = (unsigned long)obj;

    int digits = 0;

    do
    {
        objPtr /= 10;
        ++digits;
    } while (objPtr > 0);

    char *objId = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char) * digits);

    int order = 0;
    objPtr = (unsigned long)obj;

    do
    {
        memset(objId + digits - order - 1, (objPtr % 10) + '0', sizeof(char));
        objPtr /= 10;
        ++order;
    } while (objPtr > 0);

    return objId;
}

void ProgressTellerFuncImp(int current, int total, int isDuplicated, const char *filename, const char *newFilename, void *progressTellerId)
{
    PyObject *pyTellerFunc = MapFind(TellerMapping, progressTellerId);

    PyObject *pyFilename = PyBytes_FromString(filename);
    PyObject *pyNewFilename = NULL;

    if (newFilename == NULL)
    {
        pyNewFilename = Py_None;
    }
    else
    {
        pyNewFilename = PyBytes_FromString(newFilename);
    }

    PyObject *rst = PyObject_CallFunction(pyTellerFunc, "iiiOO", current, total, isDuplicated, pyFilename, pyNewFilename);

    Py_XDECREF(rst);
    Py_XDECREF(pyNewFilename);
    Py_XDECREF(pyFilename);
}

static ClassificatorContext classificatorContext = {NULL, NULL, NULL, 0};

static PyObject *PyCFunc_LoadLibrary(PyObject *self, PyObject *args)
{
    if (classificatorContext.isLoaded)
    {
        PyErr_SetString(PyExc_BaseException, "Dynamic library has been loaded");
        return NULL;
    }

    char *classificatorDllPath = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char) * MAX_PATH);

    if (!GetCurrentDirectory(MAX_PATH, classificatorDllPath))
    {
        PyErr_SetString(PyExc_IOError, "Cannot get the target dll path, make sure Classificator.dll is in the same path");
        return NULL;
    }

    memset(classificatorDllPath + strlen(classificatorDllPath), '\\', sizeof(char));

    const char *classificatorStr = "Classificator.dll";
    memcpy(classificatorDllPath + strlen(classificatorDllPath), classificatorStr, strlen(classificatorStr) * sizeof(char));

    HMODULE ClassificatorLib = LoadLibrary(classificatorStr);
    HeapFree(GetProcessHeap(), 0, classificatorDllPath);

    if (!ClassificatorLib)
    {
        PyErr_SetString(PyExc_RuntimeError, "Cannot get the target dll path, make sure Classificator.dll is in the same path");
        return NULL;
    }

    classificatorContext.Classificator = (LPClassificator)GetProcAddress(ClassificatorLib, "Classificator");
    classificatorContext.ReleaseFilenameBuffer2D = (LPReleaseFilenameBuffer2D)GetProcAddress(ClassificatorLib, "ReleaseFilenameBuffer2D");
    classificatorContext.GetClassificatorError = (LPGetClassificatorError)GetProcAddress(ClassificatorLib, "GetClassificatorError");

    if (classificatorContext.Classificator == NULL || classificatorContext.ReleaseFilenameBuffer2D == NULL || classificatorContext.GetClassificatorError == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Cannot load dll functions, make sure Classificator.dll is a correct dll");
        return NULL;
    }

    return Py_None;
}

static PyObject *PyCFunc_Classificator(PyObject *self, PyObject *args)
{

    const char *targetFolder = NULL;
    PyObject *pyMoveToFolderStr = NULL;
    PyObject *pyProgressTeller = NULL;

    if (!PyArg_ParseTuple(args, "sOO", &targetFolder, &pyMoveToFolderStr, &pyProgressTeller))
    {
        PyErr_SetString(PyExc_TypeError, "Wrong Arguments");
        return NULL;
    }

    char *moveToFolder = NULL;

    if (!PyUnicode_Check(pyMoveToFolderStr) && pyMoveToFolderStr != Py_None)
    {
        PyErr_SetString(PyExc_TypeError, "Wrong Arguments, position 1 must be a string");
        return NULL;
    }

    if (!PyCallable_Check(pyProgressTeller))
    {
        PyErr_SetString(PyExc_TypeError, "Wrong Arguments, position 2 must be a callable object");
        return NULL;
    }

    if (pyMoveToFolderStr != Py_None)
    {
        PyObject *pyRawStr = PyUnicode_AsEncodedString(pyMoveToFolderStr, "utf-8", "strict");

        if (pyRawStr == NULL)
        {
            PyErr_SetString(PyExc_UnicodeError, "Decode filepath failed");
            return NULL;
        }

        moveToFolder = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char) * PyBytes_Size(pyRawStr));
        memcpy(moveToFolder, PyBytes_AsString(pyRawStr), sizeof(char) * PyBytes_Size(pyRawStr));

        Py_DECREF(pyRawStr);
    }

    char *pyTellerFuncId = GetIdByPtr(pyProgressTeller);
    char **duplicatedFiles = NULL;
    int duplicatedFilesCount = 0;

    MapInsert(TellerMapping, pyTellerFuncId, pyProgressTeller);

    int isSuccess = classificatorContext.Classificator(
        targetFolder,
        moveToFolder,
        &duplicatedFiles,
        &duplicatedFilesCount,
        ProgressTellerFuncImp,
        pyTellerFuncId
        //
    );

    if (!isSuccess)
    {
        char errStr[50] = {'\0'};
        sprintf(errStr, "Do Classificator failed, error code: %d", classificatorContext.GetClassificatorError());
        PyErr_SetString(PyExc_RuntimeError, errStr);

        classificatorContext.ReleaseFilenameBuffer2D(&duplicatedFiles, duplicatedFilesCount);

        MapErase(&TellerMapping, pyTellerFuncId);
        HeapFree(GetProcessHeap(), 0, pyTellerFuncId);

        return NULL;
    }

    PyObject *pyDuplicatedFileList = PyList_New(duplicatedFilesCount);

    for (int i = 0; i < duplicatedFilesCount; ++i)
    {
        PyObject *duplocatedFileName = PyBytes_FromString(duplicatedFiles[i]);
        PyList_SetItem(pyDuplicatedFileList, i, duplocatedFileName);
    }

    classificatorContext.ReleaseFilenameBuffer2D(&duplicatedFiles, duplicatedFilesCount);

    MapErase(&TellerMapping, pyTellerFuncId);
    HeapFree(GetProcessHeap(), 0, pyTellerFuncId);

    return pyDuplicatedFileList;
}

static PyMethodDef Methods[] = {
    {"LoadLibrary", PyCFunc_LoadLibrary, METH_VARARGS, "Load Classificator dynamic library"},
    {"Classificator", PyCFunc_Classificator, METH_VARARGS, "Start classificating files"},
    {NULL, NULL, 0, NULL}
    // py method
};

static struct PyModuleDef Module = {
    PyModuleDef_HEAD_INIT,
    "ClassificatorModule",
    NULL,
    -1,
    Methods};

PyMODINIT_FUNC
PyInit_ClassificatorModule()
{
    TellerMapping = MapCreate();
    return PyModule_Create(&Module);
}
