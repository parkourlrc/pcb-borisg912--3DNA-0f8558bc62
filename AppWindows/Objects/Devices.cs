using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.UI.Xaml.Controls;

namespace AppWindows.Objects
{
    public class DiscoveryDeviceObject
    {
        public string Name { get; set; }
        public DeviceConenctivityType ConenctivityType { get; set; }
        public DeviceOperatingStatus OperatingStatus { get; set; }
        public string Address { get; set; }
        public string SystemId { get; set; }

        public string GetDeviceConnectivityGlyph()
        {
            switch (ConenctivityType)
            {
                case DeviceConenctivityType.WiFi:
                    return "\uE701"; // Wi-Fi icon
                case DeviceConenctivityType.WiFiDirect:
                    return "\uE88A"; // Wi-Fi Direct icon
                case DeviceConenctivityType.WiFiDevMode:
                    return "\uEC7A"; // Developer Mode over Wi-Fi icon
                case DeviceConenctivityType.Ethernet:
                    return "\uE839"; // Ethernet icon
                case DeviceConenctivityType.EthernetDevMode:
                    return "\uEC7A"; // Developer Mode over Ethernet icon
                case DeviceConenctivityType.Bluetooth:
                    return "\uE702"; // Bluetooth icon
                case DeviceConenctivityType.BluetoothDevMode:
                    return "\uEC7A"; // Developer Mode over Bluetooth icon
                case DeviceConenctivityType.Zigbee:
                    return "\uE95A"; // Zigbee icon
                case DeviceConenctivityType.MatterOverIP:
                    return "\uEC26"; // Matter over IP icon
                case DeviceConenctivityType.MatterOverThread:
                    return "\uF0B9"; // Matter over Thread icon
                case DeviceConenctivityType.USB:
                    return "\uECF0"; // USB icon
                case DeviceConenctivityType.UsbDevMode:
                    return "\uEC7A"; // Developer Mode over USB icon
                default:
                    return "\uE9CE"; // Unknown connectivity type icon
            }
        }
        public string GetDeviceConnectivityString()
        {
            switch (ConenctivityType)
            {
                case DeviceConenctivityType.WiFi:
                    return "Wi-Fi";
                case DeviceConenctivityType.WiFiDirect:
                    return "Wi-Fi Direct";
                case DeviceConenctivityType.WiFiDevMode:
                    return "Developer Mode over Wi-Fi";
                case DeviceConenctivityType.Ethernet:
                    return "Ethernet";
                case DeviceConenctivityType.EthernetDevMode:
                    return "Developer Mode over Ethernet";
                case DeviceConenctivityType.Bluetooth:
                    return "Bluetooth";
                case DeviceConenctivityType.BluetoothDevMode:
                    return "Developer Mode over Bluetooth";
                case DeviceConenctivityType.Zigbee:
                    return "Zigbee";
                case DeviceConenctivityType.MatterOverIP:
                    return "Matter over Wi-Fi/Ethernet";
                case DeviceConenctivityType.MatterOverThread:
                    return "Matter over Thread";
                case DeviceConenctivityType.USB:
                    return "USB";
                case DeviceConenctivityType.UsbDevMode:
                    return "Developer Mode over USB";
                default:
                    return "Unknown";
            }
        }
    }
    public class WorkingDeviceObject
    {
        public string Name { get; set; }
        public string FriendlyName { get; set; }
        public string SystemId { get; set; }
        public string HardwareVersion { get; set; }
        public string SoftwareVersion { get; set; }
        public DeviceConenctivityType CurrentConenctivityType { get; set; }
        public DeviceConenctivityType[] SupportedConenctivityTypes { get; set; }
        public DeviceOperatingStatus OperatingStatus { get; set; }
        public string MACAddress { get; set; }
        public string IPAddress { get; set; }
        public string ConnectedWiFiSSID { get; set; }
        public string ConnectedMatterNetworkName { get; set; }
        public bool IsSmartHomeImplemented { get; set; } = false;
        public bool IsSmartHomeStandbyConfigured { get; set; } = false;
        public bool IsLongTermPaired { get; set; } = false;
        public bool IsSavedOnDevice { get; set; } = false;
        public bool IsAdminCredentialsSet { get; set; } = false;
        public SmartDeviceIntegrators[] SupportedIntegrators { get; set; }
        public bool IsWebCapabilitiesSupported { get; set; } = false;
        public bool IsWebCapabilitiesConfigured { get; set; } = false;
        public bool IsProximityBleAdvertisingSupported { get; set; } = false;
    }

    public enum DeviceConenctivityType
    {
        Unknown = 0,
        WiFi = 1,
        WiFiDirect = 2,
        WiFiDevMode = 3,
        Ethernet = 4,
        EthernetDevMode = 5,
        Bluetooth = 6,
        BluetoothDevMode = 7,
        Zigbee = 8,
        MatterOverIP = 9,
        MatterOverThread = 10,
        USB = 11,
        UsbDevMode = 12,
    }
    public enum DeviceOperatingStatus
    {
        Unknown = 0,
        Offline = 1,
        Setup = 2,
        WirelessSetup = 3,
        Connected = 4,
        OneTimeConnected = 5,
        DeveloperMode = 6,
    }
    public enum SmartDeviceIntegrators
    {
        GoogleHome = 0,
        appleHomeKit = 1,
        samsungSmartThings = 2,
        amazonAlexa = 3,
        huaweiHiLink = 4,
        xiaomiMiHome = 5,
        homeAssistant = 6,
        microsoftCortana = 7,
    }
}
