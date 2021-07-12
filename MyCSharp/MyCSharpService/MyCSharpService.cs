using System;
using System.Collections.Generic;
using System.Text;
using System.ServiceModel;
using System.Runtime.InteropServices;
using System.Timers;
using System.IO;
using System.Security.AccessControl;
using System.Runtime.ConstrainedExecution;
using System.Security;
using System.Text.RegularExpressions;
using System.Linq;

namespace MyCSharp.Service
{
    [SuppressUnmanagedCodeSecurity()]
    public class Win32API
    {
        #region Native Method Signatures
        [DllImport("fltlib", SetLastError = true)]
        public static extern int FilterConnectCommunicationPort (
            [MarshalAs (UnmanagedType.LPWStr)]
            string portName,
            uint options,
            IntPtr context,
            uint sizeOfContext,
            IntPtr securityAttributes,
            IntPtr hPort
        );

        [DllImport("fltlib", SetLastError = true)]
        public static extern int FilterGetMessage (
            IntPtr hPort,
            IntPtr msgBuffer,
            uint msgBufSize,
            IntPtr lpOverlapped
        );

        [DllImport("fltlib", SetLastError = true)]
        public static extern int FilterSendMessage (
            IntPtr hPort,
            IntPtr inBuffer,
            uint inBufferSize,
            IntPtr outBuffer,
            uint outBufferSize,
            uint bytesReturned
        );

        internal const int GENERIC_READ = unchecked((int)0x80000000);
        internal const int GENERIC_WRITE = unchecked((int)0x40000000);
        internal const int INVALID_HANDLE_VALUE = -1;
        internal const int ERROR_FILE_EXISTS = unchecked((int)0x00000050);
        internal const string CheckStreamName = ":pske:$DATA";

        [DllImport("kernel32", SetLastError = true, CharSet = CharSet.Unicode)]
        internal extern static IntPtr CreateFile(String fileName,
           int dwDesiredAccess, System.IO.FileShare dwShareMode,
           IntPtr securityAttrs_MustBeZero, System.IO.FileMode dwCreationDisposition,
           int dwFlagsAndAttributes, IntPtr hTemplateFile_MustBeZero);

        [DllImport("kernel32", SetLastError = true)]
        [ReliabilityContract(Consistency.WillNotCorruptState, Cer.MayFail)]
        internal extern static bool CloseHandle(IntPtr FileHandle);

        [DllImport("ntdll.dll", SetLastError = true)]
        internal static extern NTSTATUS NtQueryInformationFile(IntPtr FileHandle,
            ref IO_STATUS_BLOCK IoStatusBlock, IntPtr FileInformation, uint FileInformationLength,
            FILE_INFORMATION_CLASS FileStreamInformation);

        [DllImport("kernel32")]
        public static extern Int32 GetLastError();

        #endregion

        #region Structures

        [StructLayout(LayoutKind.Sequential)]
        public struct IO_STATUS_BLOCK
        {
            public UInt32 Status;
            public UInt64 Information;
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

        #region Functions

        public static bool CreateStreamFile(string FileName)
        {
            bool bStreamFile = false;
            IntPtr FileHandle;
            string StreamName = FileName + ":MiniFlt";

            FileHandle = Win32API.CreateFile(StreamName, Win32API.GENERIC_READ | Win32API.GENERIC_WRITE,
                0, IntPtr.Zero, FileMode.CreateNew, 0, IntPtr.Zero);

            if (FileHandle.ToInt32() != Win32API.INVALID_HANDLE_VALUE)
            {
                Win32API.CloseHandle(FileHandle);
                bStreamFile = true;
            }
            else if (Win32API.ERROR_FILE_EXISTS == Win32API.GetLastError())
            {
                bStreamFile = true;
            }

            return bStreamFile;
        }

