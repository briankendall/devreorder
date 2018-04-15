using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Microsoft.DirectX.DirectInput;

namespace DeviceLister
{
    public partial class DeviceListerForm : Form
    {
        public DeviceListerForm()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            string deviceData = "";

            foreach (DeviceInstance di in Manager.GetDevices(DeviceClass.GameControl, EnumDevicesFlags.AttachedOnly))
            {
                deviceData += "\"" + di.ProductName + "\": {" + di.InstanceGuid + "}" + System.Environment.NewLine;
            }

            textBox.Text = deviceData;
            textBox.Select(0, 0);
        }

        private void textBox_TextChanged(object sender, EventArgs e)
        {

        }
    }
}
