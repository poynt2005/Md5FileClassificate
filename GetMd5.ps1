$CsCode = @'

using System;
using System.Runtime.InteropServices;

namespace Utils {
    public class Utils {
        [DllImport("md5.dll", CharSet = CharSet.Ansi)]
        private static extern IntPtr Md5Hash(IntPtr message, UInt32 length);

        [DllImport("md5.dll", CharSet = CharSet.Ansi)]
        private static extern IntPtr Md5FileHash(IntPtr filename);

        [DllImport("md5.dll", CharSet = CharSet.Ansi)]
        private static extern void ReleaseHashString(ref IntPtr buffer);

        public static String Md5Hash(String message){
            UInt32 messageLength = (UInt32)message.Length;
            var messageStrPtr = Marshal.StringToHGlobalAnsi(message);
            var hashStrPtr = Md5Hash(messageStrPtr, messageLength);
            var hashStr = Marshal.PtrToStringAnsi(hashStrPtr);
            ReleaseHashString(ref hashStrPtr);
            Marshal.FreeHGlobal(messageStrPtr);
            return hashStr;
        }

        public static String Md5FileHash(String filename){
            var filenameStrPtr = Marshal.StringToHGlobalAnsi(filename);
            var hashStrPtr = Md5FileHash(filenameStrPtr);
            var hashStr = Marshal.PtrToStringAnsi(hashStrPtr);
            ReleaseHashString(ref hashStrPtr);
            Marshal.FreeHGlobal(filenameStrPtr);
            return hashStr;
        }
    }
}
'@

Add-Type -TypeDefinition $CsCode -Language CSharp

$TargetType = Read-Host "Enter what you want to get md5 hash, (1)InputString, (2)InputFile"

if($TargetType -eq "1") {
    $Str = Read-Host "Enter String"

    $hash = [Utils.Utils]::Md5Hash($Str)
    if(!$hash) {
        Write-Host "Get Md5 hash string error"
        exit
    }

    Write-Host "hash: $hash"
}
elseif($TargetType -eq "2") {
    $Filename = Read-Host "Enter filename"

    $hash = [Utils.Utils]::Md5FileHash($Filename)
    if(!$hash) {
        Write-Host "Get Md5 hash string error"
        exit
    }

    Write-Host "hash: $hash"
}
else {
    Write-Host "Error Type"
}

