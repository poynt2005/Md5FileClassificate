#include <Windows.h>
#include <string.h>
#include <Shlwapi.h>

#include "MyBuff.h"

#define GET_WORKDIR_FAILED 1
#define LOAD_DLL_FAILED 2
#define LOAD_FUNCTION_FAILED 3
#define TARGET_DIR_NOT_EXISTS 4
#define LIST_DIR_FAILED 5
#define GET_ABS_PATH_FAILED 6

// current, total, isDuplicated, filename, new filename, args
typedef void (*ProgressTeller)(int, int, int, const char *, const char *, void *args);

typedef unsigned char *(*LPMd5FileHash)(const char *);
typedef void (*LPReleaseHashString)(unsigned char **);

static inline char *PathStem(const char *path)
{
    char *tmpPath = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char) * MAX_PATH);
    memcpy(tmpPath, path, strlen(path));
    PathRemoveExtension(tmpPath);
    char *fileName = PathFindFileName(tmpPath);
    char *tmpFileName = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char) * (strlen(fileName) + 1));
    memcpy(tmpFileName, fileName, strlen(fileName));
    HeapFree(GetProcessHeap(), 0, tmpPath);

    return tmpFileName;
}

static inline int HasString(const char *targetStr, MyBuff *list)
{
    for (int i = 0; i < SizeBuff(list); ++i)
    {
        if (!strcmp(targetStr, AtBuff(list, i, NULL)))
        {
            return 1;
        }
    }
    return 0;
}

static inline char *IntToStr(const int N)
{
    int digit = 0;

    int tempN = N;
    do
    {
        tempN /= 10;
        ++digit;
    } while (tempN > 0);

    char *digitStr = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char) * digit);

    tempN = N;
    int order = 0;
    do
    {
        memset(digitStr + digit - order - 1, (tempN % 10) + '0', sizeof(char));
        tempN /= 10;
        ++order;
    } while (tempN > 0);

    return digitStr;
}

int errorCode = 0;

