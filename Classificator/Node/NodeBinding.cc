#include <Windows.h>
#include <Napi.h>
#include <memory>
#include <string>
#include <string.h>
#include <unordered_map>

#include <iostream>

using ProgressTeller = void (*)(int, int, int, const char *, const char *, void *);

using LPClassificator = int (*)(const char *, const char *, char ***, int *, ProgressTeller, void *);
using LPReleaseFilenameBuffer2D = void (*)(char ***, const int);
using LPGetClassificatorError = int (*)();

using _ClassificatorContext = struct classificatorContext
{
    LPClassificator Classificator;
    LPReleaseFilenameBuffer2D ReleaseFilenameBuffer2D;
    LPGetClassificatorError GetClassificatorError;
    bool isLoaded;
};

using TellerContext = struct tellerContext
{
    Napi::Env env;
    Napi::Function jsCallback;
};

std::unordered_map<unsigned long long, std::unique_ptr<TellerContext>> TellerContextMapping;
_ClassificatorContext ClassificatorContext{nullptr, nullptr, nullptr, false};

Napi::Value Node_LoadDllLibrary(const Napi::CallbackInfo &info)
{
    auto env = info.Env();

    if (ClassificatorContext.isLoaded)
    {
        Napi::Error::New(env, "Dynamic library has been loaded").ThrowAsJavaScriptException();
        return env.Null();
    }

    auto dllPath = std::make_unique<char[]>(MAX_PATH);
    memset(dllPath.get(), '\0', sizeof(char) * MAX_PATH);

    if (!GetCurrentDirectory(MAX_PATH, const_cast<char *>(dllPath.get())))
    {
        Napi::Error::New(env, "Cannot get current directory").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (dllPath.get()[strlen(dllPath.get()) - 1] == '\\')
    {
        constexpr char *dllName = "Classificator.dll";
        memcpy(dllPath.get() + strlen(dllPath.get()), dllName, strlen(dllName));
    }
    else
    {
        constexpr char *dllName = "\\Classificator.dll";
        memcpy(dllPath.get() + strlen(dllPath.get()), dllName, strlen(dllName));
    }

    auto dll = LoadLibrary(dllPath.get());

    if (dll == nullptr)
    {
        Napi::Error::New(env, "Cannot get the target dll path, make sure Classificator.dll is in the same path").ThrowAsJavaScriptException();
        return env.Null();
    }

    ClassificatorContext.Classificator = (LPClassificator)GetProcAddress(dll, "Classificator");
    ClassificatorContext.GetClassificatorError = (LPGetClassificatorError)GetProcAddress(dll, "GetClassificatorError");
    ClassificatorContext.ReleaseFilenameBuffer2D = (LPReleaseFilenameBuffer2D)GetProcAddress(dll, "ReleaseFilenameBuffer2D");

    if (ClassificatorContext.Classificator == nullptr || ClassificatorContext.GetClassificatorError == nullptr || ClassificatorContext.ReleaseFilenameBuffer2D == nullptr)
    {
        Napi::Error::New(env, "Cannot load dll functions, make sure Classificator.dll is a correct dll").ThrowAsJavaScriptException();
        return env.Null();
    }

    ClassificatorContext.isLoaded = true;
    return env.Null();
}

// param: srcdir, dstdir, tellercallback
Napi::Value Node_Classificate(const Napi::CallbackInfo &info)
{
    auto env = info.Env();

    if (!ClassificatorContext.isLoaded)
    {
        Napi::Error::New(env, "Dynamic library not loaded").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (info.Length() < 3)
    {
        Napi::TypeError::New(env, "Wrong Arguments, except at lease 3 arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!info[0].IsString())
    {
        Napi::TypeError::New(env, "Wrong Argument, position 0 except a string").ThrowAsJavaScriptException();
        return env.Null();
    }

    auto srcDir = info[0].As<Napi::String>().Utf8Value();

    char *dstDir = nullptr;

    if (info[1].IsString())
    {
        auto dstTmp = info[1].As<Napi::String>().Utf8Value();
        dstDir = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char) * dstTmp.length() + 1);
        memcpy(dstDir, dstTmp.data(), sizeof(char) * dstTmp.length());
    }

    if (!info[2].IsFunction())
    {
        Napi::TypeError::New(env, "Wrong Argument, position 2 except a function").ThrowAsJavaScriptException();
        return env.Null();
    }

    auto jsTellerFunc = info[2].As<Napi::Function>();
    auto context = std::make_unique<TellerContext>(TellerContext{env, jsTellerFunc});
    auto funcId = new unsigned long long(reinterpret_cast<unsigned long long>(&jsTellerFunc));

    TellerContextMapping.insert(std::pair<unsigned long long, std::unique_ptr<TellerContext>>(*funcId, std::move(context)));

    char **duplicatedFiles = NULL;
    int duplicatedFilesCount = 0;

    int isSuccess = ClassificatorContext.Classificator(
        srcDir.data(),
        dstDir,
        &duplicatedFiles,
        &duplicatedFilesCount,
        [](int current, int total, int isDuplicated, const char *filename, const char *newFilename, void *funcId) -> void
        {
            auto &context = TellerContextMapping[*(reinterpret_cast<unsigned long long *>(funcId))];
            auto &jsTellerFunc = context->jsCallback;
            auto &env = context->env;

            auto jsNewFileNameStr = env.Null();

            if (newFilename != nullptr)
            {
                jsNewFileNameStr = Napi::String::New(env, newFilename);
            }

            jsTellerFunc.Call(
                env.Global(),
                {
                    Napi::Number::New(env, current),
                    Napi::Number::New(env, total),
                    Napi::Boolean::New(env, isDuplicated),
                    Napi::String::New(env, filename),
                    jsNewFileNameStr
                    //
                }
                //
            );
        },
        funcId
        //
    );

    if (!isSuccess)
    {
        std::string errMessage("Do Classificator failed, error code: ");
        errMessage = errMessage + std::to_string(ClassificatorContext.GetClassificatorError());

        Napi::Error::New(env, errMessage).ThrowAsJavaScriptException();

        ClassificatorContext.ReleaseFilenameBuffer2D(&duplicatedFiles, duplicatedFilesCount);
        TellerContextMapping.erase(*funcId);
        if (dstDir != nullptr)
        {
            HeapFree(GetProcessHeap(), 0, dstDir);
            dstDir = nullptr;
        }
        delete funcId;

        return env.Null();
    }

    auto jsDuplicatedFiles = Napi::Array::New(env, duplicatedFilesCount);
    for (int i = 0; i < duplicatedFilesCount; ++i)
    {
        jsDuplicatedFiles.Set(i, Napi::String::New(env, duplicatedFiles[i]));
    }

    ClassificatorContext.ReleaseFilenameBuffer2D(&duplicatedFiles, duplicatedFilesCount);
    TellerContextMapping.erase(*funcId);
    if (dstDir != nullptr)
    {
        HeapFree(GetProcessHeap(), 0, dstDir);
        dstDir = nullptr;
    }
    delete funcId;

    return jsDuplicatedFiles;
}

Napi::Object Initialize(Napi::Env env, Napi::Object exports)
{
    exports.Set(
        Napi::String::New(env, "LoadDllLibrary"),
        Napi::Function::New(env, Node_LoadDllLibrary)
        //
    );

    exports.Set(
        Napi::String::New(env, "Classificate"),
        Napi::Function::New(env, Node_Classificate)
        //
    );

    return exports;
}

NODE_API_MODULE(Classificator, Initialize)