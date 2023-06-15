
namespace WindowsFormsApp1
{
    partial class Classificator
    {
        /// <summary>
        /// 設計工具所需的變數。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 清除任何使用中的資源。
        /// </summary>
        /// <param name="disposing">如果應該處置受控資源則為 true，否則為 false。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form 設計工具產生的程式碼

        /// <summary>
        /// 此為設計工具支援所需的方法 - 請勿使用程式碼編輯器修改
        /// 這個方法的內容。
        /// </summary>
        private void InitializeComponent()
        {
            this.SelectFolder = new System.Windows.Forms.Button();
            this.Classificate = new System.Windows.Forms.Button();
            this.richTextBox1 = new System.Windows.Forms.RichTextBox();
            this.SuspendLayout();
            // 
            // SelectFolder
            // 
            this.SelectFolder.Location = new System.Drawing.Point(12, 301);
            this.SelectFolder.Name = "SelectFolder";
            this.SelectFolder.Size = new System.Drawing.Size(160, 40);
            this.SelectFolder.TabIndex = 0;
            this.SelectFolder.Text = "SelectFolder";
            this.SelectFolder.UseVisualStyleBackColor = true;
            this.SelectFolder.Click += new System.EventHandler(this.SelectFolder_Click);
            // 
            // Classificate
            // 
            this.Classificate.Enabled = false;
            this.Classificate.Location = new System.Drawing.Point(210, 301);
            this.Classificate.Name = "Classificate";
            this.Classificate.Size = new System.Drawing.Size(160, 40);
            this.Classificate.TabIndex = 1;
            this.Classificate.Text = "Classificate";
            this.Classificate.UseVisualStyleBackColor = true;
            this.Classificate.Click += new System.EventHandler(this.Classificate_Click);
            // 
            // richTextBox1
            // 
            this.richTextBox1.Location = new System.Drawing.Point(12, 12);
            this.richTextBox1.Name = "richTextBox1";
            this.richTextBox1.ReadOnly = true;
            this.richTextBox1.Size = new System.Drawing.Size(358, 272);
            this.richTextBox1.TabIndex = 2;
            this.richTextBox1.Text = "";
            this.richTextBox1.TextChanged += new System.EventHandler(this.richTextBox1_TextChanged);
            // 
            // Classificator
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(382, 353);
            this.Controls.Add(this.richTextBox1);
            this.Controls.Add(this.Classificate);
            this.Controls.Add(this.SelectFolder);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;
            this.Name = "Classificator";
            this.Text = "Classificator";
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button SelectFolder;
        private System.Windows.Forms.Button Classificate;
        private System.Windows.Forms.RichTextBox richTextBox1;
    }
}

