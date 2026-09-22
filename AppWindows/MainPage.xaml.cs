using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace AppWindows
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            this.InitializeComponent();
        }

        private void DemoDevices()
        {
            var devices = new List<Objects.DiscoveryDeviceObject>
            {
                new Objects.DiscoveryDeviceObject
                {
                    Name = "Sigma 1",
                    ConenctivityType = Objects.DeviceConenctivityType.WiFi,
                    OperatingStatus = Objects.DeviceOperatingStatus.Connected,
                    Address = "192.168.1.100"
                },
                new Objects.DiscoveryDeviceObject
                {
                    Name = "Sigma 2",
                    ConenctivityType = Objects.DeviceConenctivityType.Zigbee,
                    OperatingStatus = Objects.DeviceOperatingStatus.Offline,
                    Address = "192.168.1.101"
                }
            };

            DiscoveryDeviceView.ItemsSource = devices;
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            DemoDevices();
        }
    }
}
