
#include <Windows.h>
#include <string.h>
#include <Rpc.h>
#include "resource.h"

const char *exts[] = {
    "png",
    "jpg",
    "jpg",
    "jpg"
    //
};

typedef struct _threadFuncContext
{
    char *destFolder;
    int fileID;
} ThreadFuncContext;

HMODULE hModule = NULL;

static inline char *GetRandomStr()
{
    UUID uuid;
    unsigned char *uuidStr = NULL;

    if (UuidCreate(&uuid) != RPC_S_OK)
    {
        return NULL;
    }

    if (UuidToString(&uuid, &uuidStr) != RPC_S_OK)
    {
        return NULL;
    }

    char *str = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char) * 9);
    memcpy(str, (char *)uuidStr, sizeof(char) * 8);

    RpcStringFree(&uuidStr);

    return str;
}

static inline char *GetRandomFilename(const char *destFolder, const char *ext)
{

    char *filename = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char) * MAX_PATH);

    char *randomStr = GetRandomStr();

    if (randomStr == NULL)
    {
        HeapFree(GetProcessHeap(), 0, filename);
        return NULL;
    }

    memcpy(filename, destFolder, strlen(destFolder));

    if (filename[strlen(filename) - 1] == '\\')
    {
        memcpy(filename + strlen(filename), randomStr, strlen(randomStr) * sizeof(char));
    }
    else
    {
        memset(filename + strlen(filename), '\\', sizeof(char));
        memcpy(filename + strlen(filename), randomStr, strlen(randomStr) * sizeof(char));
    }

    memset(filename + strlen(filename), '.', sizeof(char));
    memcpy(filename + strlen(filename), ext, sizeof(char) * strlen(ext));

    HeapFree(GetProcessHeap(), 0, randomStr);
    return filename;
}

char *GetResource(const int resourceId, DWORD *fileSize)
{
    HRSRC hRsrc = FindResource(
        hModule,
        MAKEINTRESOURCE(resourceId),
        "IMG"
        //
    );

    if (hRsrc == NULL)
    {
        return NULL;
    }

    HGLOBAL hGlobal = LoadResource(
        hModule,
        hRsrc
        //
    );

    if (hGlobal == NULL)
    {
        return NULL;
    }

    DWORD sizeofResource = SizeofResource(
        hModule,
        hRsrc
        //
    );

    if (!sizeofResource)
    {
        return NULL;
    }

    char *resource = (char *)LockResource(hGlobal);

    if (resource == NULL)
    {
        return NULL;
    }

    char *fileContent = (char *)HeapAlloc(GetProcessHeap(), 0, sizeof(char) * sizeofResource);
    memcpy(fileContent, resource, sizeof(char) * sizeofResource);

    *fileSize = sizeofResource;

    return fileContent;
}

int WriteFileToDisk(const char *destFolder, const int fileID)
{
    char *randomFilename = GetRandomFilename(destFolder, exts[fileID]);
    if (randomFilename == NULL)
    {
        return 0;
    }

    DWORD fileSize = 0;
    char *fileContent = GetResource(fileID, &fileSize);
    if (fileContent == NULL)
    {
        HeapFree(GetProcessHeap(), 0, randomFilename);
        return 0;
    }

    HANDLE hFile = CreateFile(
        randomFilename,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        //
    );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        HeapFree(GetProcessHeap(), 0, randomFilename);
        HeapFree(GetProcessHeap(), 0, fileContent);
        return 0;
    }

    if (!WriteFile(
            hFile,
            fileContent,
            fileSize,
            &fileSize,
            NULL))
    {
        HeapFree(GetProcessHeap(), 0, randomFilename);
        HeapFree(GetProcessHeap(), 0, fileContent);
        CloseHandle(hFile);
        return 0;
    }

    HeapFree(GetProcessHeap(), 0, randomFilename);
    HeapFree(GetProcessHeap(), 0, fileContent);
    CloseHandle(hFile);

    return 1;
}

