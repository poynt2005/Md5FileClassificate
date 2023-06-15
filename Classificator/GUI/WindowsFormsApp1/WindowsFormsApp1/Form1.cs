using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Reflection;
using System.IO;
using System.Windows.Forms;

namespace WindowsFormsApp1
{

    public delegate void ProgressTeller(int current, int total, bool isDuplicated, String filename, String newFilename);
    public partial class Classificator : Form
    {
        private String selectedFolder = null;

        private class ClassificatorContext
        {
            public MethodInfo ClassificateMethodInfo;
            public Type TellerDelegateType;
        };

        ClassificatorContext classificatorContext = null;

        public static void TellerFuncAdjuster(Int32 current, Int32 total, Boolean isDuplicated, String filename, String newFilename, Object args)
        {
            ProgressTeller teller = (ProgressTeller)args;
            teller(current, total, isDuplicated, filename, newFilename);
        }
    
        public void TellerFunc(int current, int total, bool isDuplicated, String filename, String newFilename) {

            String duplicatedMessage = "";
            if (isDuplicated) {
                duplicatedMessage = " , found duplicated and move it to: " + newFilename;
            }

            richTextBox1.Text = ($"({current / total * 100}%), processing: {filename}{duplicatedMessage}" + Environment.NewLine);
        }

        public Classificator()
        {
            InitializeComponent();
        }

        private bool LoadClassificatorLibrary() {
            if(classificatorContext != null)
            {
                return true;
            }

            String[] dependencies = { "Classificator.dll", "CsClassificator.dll", "md5.dll" };

            foreach (var dep in dependencies) {
                if (!File.Exists(Path.GetFullPath(dep))) {
                    MessageBox.Show("Error Loading DLL: " + dep +  " Load Failed, File Not Exists!!", "Dll Not Exists");
                    return false;
                }
            }

            Assembly asm;
            try
            {
                asm = Assembly.LoadFile(Path.GetFullPath(dependencies[1]));
            }
            catch (Exception ex) {
                MessageBox.Show("Error Loading DLL: " + dependencies[1] + " Load Failed, " + ex.ToString() + "!!", "Core Dll Load Failed");
                return false;
            }

            try
            {
                var LoadMd5LibraryMethodInfo = asm.GetType("Classificator").GetMethod("LoadMd5Library");
                LoadMd5LibraryMethodInfo.Invoke(null, null);
            }
            catch (Exception ex) {
                MessageBox.Show("Error Loading Native DLL: " + dependencies[1] + " Load Failed, " + ex.ToString() + "!!", "Native Core Dll Load Failed");
                return false;
            }

            classificatorContext = new ClassificatorContext();

            classificatorContext.ClassificateMethodInfo = asm.GetType("Classificator").GetMethod("Classificate");
            classificatorContext.TellerDelegateType = asm.GetType("ProgressTellerDelegate");

            return true;
        }

        private void richTextBox1_TextChanged(object sender, EventArgs e)
        {

        }

        private void SelectFolder_Click(object sender, EventArgs e)
        {
            selectedFolder = null;
            Classificate.Enabled = false;
            var fbd = new FolderBrowserDialog();
            var result = fbd.ShowDialog();

            if (result == DialogResult.OK && !string.IsNullOrWhiteSpace(fbd.SelectedPath))
            {
                selectedFolder = fbd.SelectedPath;
                Classificate.Enabled = true;

                richTextBox1.Text = "Selected Folder: " + selectedFolder + Environment.NewLine;
            }

            fbd.Dispose();
        }

        private void Classificate_Click(object sender, EventArgs e)
        {
            if (!LoadClassificatorLibrary())
            {
                return;
            }

            richTextBox1.Text = "";

            
            var teller = Delegate.CreateDelegate(classificatorContext.TellerDelegateType, typeof(Classificator).GetMethod("TellerFuncAdjuster"));

            var duplicatedStoreFolder = Path.GetFullPath("./Duplicated");

            Object[] classificateParams = { selectedFolder, duplicatedStoreFolder, teller, new ProgressTeller(TellerFunc) };
            var duplicatedFileResults = (List<String>)classificatorContext.ClassificateMethodInfo.Invoke(null, classificateParams);

            richTextBox1.Text += (Environment.NewLine + Environment.NewLine);
            richTextBox1.Text += ("///////////////////////////" + Environment.NewLine);
            richTextBox1.Text += ("Duplicated Files:" + Environment.NewLine);

            foreach (var result in duplicatedFileResults) {
                richTextBox1.Text += (result + Environment.NewLine);
            }
        }
    }
}
