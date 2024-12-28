using System;
using System.Windows.Forms;
using Microsoft.DirectX.DirectInput;
using System.Runtime.InteropServices;

namespace DeviceLister
{
    public partial class DeviceListerForm : Form
    {
        [DllImport("DIDeviceInputId.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern IntPtr GetDeviceInstanceID(string input);
        
        public DeviceListerForm()
        {
            InitializeComponent();
        }

        private void DeviceListerForm_Load(object sender, EventArgs e)
        {
            string deviceData = "";

            foreach (DeviceInstance di in Manager.GetDevices(DeviceClass.GameControl, EnumDevicesFlags.AttachedOnly))
            {
                string deviceInstanceId;
                IntPtr resultPtr = GetDeviceInstanceID(di.InstanceGuid.ToString());

                if (resultPtr == IntPtr.Zero) {
                    deviceInstanceId = "[failed to get device instance ID]";
                } else {
                    deviceInstanceId = Marshal.PtrToStringUni(resultPtr);
                }

				deviceData += "\"" + di.ProductName + "\": {" + di.InstanceGuid + "} <" + deviceInstanceId + ">" + System.Environment.NewLine;
            }

            textBox.Text = deviceData;
            textBox.Select(0, 0);
        }
    }
}
