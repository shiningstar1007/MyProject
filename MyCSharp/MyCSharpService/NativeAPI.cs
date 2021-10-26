using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.ConstrainedExecution;
using System.Runtime.InteropServices;
using System.Text;
using System.IO;

namespace MyCSharpService
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
    }
}
