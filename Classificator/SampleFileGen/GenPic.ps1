$CsCode = @'

using System;
using System.Runtime.InteropServices;

namespace Utils {
    public class Utils {
        
        [DllImport("Gen.dll", CharSet = CharSet.Ansi)]
        private static extern bool Generate(IntPtr destFolder, int totalNumber);
        
        public static bool GenPic(String destFolder, int picNumber) {
            var destFolderStrPtr = Marshal.StringToHGlobalAnsi(destFolder);
            
            if(!Generate(destFolderStrPtr, picNumber)) {
                Marshal.FreeHGlobal(destFolderStrPtr);
                return false;
            }

            Marshal.FreeHGlobal(destFolderStrPtr);

            return true;
        }
    }
}

'@

Add-Type -TypeDefinition $CsCode -Language CSharp

$picNum = Read-Host "Enter How many pic you want to generate"
$picNum = [System.Int32]::Parse($picNum)


$isGened = [Utils.Utils]::GenPic("GenPics", $picNum)

if(!$isGened) {
    Write-Host "Generated pictures failed"
}
else {
    Write-Host "Generated pictures"
}