__declspec(dllexport) int Classificator(const char *targetDirectory, const char *moveToDirectory, char ***duplicatedFiles, int *duplicatedFilesCount, ProgressTeller teller, void *args)
{
    errorCode = 0;

    char *libPath = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char) * MAX_PATH);

    if (!GetCurrentDirectory(MAX_PATH, libPath))
    {
        errorCode = GET_WORKDIR_FAILED;
        return 0;
    }
    memset(libPath + strlen(libPath), '\\', sizeof(char));
    const char *libName = "md5.dll";
    memcpy(libPath + strlen(libPath), libName, strlen(libName));

    HMODULE dll = LoadLibrary(libPath);

    if (dll == NULL)
    {
        errorCode = LOAD_DLL_FAILED;
        return 0;
    }

    LPMd5FileHash Md5FileHash = (LPMd5FileHash)GetProcAddress(dll, "Md5FileHash");
    LPReleaseHashString ReleaseHashString = (LPReleaseHashString)GetProcAddress(dll, "ReleaseHashString");

    if (!Md5FileHash || !ReleaseHashString)
    {
        errorCode = LOAD_FUNCTION_FAILED;
        return 0;
    }

    char *szAbsDirectoryPath = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char) * MAX_PATH);

    if (!GetFullPathName(targetDirectory, MAX_PATH, szAbsDirectoryPath, NULL))
    {
        errorCode = GET_ABS_PATH_FAILED;
        return 0;
    }

    if (szAbsDirectoryPath[strlen(szAbsDirectoryPath) - 1] != '\\')
    {
        memset(szAbsDirectoryPath + strlen(szAbsDirectoryPath), '\\', sizeof(char));
    }

    if (GetFileAttributes(szAbsDirectoryPath) == INVALID_FILE_ATTRIBUTES)
    {
        errorCode = TARGET_DIR_NOT_EXISTS;
        return 0;
    }

    char *szAbsMoveToDirectoryPath = NULL;

    if (moveToDirectory != NULL)
    {
        szAbsMoveToDirectoryPath = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char) * MAX_PATH);

        if (!GetFullPathName(moveToDirectory, MAX_PATH, szAbsMoveToDirectoryPath, NULL))
        {
            errorCode = GET_ABS_PATH_FAILED;
            return 0;
        }

        if (GetFileAttributes(szAbsMoveToDirectoryPath) == INVALID_FILE_ATTRIBUTES)
        {
            CreateDirectory(szAbsMoveToDirectoryPath, NULL);
        }
    }

    char *szDirSelector = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char) * (MAX_PATH + 5));
    memcpy(szDirSelector, szAbsDirectoryPath, strlen(szAbsDirectoryPath));

    const char *selector = "*.*";
    memcpy(szDirSelector + strlen(szDirSelector), selector, sizeof(char) * strlen(selector));

    WIN32_FIND_DATAA FindFileData;

    HANDLE hFile = FindFirstFile(szDirSelector, &FindFileData);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        errorCode = LIST_DIR_FAILED;
        return 0;
    }

    MyBuff *fileNameBuff = NewBuff();
    do
    {
        // Is not directory
        if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            PushBuff(fileNameBuff, FindFileData.cFileName, sizeof(char) * (strlen(FindFileData.cFileName) + 1));
        }
    } while (FindNextFile(hFile, &FindFileData) != 0);

    DWORD dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES)
    {
        errorCode = LIST_DIR_FAILED;
        return 0;
    }

    FindClose(hFile);

    MyBuff *duplicateFileNameBuff = NewBuff();
    MyBuff *hashesBuff = NewBuff();

    for (int i = 0; i < SizeBuff(fileNameBuff); ++i)
    {

        char *szTargetFilePath = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char) * MAX_PATH);

        PathCombine(szTargetFilePath, szAbsDirectoryPath, (char *)AtBuff(fileNameBuff, i, NULL));

        unsigned char *hash = Md5FileHash(szTargetFilePath);

        char *md5_32 = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char) * 32 + 1);
        memcpy(md5_32, (char *)hash, sizeof(char) * 32);

        ReleaseHashString(&hash);

        if (!HasString(md5_32, hashesBuff))
        {
            PushBuff(hashesBuff, md5_32, sizeof(char) * (strlen(md5_32) + 1));

            if (teller != NULL)
            {
                teller(i + 1, SizeBuff(fileNameBuff), 0, szTargetFilePath, NULL, args);
            }
        }
        else
        {
            PushBuff(duplicateFileNameBuff, szTargetFilePath, sizeof(char) * (strlen(szTargetFilePath) + 1));

            if (szAbsMoveToDirectoryPath != NULL)
            {
                char *szDestNewFilePath = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char) * MAX_PATH);
                PathCombine(szDestNewFilePath, szAbsMoveToDirectoryPath, (char *)AtBuff(fileNameBuff, i, NULL));

                int renameCount = 1;
                while (INVALID_FILE_ATTRIBUTES != GetFileAttributes(szDestNewFilePath))
                {
                    memset(szDestNewFilePath, '\0', sizeof(char) * MAX_PATH);

                    char *szDstFilename = PathStem((char *)AtBuff(fileNameBuff, i, NULL));
                    char *renameCountStr = IntToStr(renameCount);
                    char *szFileExt = PathFindExtension((char *)AtBuff(fileNameBuff, i, NULL));

                    char *szDstNewFilename = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, strlen(szDstFilename) + 20);

                    memcpy(szDstNewFilename, szDstFilename, strlen(szDstFilename));
                    HeapFree(GetProcessHeap(), 0, szDstFilename);
                    memset(szDstNewFilename + strlen(szDstNewFilename), '_', sizeof(char));
                    memcpy(szDstNewFilename + strlen(szDstNewFilename), renameCountStr, strlen(renameCountStr));
                    memcpy(szDstNewFilename + strlen(szDstNewFilename), szFileExt, strlen(szFileExt));
                    HeapFree(GetProcessHeap(), 0, renameCountStr);
                    PathCombine(szDestNewFilePath, szAbsMoveToDirectoryPath, szDstNewFilename);
                    ++renameCount;
                }

                MoveFile(szTargetFilePath, szDestNewFilePath);

                if (teller != NULL)
                {
                    teller(i + 1, SizeBuff(fileNameBuff), 1, szTargetFilePath, szDestNewFilePath, args);
                }
            }
            else
            {
                if (teller != NULL)
                {
                    teller(i + 1, SizeBuff(fileNameBuff), 1, szTargetFilePath, NULL, args);
                }
            }
        }

        HeapFree(GetProcessHeap(), 0, szTargetFilePath);
        HeapFree(GetProcessHeap(), 0, md5_32);
    }

    HeapFree(GetProcessHeap(), 0, szDirSelector);
    HeapFree(GetProcessHeap(), 0, szAbsDirectoryPath);

    if (duplicatedFiles != NULL)
    {
        *duplicatedFiles = (char **)HeapAlloc(GetProcessHeap(), 0, sizeof(char *) * SizeBuff(duplicateFileNameBuff));
        for (int i = 0; i < SizeBuff(duplicateFileNameBuff); ++i)
        {
            (*duplicatedFiles)[i] = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char) * (strlen((char *)AtBuff(duplicateFileNameBuff, i, NULL)) + 1));
            memcpy((*duplicatedFiles)[i], (char *)AtBuff(duplicateFileNameBuff, i, NULL), strlen((char *)AtBuff(duplicateFileNameBuff, i, NULL)));
        }
    }

    *duplicatedFilesCount = SizeBuff(duplicateFileNameBuff);

    FreeBuff(&fileNameBuff);
    FreeBuff(&duplicateFileNameBuff);
    FreeBuff(&hashesBuff);

    return 1;
}

__declspec(dllexport) void ReleaseFilenameBuffer2D(char ***buffer2D, const int N)
{
    if (buffer2D == NULL)
    {
        return;
    }

    for (int i = 0; i < N; i++)
    {
        HeapFree(GetProcessHeap(), 0, (*buffer2D)[i]);
    }

    HeapFree(GetProcessHeap(), 0, *buffer2D);
    *buffer2D = NULL;
}

__declspec(dllexport) int GetClassificatorError()
{
    return errorCode;
}