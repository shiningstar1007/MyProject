using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.ConstrainedExecution;
using System.Runtime.InteropServices;
using System.Text;
using System.IO;

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

        [DllImport("fltlib", SetLastError = true)]
        public static extern int FilterConnectCommunicationPort(
            [MarshalAs (UnmanagedType.LPWStr)]
            string portName,
            uint options,
            IntPtr context,
            uint sizeOfContext,
            IntPtr securityAttributes,
            IntPtr hPort
        );

        [DllImport("fltlib", SetLastError = true)]
        public static extern int FilterGetMessage(
            IntPtr hPort,
            IntPtr msgBuffer,
            uint msgBufSize,
            IntPtr lpOverlapped
        );

        [DllImport("fltlib", SetLastError = true)]
        public static extern int FilterSendMessage(
            IntPtr hPort,
            IntPtr inBuffer,
            uint inBufferSize,
            IntPtr outBuffer,
            uint outBufferSize,
            uint bytesReturned
        );

        [DllImport("ntdll.dll", SetLastError = true)]
        internal static extern NTSTATUS NtQueryInformationFile(IntPtr FileHandle,
            ref IO_STATUS_BLOCK IoStatusBlock, IntPtr FileInformation, uint FileInformationLength,
            FILE_INFORMATION_CLASS FileStreamInformation);

        [DllImport("kernel32")]
        public static extern Int32 GetLastError();

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
