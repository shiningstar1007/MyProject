using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.ConstrainedExecution;
using System.Runtime.InteropServices;
using System.Text;
using System.IO;
using RGiesecke.DllExport;

namespace MyCSharp.Service
{
    class NativeAPI
    {
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

        [DllImport("mpr.dll", CharSet = CharSet.Auto)]
        private static extern int WNetAddConnection2(ref NETRESOURCE netResource,
           string password, string username, uint flags);

        [DllImport("mpr.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        public static extern int WNetGetConnection([MarshalAs(UnmanagedType.LPTStr)] string localName,
                                                [MarshalAs(UnmanagedType.LPTStr)] StringBuilder remoteName,
                                                ref int length);

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

        [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        public static extern IntPtr CreateFile(
            String fileName,
            int dwDesiredAccess,
            FileShare dwShareMode,
            IntPtr securityAttrs,
            FileMode dwCreationDisposition,
            int dwFlagsAndAttributes,
            IntPtr hTemplateFile
            );

        [DllImport("kernel32", SetLastError = true)]
        [ReliabilityContract(Consistency.WillNotCorruptState, Cer.MayFail)]
        public extern static bool CloseHandle(IntPtr FileHandle);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool GetFileInformationByHandle(
            IntPtr hFile,
            out BY_HANDLE_FILE_INFORMATION lpFileInformation
        );

        [DllImport("FltLib", SetLastError = true)]
        public static extern Int16 FilterConnectCommunicationPort(
            [MarshalAs(UnmanagedType.LPWStr)] String lpPortName,
            UInt16 dwOptions,
            IntPtr lpContext,
            UInt16 wSizeOfContext,
            IntPtr lpSecurityAttributes,
            out IntPtr hPort);

        [DllImport("fltlib", SetLastError = true)]
        public static extern int FilterGetMessage(
            IntPtr hPort,
            IntPtr msgBuffer,
            uint msgBufSize,
            IntPtr lpOverlapped
        );

        [DllImport("FltLib", CallingConvention = CallingConvention.Winapi, CharSet = CharSet.Auto)]
        public static extern Int16 FilterSendMessage(
            IntPtr hPort,
            IntPtr lpInBuffer,
            UInt32 dwInBufferSize,
            [In, Out] IntPtr lpOutBuffer,
            UInt32 dwOutBufferSize,
            out UInt32 lpBytesReturned);

        [DllImport("ntdll.dll", SetLastError = true)]
        internal static extern NTSTATUS NtQueryInformationFile(IntPtr FileHandle,
            ref IO_STATUS_BLOCK IoStatusBlock, IntPtr FileInformation, uint FileInformationLength,
            FILE_INFORMATION_CLASS FileStreamInformation);

        [DllImport("kernel32")]
        public static extern Int32 GetLastError();

        [DllImport("kernel32.dll", EntryPoint = "LoadLibrary")]
        public extern static IntPtr LoadLibrary(string librayName);

        [DllImport("kernel32.dll", EntryPoint = "GetProcAddress", CharSet = CharSet.Ansi)]
        public extern static IntPtr GetProcAddress(IntPtr hwnd, string procedureName);

        [DllImport("kernel32.dll", EntryPoint = "FreeLibrary")]
        public extern static bool FreeLibrary(IntPtr hModule);

        [DllExport("IsSHA2Supported", CallingConvention = CallingConvention.StdCall)]
        public static bool IsSHA2Supported()
        {
            IntPtr hLib = NativeAPI.LoadLibrary("wintrust.dll");
            if (hLib != IntPtr.Zero)
            {
                IntPtr fpCryptCATAdminAcquireContext2 = NativeAPI.GetProcAddress(hLib, "CryptCATAdminAcquireContext2");

                NativeAPI.FreeLibrary(hLib);

                if (fpCryptCATAdminAcquireContext2 != IntPtr.Zero) return true;
            }

            return false;
        }


        #region Structures

        [StructLayout(LayoutKind.Sequential)]
        public struct IO_STATUS_BLOCK
        {
            public UInt32 Status;
            public UInt64 Information;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 4)]
        public struct BY_HANDLE_FILE_INFORMATION
        {
            public uint FileAttributes;
            public FILETIME CreationTime;
            public FILETIME LastAccessTime;
            public FILETIME LastWriteTime;
            public uint VolumeSerialNumber;
            public uint FileSizeHigh;
            public uint FileSizeLow;
            public uint NumberOfLinks;
            public uint FileIndexHigh;
            public uint FileIndexLow;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode, Pack = 1)]
        public struct FILE_STREAM_INFORMATION
        {
            public UInt32 NextEntryOffset;
            public UInt32 StreamNameLen;
            public UInt64 StreamSize;           //LARGE_INTEGER
            public UInt64 StreamAllocationSize; //LARGE_INTEGER
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
            internal Byte[] StreamName;
        }
        #endregion

