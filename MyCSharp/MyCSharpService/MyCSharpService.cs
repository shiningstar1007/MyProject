using System;
using System.Collections.Generic;
using System.Text;
using System.ServiceModel;
using System.Runtime.InteropServices;
using System.IO;
using System.Security.AccessControl;
using System.Text.RegularExpressions;
using System.Linq;
using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
using System.Management;
using System.Security.Principal;
using Microsoft.Win32;
using MyCSharpService;

namespace MyCSharp.Service
{
    public class Win32API
    {
        public bool CheckDiskSize(string fileName, long fileSize)
        {
            var drvs = DriveInfo.GetDrives().Where(e => e.IsReady && (e.DriveType == DriveType.Fixed));

            foreach (DriveInfo drv in drvs)
            {
                if (char.ToLower(drv.Name[0]) == char.ToLower(fileName[0]))
                {
                    if (fileSize >= drv.AvailableFreeSpace) return false;
                }
            }

            return true;
        }

        public static string GetUNCPathFromHostName(string path)
        {
            string hostName = "";
            Regex regex = new Regex(@"^\\\\(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])");

            if (regex.IsMatch(path))
            {
                string ipName = path.Substring(2);
                string[] name = ipName.Split(new char[] { '\\' });

                try
                {
                    IPHostEntry IpHost = Dns.GetHostEntry(name[0]);
                    hostName = @"\\" + IpHost.HostName + path.Substring(2 + name[0].Length);

                }
                catch (System.Net.Sockets.SocketException e)
                {

                }
            }

            return hostName;
        }