DWORD WINAPI ThreadFuncJob(LPVOID lpParam)
{
    ThreadFuncContext *context = (ThreadFuncContext *)lpParam;
    int isWrote = WriteFileToDisk(context->destFolder, context->fileID);
    return isWrote;
}

int CallThreadFuncJob(const char *destFolder, const int totalJobCount)
{
    HANDLE *hThread = (HANDLE *)HeapAlloc(GetProcessHeap(), 0, totalJobCount * sizeof(HANDLE));
    ThreadFuncContext **ctx = (ThreadFuncContext **)HeapAlloc(GetProcessHeap(), 0, sizeof(ThreadFuncContext *) * totalJobCount);

    for (int j = 0; j < totalJobCount; ++j)
    {
        ctx[j] = HeapAlloc(GetProcessHeap(), 0, sizeof(ThreadFuncContext));

        ctx[j]->destFolder = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char) * MAX_PATH);
        memcpy(ctx[j]->destFolder, destFolder, sizeof(char) * strlen(destFolder));
        ctx[j]->fileID = j;

        hThread[j] = CreateThread(
            NULL,
            0,
            ThreadFuncJob,
            (void *)ctx[j],
            0,
            NULL
            //
        );
    }

    WaitForMultipleObjects(totalJobCount, hThread, TRUE, INFINITE);

    int *result = (int *)HeapAlloc(GetProcessHeap(), 0, sizeof(int) * totalJobCount);
    for (int j = 0; j < totalJobCount; ++j)
    {
        DWORD exitCode;
        BOOL isExitCodeGet = GetExitCodeThread(hThread[j], &exitCode);
        result[j] = (int)exitCode & (int)isExitCodeGet;

        HeapFree(GetProcessHeap(), 0, ctx[j]->destFolder);
        ctx[j]->destFolder = NULL;

        HeapFree(GetProcessHeap(), 0, ctx[j]);
        ctx[j] = NULL;
    }

    HeapFree(GetProcessHeap(), 0, ctx);
    ctx = NULL;

    for (int j = 0; j < totalJobCount; ++j)
    {
        if (!result[j])
        {
            return 0;
        }
    }

    HeapFree(GetProcessHeap(), 0, hThread);
    hThread = NULL;

    HeapFree(GetProcessHeap(), 0, result);
    result = NULL;

    return 1;
}

__declspec(dllexport) int Generate(const char *destFolder, const int totalNumber)
{

    char *destFolderAbs = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char) * MAX_PATH);

    if (!GetFullPathName(destFolder, MAX_PATH, destFolderAbs, NULL))
    {
        HeapFree(GetProcessHeap(), 0, destFolderAbs);
        destFolderAbs = NULL;
        return 0;
    }

    if (GetFileAttributes(destFolderAbs) == INVALID_FILE_ATTRIBUTES)
    {
        if (!CreateDirectory(destFolderAbs, NULL))
        {
            HeapFree(GetProcessHeap(), 0, destFolderAbs);
            destFolderAbs = NULL;
            return 0;
        }
    }

    const int itCount = totalNumber / 4;
    const int itRest = totalNumber % 4;

    for (int i = 0; i < itCount; ++i)
    {
        if (!CallThreadFuncJob(destFolderAbs, 4))
        {
            HeapFree(GetProcessHeap(), 0, destFolderAbs);
            destFolderAbs = NULL;
            return 0;
        }
    }

    if (!CallThreadFuncJob(destFolderAbs, itRest))
    {
        HeapFree(GetProcessHeap(), 0, destFolderAbs);
        destFolderAbs = NULL;
        return 0;
    }

    HeapFree(GetProcessHeap(), 0, destFolderAbs);
    destFolderAbs = NULL;
    return 1;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    hModule = (HMODULE)hinstDLL;
    return TRUE;
}