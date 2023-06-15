using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace WindowsFormsApp1
{
    class Program
    {
        private Classificator Form;
        public Program() {
            Form = new Classificator();
            Form.Show();
        }

        [STAThread]
        public void RenderFrame() {
            Application.DoEvents();
        }

        public bool GetFormActivated() { 
            return Application.OpenForms.Count > 0;
        }
    }
}