        public static bool CheckStreamFile(string FileName)
        {
            bool bStreamFile = false;
            IntPtr FileHandle;
            IO_STATUS_BLOCK IoStatusBlock = new IO_STATUS_BLOCK();
            uint BufSize = 0x10000;   //initial buffer size of 65536 bytes
            IntPtr pBuffer = Marshal.AllocHGlobal((int)BufSize);

            FileHandle = Win32API.CreateFile(FileName, Win32API.GENERIC_READ | Win32API.GENERIC_WRITE,
                FileShare.ReadWrite, IntPtr.Zero, FileMode.Open, 0, IntPtr.Zero);

            if (FileHandle.ToInt32() != Win32API.INVALID_HANDLE_VALUE)
            {
                NTSTATUS Status = Win32API.NtQueryInformationFile(FileHandle, ref IoStatusBlock, pBuffer, BufSize,
                    FILE_INFORMATION_CLASS.FileStreamInformation);

                Win32API.CloseHandle(FileHandle);

                if (Status == NTSTATUS.STATUS_SUCCESS)
                {
                    int StructSize = Marshal.SizeOf(typeof(FILE_STREAM_INFORMATION));
                    FILE_STREAM_INFORMATION FileStreamInfo;
                    string StreamName;
                    IntPtr DataPtr = pBuffer;

                    do
                    {
                        FileStreamInfo = (FILE_STREAM_INFORMATION)Marshal.PtrToStructure(DataPtr, typeof(FILE_STREAM_INFORMATION));

                        if (FileStreamInfo.StreamNameLen == 0) break;

                        StreamName = Marshal.PtrToStringUni(DataPtr + StructSize - 2, (int)FileStreamInfo.StreamNameLen / 2);
                        if (CheckStreamName.Equals(StreamName) == true)
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
    }

    public class ACL_Subject
    {
        public string subjectName;
        public UInt64 permissions;
    }

    public class ACL_Object
    {
        public string objectName;
        public UInt64 permissions;

        public IList<ACL_Subject> aclSubject = new List<ACL_Subject>();
    }

    public class ACLSubject
    {
        public IList<ACL_Subject> aclSubject = new List<ACL_Subject>();

        public ACL_Subject SearchSubject(string subjectName)
        {
            ACL_Subject aclSub = null;

            foreach(var sub in aclSubject)
            {
                if (sub.subjectName == subjectName)
                {
                    aclSub = sub;
                    break;
                }
            }

            return aclSub;
        }
        public void ACLSubjectAdd(string subjectName, UInt64 permissions)
        {
            ACL_Subject aclSub;

            aclSub = SearchSubject(subjectName);
            if (aclSub != null) return;
            else aclSub = new ACL_Subject();

            aclSub.subjectName = subjectName;
            aclSub.permissions = permissions;

            aclSubject.Add(aclSub);
        }

        public void ACLSubjectRemove(string subjectName)
        {
            Int32 index = -1;

            foreach (var aclSub in aclSubject)
            {
                if (aclSub.subjectName == subjectName)
                {
                    index = aclSubject.IndexOf(aclSub);
                    break;
                }
            }

            if (index != -1) aclSubject.RemoveAt(index);
        }

        public void ACLSubjectList()
        {
            foreach (var aclSub in aclSubject)
            {
                Console.WriteLine("SubjectName={0}, permissions={1}", aclSub.subjectName, aclSub.permissions);
            }
        }
    }

    public class ACLObject : ACLSubject
    {
        public IList<ACL_Object> aclObject = new List<ACL_Object>();

        public ACL_Object SearchObject(string objectName)
        {
            ACL_Object aclObj = null;

            foreach(var obj in aclObject)
            {
                if (obj.objectName == objectName)
                {
                    aclObj = obj;
                    break;
                }
            }

            return aclObj;
        }

        public void ACLObjectAdd(string objectName, UInt64 permissions)
        {
            ACL_Object aclObj;

            aclObj = SearchObject(objectName);
            if (aclObj != null) return;
            else aclObj = new ACL_Object();

            aclObj.objectName = objectName;
            aclObj.permissions = permissions;

            aclObject.Add(aclObj);
        }

        public void ACLObjectRemove(string objectName)
        {
            Int32 index = -1;

            foreach (var aclObj in aclObject)
            {
                if (aclObj.objectName == objectName)
                {
                    index = aclObject.IndexOf(aclObj);
                    break;
                }
            }

            if (index != -1) aclObject.RemoveAt(index);
        }

        public void ACLObjectList()
        {
            foreach (var aclObj in aclObject)
            {
                Console.WriteLine("ObjectName={0}, permissions={1}", aclObj.objectName, aclObj.permissions);
            }
        }
    }

    public class ACLEntries : ACLObject
    {
        public void ACLEntriesAdd(string objectName, string subjectName)
        {
            ACL_Object aclObject = null;
            ACL_Subject aclSubject = null;

            aclObject = SearchObject(objectName);
            if (aclObject == null) return;

            aclSubject = SearchSubject(subjectName);
            if (aclSubject == null) return;

            aclObject.aclSubject.Add(aclSubject);
        }

        public void ACLEntriesRemove(string objectName, string subjectName)
        {
            ACL_Object aclObject = null;
            ACL_Subject aclSubject = null;

            aclObject = SearchObject(objectName);
            if (aclObject == null) return;

            aclSubject = SearchSubject(subjectName);
            if (aclSubject == null) return;

            aclObject.aclSubject.Remove(aclSubject);
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