        #region Enumerations
        [Flags]
        public enum NTSTATUS : uint
        {
            STATUS_SUCCESS = 0x00000000,
            STATUS_INFO_LENGTH_MISMATCH = 0xC0000004
        }

        [Flags]
        public enum FILE_INFORMATION_CLASS
        {
            FileStreamInformation = 22
        }


        #endregion

        internal const int GENERIC_READ = unchecked((int)0x80000000);
        internal const int GENERIC_WRITE = unchecked((int)0x40000000);
        internal const int INVALID_HANDLE_VALUE = -1;
        internal const int ERROR_FILE_EXISTS = unchecked((int)0x00000050);
        internal const string CheckStreamName = ":MyCSharp:$DATA";


        public static bool CreateStreamFile(string FileName)
        {
            bool bStreamFile = false;
            IntPtr FileHandle;
            string StreamName = FileName + ":MiniFlt";

            FileHandle = NativeAPI.CreateFile(StreamName, NativeAPI.GENERIC_READ | NativeAPI.GENERIC_WRITE,
                0, IntPtr.Zero, FileMode.CreateNew, 0, IntPtr.Zero);

            if (FileHandle.ToInt32() != NativeAPI.INVALID_HANDLE_VALUE)
            {
                NativeAPI.CloseHandle(FileHandle);
                bStreamFile = true;
            }
            else if (NativeAPI.ERROR_FILE_EXISTS == NativeAPI.GetLastError())
            {
                bStreamFile = true;
            }

            return bStreamFile;
        }

        public static bool CheckStreamFile(string FileName)
        {
            bool bStreamFile = false;
            IntPtr FileHandle;
            NativeAPI.IO_STATUS_BLOCK IoStatusBlock = new NativeAPI.IO_STATUS_BLOCK();
            uint BufSize = 0x10000;   //initial buffer size of 65536 bytes
            IntPtr pBuffer = Marshal.AllocHGlobal((int)BufSize);

            FileHandle = NativeAPI.CreateFile(FileName, NativeAPI.GENERIC_READ | NativeAPI.GENERIC_WRITE,
                FileShare.ReadWrite, IntPtr.Zero, FileMode.Open, 0, IntPtr.Zero);

            if (FileHandle.ToInt32() != NativeAPI.INVALID_HANDLE_VALUE)
            {
                NativeAPI.NTSTATUS Status = NativeAPI.NtQueryInformationFile(FileHandle, ref IoStatusBlock, pBuffer, BufSize,
                    NativeAPI.FILE_INFORMATION_CLASS.FileStreamInformation);

                NativeAPI.CloseHandle(FileHandle);

                if (Status == NativeAPI.NTSTATUS.STATUS_SUCCESS)
                {
                    int StructSize = Marshal.SizeOf(typeof(NativeAPI.FILE_STREAM_INFORMATION));
                    NativeAPI.FILE_STREAM_INFORMATION FileStreamInfo;
                    string StreamName;
                    IntPtr DataPtr = pBuffer;

                    do
                    {
                        FileStreamInfo = (NativeAPI.FILE_STREAM_INFORMATION)Marshal.PtrToStructure(DataPtr, typeof(NativeAPI.FILE_STREAM_INFORMATION));

                        if (FileStreamInfo.StreamNameLen == 0) break;

                        StreamName = Marshal.PtrToStringUni(DataPtr + StructSize - 2, (int)FileStreamInfo.StreamNameLen / 2);
                        if (NativeAPI.CheckStreamName.Equals(StreamName) == true)
                        {
                            bStreamFile = true;
                            break;
                        }

                        DataPtr += (int)FileStreamInfo.NextEntryOffset;
                    } while (FileStreamInfo.NextEntryOffset != 0);
                }
            }

            Marshal.FreeHGlobal(pBuffer);

            return bStreamFile;
        }

        public static bool CheckTargetFile(string FileName, string StreamName)
        {
            bool bCheckFile = false;
            IntPtr FileHandle;

            string streamName = FileName + ":" + StreamName;
            FileHandle = NativeAPI.CreateFile(streamName, NativeAPI.GENERIC_READ | NativeAPI.GENERIC_WRITE,
                FileShare.ReadWrite, IntPtr.Zero, FileMode.Open, 0, IntPtr.Zero);

            if (FileHandle.ToInt32() != NativeAPI.INVALID_HANDLE_VALUE) // skip file target
            {
                bCheckFile = true;

                NativeAPI.CloseHandle(FileHandle);
            }

            return bCheckFile;
        }

        public bool CheckUseFile(string fileName)
        {
            bool bUse = false;

            try
            {
                using (FileStream fs = new FileStream(fileName, FileMode.Open, FileAccess.Read, FileShare.Read))
                {
                    fs.Close();
                    bUse = true;
                }
            }
            catch (Exception e)
            {
                Console.WriteLine("ErrMsg[{0}]", e.Message.ToString());
            }

            return bUse;
        }


    }
}
