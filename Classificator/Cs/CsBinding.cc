
#include <Windows.h>
#include <string.h>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

using ProgressTeller = void (*)(int, int, int, const char *, const char *, void *);

using LPClassificator = int (*)(const char *, const char *, char ***, int *, ProgressTeller, void *);
using LPReleaseFilenameBuffer2D = void (*)(char ***, const int);
using LPGetClassificatorError = int (*)();

std::string ToStdString(System::String^% inputStr) {
    auto charPtr = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(inputStr);
    std::string s(reinterpret_cast<char*>(static_cast<void*>(charPtr)));
    System::Runtime::InteropServices::Marshal::FreeHGlobal(charPtr);
    return s;
}

delegate void ProgressTellerDelegate(System::Int32, System::Int32, System::Boolean, System::String^, System::String^, System::Object^);

ref struct TellerContext {
    ProgressTellerDelegate^ delegate;
    Object^ args;
};

ref class ClassificatorContext {
    public:
        static LPClassificator Classificator = nullptr;
        static LPReleaseFilenameBuffer2D ReleaseFilenameBuffer2D = nullptr;
        static LPGetClassificatorError GetClassificatorError = nullptr;
        static bool isLoaded = false;
        static System::Collections::Generic::Dictionary<System::UInt64, TellerContext^>^ tellerInstanceMapping = nullptr;
};


ref class Classificator {
    public:
        static bool LoadMd5Library();
        static System::Collections::Generic::List<System::String^>^ Classificate(System::String^, System::String^, ProgressTellerDelegate^, System::Object^);
};



bool Classificator::LoadMd5Library()
{
    if (ClassificatorContext::isLoaded)
    {
        throw gcnew System::Exception("Dynamic library has been loaded");
    }

    auto dllPath = fs::absolute("Classificator.dll").string();

    if (!fs::exists(dllPath))
    {
        throw gcnew System::Exception("Cannot get the target dll path, make sure Classificator.dll is in the same path");
    }

    auto dll = LoadLibrary(dllPath.data());

    if (dll == nullptr)
    {
        throw gcnew System::Exception("Cannot get the target dll path, make sure Classificator.dll is in the same path");
    }

    ClassificatorContext::Classificator = (LPClassificator)GetProcAddress(dll, "Classificator");
    ClassificatorContext::ReleaseFilenameBuffer2D = (LPReleaseFilenameBuffer2D)GetProcAddress(dll, "ReleaseFilenameBuffer2D");
    ClassificatorContext::GetClassificatorError = (LPGetClassificatorError)GetProcAddress(dll, "GetClassificatorError");


    if (ClassificatorContext::Classificator == nullptr || ClassificatorContext::ReleaseFilenameBuffer2D == nullptr || ClassificatorContext::GetClassificatorError == nullptr)
    {
        throw gcnew System::Exception("Cannot load dll functions, make sure Classificator.dll is a correct dll");
    }

    ClassificatorContext::isLoaded = true;
    ClassificatorContext::tellerInstanceMapping = gcnew System::Collections::Generic::Dictionary<System::UInt64, TellerContext^>();
    
    return true;
}

System::Collections::Generic::List<System::String^>^ Classificator::Classificate(System::String^ csSourceDir, System::String^ csDuplicateDestDir, ProgressTellerDelegate^ tellerDelegate, System::Object^ args){
    if (!ClassificatorContext::isLoaded)
    {
        throw gcnew System::Exception("Dynamic library has not loaded");
    }

    auto sourceDir = ToStdString(csSourceDir);

    char* duplicateDestDir = nullptr;

    if(csDuplicateDestDir != nullptr) {
        auto tmpDuplicateDestDir = ToStdString(csDuplicateDestDir);

        duplicateDestDir = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char) * tmpDuplicateDestDir.length() + 1);
        memcpy(duplicateDestDir, tmpDuplicateDestDir.data(), sizeof(char) * tmpDuplicateDestDir.length());
    }

    

    auto delegatePtrId = static_cast<System::UInt64>(reinterpret_cast<unsigned long long>(static_cast<void*>(System::Runtime::InteropServices::Marshal::GetFunctionPointerForDelegate(tellerDelegate))));
    
    auto LPDelegatePtrId = new System::UInt64(delegatePtrId);

    auto tellerContext = gcnew TellerContext();
    tellerContext->delegate = tellerDelegate;
    tellerContext->args = args;

    ClassificatorContext::tellerInstanceMapping->Add(delegatePtrId, tellerContext);

    char **duplicatedFiles = NULL;
    int duplicatedFilesCount = 0;

    bool isSuccess = ClassificatorContext::Classificator(
        sourceDir.data(),
        duplicateDestDir,
        &duplicatedFiles,
        &duplicatedFilesCount,
        [](int current, int total, int isDuplicated, const char *filename, const char *newFilename, void *LPDelegatePtrId) -> void {
            auto delegatePtrId = *(reinterpret_cast<System::UInt64*>(LPDelegatePtrId));
            auto tellerContext = ClassificatorContext::tellerInstanceMapping[delegatePtrId];

            tellerContext->delegate(
                static_cast<System::Int32>(current),
                static_cast<System::Int32>(total),
                static_cast<System::Boolean>(isDuplicated),
                gcnew System::String(filename),
                gcnew System::String(newFilename),
                tellerContext->args
            );

        },
        LPDelegatePtrId
        //
    );

    if(!isSuccess) {
        std::string errorMessage("Do Classificator failed, error code: ");
        errorMessage = errorMessage + std::to_string(ClassificatorContext::GetClassificatorError());

        ClassificatorContext::ReleaseFilenameBuffer2D(&duplicatedFiles, duplicatedFilesCount);
        ClassificatorContext::tellerInstanceMapping->Remove(delegatePtrId);
        delete LPDelegatePtrId;

        if(duplicateDestDir != nullptr) {
            HeapFree(GetProcessHeap(), 0 , duplicateDestDir);
        }

        throw gcnew System::Exception(gcnew System::String(errorMessage.data()));
    }


    auto duplicatedList = gcnew System::Collections::Generic::List<System::String^>();

    for(int i=0; i<duplicatedFilesCount; ++i) {
        duplicatedList->Add(gcnew System::String(duplicatedFiles[i]));
    }

    ClassificatorContext::ReleaseFilenameBuffer2D(&duplicatedFiles, duplicatedFilesCount);
    ClassificatorContext::tellerInstanceMapping->Remove(delegatePtrId);
    delete LPDelegatePtrId;

    if(duplicateDestDir != nullptr) {
        HeapFree(GetProcessHeap(), 0 , duplicateDestDir);
    }

    return duplicatedList;
}