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



    }
}