        [DllImport("mpr.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        public static extern int WNetGetConnection([MarshalAs(UnmanagedType.LPTStr)] string localName,
                                                [MarshalAs(UnmanagedType.LPTStr)] StringBuilder remoteName,
                                                ref int length);
        public static string GetUNCPath(string originalPath)
        {
            StringBuilder sb = new StringBuilder(512);
            int size = sb.Capacity;

            if (originalPath.Length > 2 && originalPath[1] == ':')
            {
                char c = originalPath[0];
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
                {
                    int error = WNetGetConnection(originalPath.Substring(0, 2), sb, ref size);
                    if (error == 0)
                    {
                        DirectoryInfo dir = new DirectoryInfo(originalPath);
                        string path = System.IO.Path.GetFullPath(originalPath).Substring(System.IO.Path.GetPathRoot(originalPath).Length);
                        return System.IO.Path.Combine(sb.ToString().TrimEnd(), path);
                    }
                }
            }

            return string.Empty;
        }
        public static string GetIPInfo()
        {
            string IPString = "";

            try
            {
                IPHostEntry ipHostInfo = Dns.GetHostEntry(Dns.GetHostName());
                IPAddress ipAddress = ipHostInfo.AddressList.FirstOrDefault(
                                        a => a.AddressFamily == AddressFamily.InterNetwork);

                IPString = ipAddress != null ? ipAddress.ToString() : "";
            }
            catch (Exception ex)
            {
            }

            return IPString;
        }

        public static List<string> GetProcessInfo()
        {
            ManagementClass management = new ManagementClass("Win32_Process");
            ManagementObjectCollection mCollection = management.GetInstances();

            List<string> processList = new List<string>();

            foreach (ManagementObject ps in mCollection)
            {
                if ((string)ps["ExecutablePath"] != null)
                {
                    processList.Add((string)ps["ExecutablePath"]);
                }
            }

            return processList;
        }

        public static bool IsLocalDrive(string path)
        {
            var drvs = DriveInfo.GetDrives().Where(e => e.IsReady && (e.DriveType == DriveType.Fixed));

            foreach (DriveInfo drv in drvs)
            {
                if (char.ToLower(drv.Name[0]) == char.ToLower(path[0]))
                {
                    return true;
                }
            }

            return false;
        }

        public static bool CreateDir(string dirPath)
        {
            if (Directory.Exists(dirPath))
            {
                return false;
            }

            Directory.CreateDirectory(dirPath);

            DirectoryInfo dInfo = new DirectoryInfo(dirPath);
            DirectorySecurity dSecurity = dInfo.GetAccessControl();
            dSecurity.AddAccessRule(new FileSystemAccessRule(new SecurityIdentifier(WellKnownSidType.WorldSid, null),
                                                            FileSystemRights.FullControl,
                                                            InheritanceFlags.ObjectInherit | InheritanceFlags.ContainerInherit,
                                                            PropagationFlags.NoPropagateInherit,
                                                            AccessControlType.Allow));
            dInfo.SetAccessControl(dSecurity);

            return true;
        }

        public string MyReadFile(string fileName, int length)
        {
            byte[] buffer = new byte[length];

            using (FileStream fs = new FileStream(fileName, FileMode.Open, FileSystemRights.Read, FileShare.Read, (length - 1), FileOptions.Asynchronous))
            {
                fs.Read(buffer, 0, length);
            }

            return Encoding.Default.GetString(buffer);
        }

        public void MyWriteFile(string fileName, string data, int length)
        {
            using (FileStream fs = new FileStream(fileName, FileMode.Append))
            {
                using (StreamWriter sw = new StreamWriter(fs))
                {
                    sw.Write(data);
                }
            }
        }

        public void CreateRegistrySubKeyValueString(string subKeyName, string keyName, object data)
        {
            using (RegistryKey regKey = Registry.LocalMachine.CreateSubKey(subKeyName))
            {
                regKey.SetValue(keyName, (string)data, RegistryValueKind.String);
            }
        }

        public void CreateRegistrySubKeyValueType(RegistryValueKind regType, string subKeyName, string keyName, object data)
        {
            using (RegistryKey regKey = Registry.LocalMachine.CreateSubKey(subKeyName))
            {
            }
            if (data != null)
            {
                if (regType == RegistryValueKind.DWord)
                {
                    SetRegistryKeyValueDWORD(subKeyName, keyName, data);
                }
                else if (regType == RegistryValueKind.Binary)
                {
                    SetRegistryKeyValueBinary(subKeyName, keyName, data);
                }
            }
        }

        public void SetRegistryKeyValueString(string regPath, string keyName, object data)
        {
            if (data != null)
            {
                using (RegistryKey regKey = Registry.LocalMachine.OpenSubKey(regPath, true))
                {
                    regKey.SetValue(keyName, (string)data, RegistryValueKind.String);
                }
            }
        }

        public void SetRegistryKeyValueDWORD(string regPath, string keyName, object data)
        {
            if (data != null)
            {
                using (RegistryKey regKey = Registry.LocalMachine.OpenSubKey(regPath, true))
                {
                    regKey.SetValue(keyName, (int)data, RegistryValueKind.DWord);
                }
            }
        }

        public void SetRegistryKeyValueBinary(string regPath, string keyName, object data)
        {
            if (data != null)
            {
                using (RegistryKey regKey = Registry.LocalMachine.OpenSubKey(regPath, true))
                {
                    regKey.SetValue(keyName, (byte)data, RegistryValueKind.Binary);
                }
            }
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
        private struct NETRESOURCE
        {
            public uint dwScope;
            public uint dwType;
            public uint dwDisplayType;
            public uint dwUsage;
            public string lpLocalName;
            public string lpRemoteName;
            public string lpComment;
            public string lpProvider;
        }
        const int RESOURCETYPE_DISK = 0x00000001;
        const int CONNECT_TEMPORARY = 0x00000004;
        const int ERROR_NO_ADMIN_INFO = 13120;
        const int ERROR_NO_SUCH_LOGON_SESSION = 1312;
        const int ERROR_SESSION_CREDENTIAL_CONFLICT = 1219;

        private const int RESOURCETYPE_ANY = 0x0;
        private const int CONNECT_INTERACTIVE = 0x00000008;
        private const int CONNECT_PROMPT = 0x00000010;
        private const int CONNECT_UPDATE_PROFILE = 0x00000001;

        // API 함수 선언
        [DllImport("mpr.dll", CharSet = CharSet.Auto)]
        private static extern int WNetAddConnection2(ref NETRESOURCE netResource,
           string password, string username, uint flags);

        // API 함수 선언 (공유해제)
        [DllImport("mpr.dll", EntryPoint = "WNetCancelConnection2", CharSet = CharSet.Auto)]
        private static extern int WNetCancelConnection2(string lpName, int dwFlags, int fForce);

        public static int NetWorkDriveConnect(string serverName, string userName, string userPwd)
        {
            NETRESOURCE netResource = new NETRESOURCE();
            netResource.dwType = RESOURCETYPE_DISK;
            netResource.lpRemoteName = serverName;
            netResource.lpProvider = "";

            int returnCode = WNetAddConnection2(ref netResource, userPwd, userName, 0);


            return returnCode;
        }

        public static int NetWorkDriveDisConnect(string serverName)
        {
            int returnCode = WNetCancelConnection2(serverName, CONNECT_UPDATE_PROFILE, 1);

            return returnCode;
        }

    }

    [ServiceContract()]
    public interface IMyCSharpService
    {
        [OperationContract()]
        string testFunc();
    }
    [ServiceBehavior(InstanceContextMode = InstanceContextMode.Single)]
    public class MyCSharpService : IMyCSharpService
    {
        public string testFunc()
        {
            return "testFunc";
        }
    }
}
