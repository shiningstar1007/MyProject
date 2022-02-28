using Microsoft.Win32;
using MyCSharp.Service;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Management;
using System.Net;
using System.Security.AccessControl;
using System.Security.Principal;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace MyCSharpService
{
    class CommFunc
    {
        public static RUN_MODE StrToRunMode(String RunModeStr)
        {
            if (RunModeStr == RUN_MODE_STR.RUN_TEST_STR) return RUN_MODE.RUN_TEST;
            else if (RunModeStr == RUN_MODE_STR.RUN_DISABLE_STR) return RUN_MODE.RUN_DISABLE;

            return RUN_MODE.RUN_NORMAL;
        }
        public static String RunModeToStr(RUN_MODE RunMode)
        {
            if (RunMode == RUN_MODE.RUN_TEST) return RUN_MODE_STR.RUN_TEST_STR;
            else if (RunMode == RUN_MODE.RUN_DISABLE) return RUN_MODE_STR.RUN_DISABLE_STR;

            return RUN_MODE_STR.RUN_NORMAL_STR;
        }
        public static SUB_TYPE StrToSubType(String SubTypeStr)
        {
            if (SubTypeStr == SUB_TYPE_STR.SUB_USER_STR) return SUB_TYPE.SUB_USER;
            else if (SubTypeStr == SUB_TYPE_STR.SUB_PROC_STR) return SUB_TYPE.SUB_PROC;
            else if (SubTypeStr == SUB_TYPE_STR.SUB_SHARE_STR) return SUB_TYPE.SUB_SHARE;
            else if (SubTypeStr == SUB_TYPE_STR.SUB_GROUP_STR) return SUB_TYPE.SUB_GROUP;

            return SUB_TYPE.SUB_UNKNOWN;
        }
        public static String SubTypeToStr(SUB_TYPE SubType)
        {
            if (SubType == SUB_TYPE.SUB_USER) return SUB_TYPE_STR.SUB_USER_STR;
            else if (SubType == SUB_TYPE.SUB_PROC) return SUB_TYPE_STR.SUB_PROC_STR;
            else if (SubType == SUB_TYPE.SUB_SHARE) return SUB_TYPE_STR.SUB_SHARE_STR;
            else if (SubType == SUB_TYPE.SUB_GROUP) return SUB_TYPE_STR.SUB_GROUP_STR;

            return "";
        }
        public static OBJ_TYPE StrToObjType(String ObjTypeStr)
        {
            if (ObjTypeStr == OBJ_TYPE_STR.OBJ_FILE_STR) return OBJ_TYPE.OBJ_FILE;
            else if (ObjTypeStr == OBJ_TYPE_STR.OBJ_DIR_STR) return OBJ_TYPE.OBJ_DIR;

            return OBJ_TYPE.OBJ_UNKNOWN;
        }
        public static String ObjTypeToStr(OBJ_TYPE ObjType)
        {
            if (ObjType == OBJ_TYPE.OBJ_FILE) return OBJ_TYPE_STR.OBJ_FILE_STR;
            else if (ObjType == OBJ_TYPE.OBJ_DIR) return OBJ_TYPE_STR.OBJ_DIR_STR;

            return "";
        }
        public static EFFECT_MODE StrToEffectMode(String EffectModeStr)
        {
            if (EffectModeStr == EFFECT_MODE_STR.EFT_ALLOW_STR) return EFFECT_MODE.EFT_ALLOW;
            else if (EffectModeStr == EFFECT_MODE_STR.EFT_DENY_STR) return EFFECT_MODE.EFT_DENY;

            return EFFECT_MODE.EFT_UNKNOWN;
        }
        public static String EffectModeToStr(EFFECT_MODE EffectMode)
        {
            if (EffectMode == EFFECT_MODE.EFT_ALLOW) return EFFECT_MODE_STR.EFT_ALLOW_STR;
            else if (EffectMode == EFFECT_MODE.EFT_DENY) return EFFECT_MODE_STR.EFT_DENY_STR;

            return "";
        }
        public static LOGGING_TYPE StrToLoggingType(String LoggingTypeStr)
        {
            if (LoggingTypeStr == LOGGING_TYPE_STR.LOG_ALLOW_STR) return LOGGING_TYPE.LOG_ALLOW;
            else if (LoggingTypeStr == LOGGING_TYPE_STR.LOG_DENY_STR) return LOGGING_TYPE.LOG_DENY;

            return LOGGING_TYPE.LOG_ALL;
        }
        public static String LoggingTypeToStr(LOGGING_TYPE LoggingType)
        {
            if (LoggingType == LOGGING_TYPE.LOG_ALLOW) return LOGGING_TYPE_STR.LOG_ALLOW_STR;
            else if (LoggingType == LOGGING_TYPE.LOG_DENY) return LOGGING_TYPE_STR.LOG_DENY_STR;

            return LOGGING_TYPE_STR.LOG_ALL_STR;
        }
        public static SHARED_PERM StrToSharedPerm(String SharedPermStr)
        {
            if (SharedPermStr == SHARED_PERM_STR.SDP_DECRYPT_STR) return SHARED_PERM.SDP_DECRYPT;
            else if (SharedPermStr == SHARED_PERM_STR.SDP_ENCRYPT_STR) return SHARED_PERM.SDP_ENCRYPT;
            else if (SharedPermStr == SHARED_PERM_STR.SDP_DENY_STR) return SHARED_PERM.SDP_DENY;

            return SHARED_PERM.SDP_UNKNOWN;
        }
        public static String SharedPermToStr(SHARED_PERM SharedPerm)
        {
            if (SharedPerm == SHARED_PERM.SDP_DECRYPT) return SHARED_PERM_STR.SDP_DECRYPT_STR;
            else if (SharedPerm == SHARED_PERM.SDP_ENCRYPT) return SHARED_PERM_STR.SDP_ENCRYPT_STR;
            else if (SharedPerm == SHARED_PERM.SDP_DENY) return SHARED_PERM_STR.SDP_DENY_STR;

            return "";
        }
        public static ONOFF_MODE StrToOnOffMode(String OnOffModeStr)
        {
            if (OnOffModeStr == ONOFF_MODE_STR.OFM_ON_STR) return ONOFF_MODE.OFM_ON;

            return ONOFF_MODE.OFM_OFF;
        }
        public static String OnOffModeToStr(ONOFF_MODE OnOffMode)
        {
            if (OnOffMode == ONOFF_MODE.OFM_ON) return ONOFF_MODE_STR.OFM_ON_STR;

            return ONOFF_MODE_STR.OFM_OFF_STR;
        }
        public enum ACL_ACTION : UInt32
        {
            ACT_READ = 0x00000001,
            ACT_WRITE = 0x00000002,
            ACT_TRAVERSE = 0x00000004,
            ACT_EXECUTE = 0x00000008,
            ACT_DELETE = 0x00000010,
            ACT_CREATE = 0x00000020,
            ACT_DECRYPT = 0x00000040,
            ACT_ENCRYPT = 0x00000080,
            ACT_ALL = 0xFFFFFFFF
        }
        public static class ACL_ACTION_STR
        {
            public const String ACT_READ_STR = "read";
            public const String ACT_WRITE_STR = "write";
            public const String ACT_TRAVERSE_STR = "traverse";
            public const String ACT_EXECUTE_STR = "execute";
            public const String ACT_DELETE_STR = "delete";
            public const String ACT_CREATE_STR = "create";
            public const String ACT_DECRYPT_STR = "decrypt";
            public const String ACT_ENCRYPT_STR = "encrypt";
            public const String ACT_ALL_STR = "all";
        }
        public static UInt32 StrToAction(String ActionStr)
        {
            String[] ActionBuf;
            UInt32 Action = 0;

            if (ActionStr == ACL_ACTION_STR.ACT_ALL_STR) return (UInt32)ACL_ACTION.ACT_ALL;

            ActionBuf = ActionStr.Split(new char[] { ',' });

            foreach (var data in ActionBuf)
            {
                if (data == ACL_ACTION_STR.ACT_READ_STR) Action |= (UInt32)ACL_ACTION.ACT_READ;
                else if (data == ACL_ACTION_STR.ACT_WRITE_STR) Action |= (UInt32)ACL_ACTION.ACT_WRITE;
                else if (data == ACL_ACTION_STR.ACT_TRAVERSE_STR) Action |= (UInt32)ACL_ACTION.ACT_TRAVERSE;
                else if (data == ACL_ACTION_STR.ACT_EXECUTE_STR) Action |= (UInt32)ACL_ACTION.ACT_EXECUTE;
                else if (data == ACL_ACTION_STR.ACT_DELETE_STR) Action |= (UInt32)ACL_ACTION.ACT_DELETE;
                else if (data == ACL_ACTION_STR.ACT_CREATE_STR) Action |= (UInt32)ACL_ACTION.ACT_CREATE;
                else if (data == ACL_ACTION_STR.ACT_DECRYPT_STR) Action |= (UInt32)ACL_ACTION.ACT_DECRYPT;
                else if (data == ACL_ACTION_STR.ACT_ENCRYPT_STR) Action |= (UInt32)ACL_ACTION.ACT_ENCRYPT;
            }

            return Action;
        }
        public static String ActionToStr(UInt32 Action)
        {
            var ActionStr = new StringBuilder();

            if (Action == (UInt32)ACL_ACTION.ACT_ALL) return ACL_ACTION_STR.ACT_ALL_STR;
            else
            {
                if ((Action & (UInt32)ACL_ACTION.ACT_READ) == (UInt32)ACL_ACTION.ACT_READ) ActionStr.Append(ACL_ACTION_STR.ACT_READ_STR);
                if ((Action & (UInt32)ACL_ACTION.ACT_WRITE) == (UInt32)ACL_ACTION.ACT_WRITE) ActionStr.Append("," + ACL_ACTION_STR.ACT_WRITE_STR);
                if ((Action & (UInt32)ACL_ACTION.ACT_TRAVERSE) == (UInt32)ACL_ACTION.ACT_TRAVERSE) ActionStr.Append("," + ACL_ACTION_STR.ACT_TRAVERSE_STR);
                if ((Action & (UInt32)ACL_ACTION.ACT_EXECUTE) == (UInt32)ACL_ACTION.ACT_EXECUTE) ActionStr.Append("," + ACL_ACTION_STR.ACT_EXECUTE_STR);
                if ((Action & (UInt32)ACL_ACTION.ACT_DELETE) == (UInt32)ACL_ACTION.ACT_DELETE) ActionStr.Append("," + ACL_ACTION_STR.ACT_DELETE_STR);
                if ((Action & (UInt32)ACL_ACTION.ACT_CREATE) == (UInt32)ACL_ACTION.ACT_CREATE) ActionStr.Append("," + ACL_ACTION_STR.ACT_CREATE_STR);
                if ((Action & (UInt32)ACL_ACTION.ACT_DECRYPT) == (UInt32)ACL_ACTION.ACT_DECRYPT) ActionStr.Append("," + ACL_ACTION_STR.ACT_DECRYPT_STR);
                if ((Action & (UInt32)ACL_ACTION.ACT_ENCRYPT) == (UInt32)ACL_ACTION.ACT_ENCRYPT) ActionStr.Append("," + ACL_ACTION_STR.ACT_ENCRYPT_STR);
            }

            return ActionStr.ToString();
        }
        public Boolean checkWindowsGreater(int nMajor, int nMinor)
        {
            int MajorVer = Environment.OSVersion.Version.Major;
            int MinorVer = Environment.OSVersion.Version.Minor;

            return (nMajor >= MajorVer && nMinor >= MinorVer) ? true : false;
        }
        public static SecurityIdentifier GetUserSId(String UserName)
        {
            NTAccount Account = new NTAccount(UserName);
            SecurityIdentifier UserSId = (SecurityIdentifier)Account.Translate(typeof(SecurityIdentifier));

            return UserSId;
        }
        public static UInt64 GetSIdKey(SecurityIdentifier UserSId)
        {
            String[] SidStr = UserSId.ToString().Split(new char[] { '-' });
            UInt64 SIdKey = 0;

            foreach (var data in SidStr)
            {
                try
                {
                    UInt64 Sid = UInt64.Parse(data);
                    SIdKey += Sid;
                }
                catch { }
            }

            return SIdKey;
        }
        public static UInt64 GetObjKey(OBJ_TYPE ObjType, String ObjPath)
        {
            IntPtr hFile;
            NativeAPI.BY_HANDLE_FILE_INFORMATION FileInfo = new NativeAPI.BY_HANDLE_FILE_INFORMATION();
            UInt64 ObjKey = 0;

            hFile = NativeAPI.CreateFile(ObjPath, NativeAPI.GENERIC_READ, FileShare.Read, IntPtr.Zero, FileMode.Open, 0, IntPtr.Zero);
            if (hFile.ToInt32() != NativeAPI.INVALID_HANDLE_VALUE)
            {
                if (NativeAPI.GetFileInformationByHandle(hFile, out FileInfo))
                {
                    ObjKey = ((UInt64)FileInfo.FileIndexHigh << 32) | (UInt64)FileInfo.FileIndexLow;
                }

                NativeAPI.CloseHandle(hFile);
            }

            return ObjKey;
        }
        public UInt64 getStrKey(String objPath)
        {
            UInt64 strKey = 0;
            char[] PathArr = objPath.ToCharArray();

            foreach (var data in PathArr)
            {
                if (data >= 'a' && data <= 'z') strKey += (UInt64)(data - 32);
                else strKey += data;
            }

            return strKey;
        }
        public static Boolean CheckProcessExt(String procExt)
        {
            Boolean bExeFile = false;
            String buffer;

            try
            {
                using (RegistryKey regKey = Registry.ClassesRoot.OpenSubKey(procExt, true))
                {
                    buffer = (string)regKey.GetValue("");
                    if (String.Equals(buffer, "exefile")) bExeFile = true;
                }
            }
            catch { }

            return bExeFile;
        }
        public static List<String> userList()
        {
            SelectQuery userQuery = new SelectQuery("Win32_UserAccount");
            ManagementObjectSearcher searcher = new ManagementObjectSearcher(userQuery);
            List<string> userNameList = new List<string>();

            foreach (ManagementObject envVar in searcher.Get())
            {
                userNameList.Add((String)envVar["Name"]);
            }

            return userNameList;
        }
        public static List<String> GetGroupList()
        {
            SelectQuery userQuery = new SelectQuery("Win32_Group");
            ManagementObjectSearcher searcher = new ManagementObjectSearcher(userQuery);
            List<string> groupNameList = new List<string>();

            foreach (ManagementObject envVar in searcher.Get())
            {
                groupNameList.Add((String)envVar["Name"]);
            }

            return groupNameList;
        }
        public static List<String> GetCurrentProcessList()
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

        public static bool CheckIPType(string path)
        {
            Regex regex = new Regex(@"^\\\\(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])");

            return regex.IsMatch(path);
        }

        public static bool CheckLocalDrive(string path)
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

        public static string executeCMD(string textCMD)
        {
            ProcessStartInfo pri = new ProcessStartInfo();
            Process pro = new Process();

            pri.FileName = @"cmd.exe";
            pri.CreateNoWindow = false;
            pri.UseShellExecute = false;

            pri.RedirectStandardInput = true;
            pri.RedirectStandardOutput = true;
            pri.RedirectStandardError = true;

            pro.StartInfo = pri;
            pro.Start();

            pro.StandardInput.Write(textCMD + Environment.NewLine);
            pro.StandardInput.Close();

            System.IO.StreamReader sr = pro.StandardError;

            string resultValue = sr.ReadToEnd();
            pro.WaitForExit();
            pro.Close();

            return resultValue == "" ? "" : resultValue;
        }

        public bool checkDiskSize(string fileName, long fileSize)
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

        public int ConnectRemoteServerStart(string server, string netUserId, string netPwd)
        {
            int index, retValue = 0;
            string tempName = server.Substring(2);
            string[] hostName = tempName.Split(new char[] { '\\' });

            string Msg = executeCMD(string.Format("net use {0} /user:{1}\\{2} {3} /PERSISTENT:YES", server, hostName[0], netUserId, netPwd));

            if (string.IsNullOrEmpty(Msg) == false)
            {
                index = Msg.IndexOf("86");
                if (index != -1) retValue = 86; //account error
                else
                {
                    index = Msg.IndexOf("1326");
                    if (index != -1) retValue = 86; //account error
                    else retValue = 53; //network error
                }
            }
            return retValue;
        }

        public void CloseRemoteServerEnd(string server)
        {
            executeCMD(string.Format("net use /delete {0}", server));
        }

        public bool CheckDirectoryInfo(string path)
        {
            bool bSetProperties = false;

            DirectoryInfo checkDir = new DirectoryInfo(path);
            if ((checkDir.Attributes & FileAttributes.Compressed) == FileAttributes.Compressed)
            {
                bSetProperties = true;
            }
            else if ((checkDir.Attributes & FileAttributes.Encrypted) == FileAttributes.Encrypted)
            {
                bSetProperties = true;
            }

            return bSetProperties;
        }

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
    }
}
