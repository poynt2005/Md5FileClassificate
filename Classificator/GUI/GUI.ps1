$CsCode = @'

using System;
using System.Reflection;
using System.IO;

namespace Utils {
    public class Utils {
        private static Object FormProgramInstance = null;
        private static MethodInfo RenderFormMethodInfo = null;
        private static MethodInfo GetFormOpenedMethodInfo = null;

        public static bool OpenForm(){
            if(FormProgramInstance != null) {
                return false;
            }

            if(FormProgramInstance == null) {
                var asm = Assembly.LoadFile(Path.GetFullPath("Form.dll"));
                Console.WriteLine(asm);
                var FormProgramType = asm.GetType("WindowsFormsApp1.Program");
                
                FormProgramInstance = Activator.CreateInstance(FormProgramType);
                
                RenderFormMethodInfo = FormProgramType.GetMethod("RenderFrame");
                GetFormOpenedMethodInfo = FormProgramType.GetMethod("GetFormActivated");

            }
            return true;
        }
        
        public static void RenderForm(){
            RenderFormMethodInfo.Invoke(FormProgramInstance, null);
        }

        public static bool GetFormOpened(){
            return (bool)GetFormOpenedMethodInfo.Invoke(FormProgramInstance, null);
        }
    }
}

'@

Add-Type -TypeDefinition $CsCode -Language CSharp

if([Utils.Utils]::OpenForm()) {
    while([Utils.Utils]::GetFormOpened()) {
        [Utils.Utils]::RenderForm()
    }
}
else {
    Write-Host "Open WinForm Failed"
}