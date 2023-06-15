$CsCode = @'
using System;
using System.Runtime.InteropServices;
using System.IO;
using System.Reflection;
using System.Collections.Generic;

namespace Utils {
    
    public delegate void ProgressTeller(int current, int total, bool isDuplicated, String filename, String newFilename);
    
    public class Utils {
        private class ClassificatorContext {
            public static Type tellerDelegateType = null;
            public static MethodInfo ClassificateMethod = null;
            public static bool isLoaded = false;
        };

        private static bool LoadClassificator(){
            if(ClassificatorContext.isLoaded) {
                return true;
            }

            try {
                var classificatorLibrary = Assembly.LoadFile(Path.GetFullPath("./CsClassificator.dll"));
                ClassificatorContext.tellerDelegateType = classificatorLibrary.GetType("ProgressTellerDelegate"); 
                var ClassificatorType = classificatorLibrary.GetType("Classificator");
                ClassificatorContext.ClassificateMethod = ClassificatorType.GetMethod("Classificate");

                var loadDllLibrary = ClassificatorType.GetMethod("LoadMd5Library");

                var isLoadDll = (bool)loadDllLibrary.Invoke(null, null);

                if(!isLoadDll) {
                    return false;
                }
                
                ClassificatorContext.isLoaded = true;
                return true;
            }
            catch(Exception e) {
                e.ToString();
                return false;
            }
        }

        public static void ProgressTellerAdjuster(int current, int total, bool isDuplicated, String filename, String newFilename, Object args) {
            ProgressTeller teller = (ProgressTeller)args;
            teller(current, total, isDuplicated, filename, newFilename);
        }

        public static String[] Classificate(String sourceDir, String duplicateDestDir, ProgressTeller teller){
            if(!LoadClassificator()) {
                throw new Exception("Cannot load dynamic library");
            }

            var tellerAdjuster = Delegate.CreateDelegate(ClassificatorContext.tellerDelegateType, typeof(Utils).GetMethod("ProgressTellerAdjuster"));
            
            Object[] classificateParams = { sourceDir, duplicateDestDir, tellerAdjuster, teller };
            var duplicated = (List<String>)ClassificatorContext.ClassificateMethod.Invoke(null, classificateParams);
            return duplicated.ToArray();
        }
    }
}

'@

Add-Type -TypeDefinition $CsCode -Language CSharp

function Classificate-File {
    param(
        [string]$sourceFileDirectory,
        [string]$destFileDirectory,
        [scriptblock]$tellerCallback
    )

    $teller = [Utils.ProgressTeller] {
        param(
            [int]$current,
            [int]$total,
            [bool]$isDuplicated,
            [string]$filename,
            [string]$newFilename
        )
    
        & $tellerCallback $current $total $isDuplicated $filename $newFilename
    }

    $duplicated = [Utils.Utils]::Classificate($sourceFileDirectory, $destFileDirectory, $teller);

    return $duplicated
}

Export-ModuleMember -Function Classificate-File