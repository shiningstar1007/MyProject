using System;
using System.Collections.Generic;
using System.Text;
using System.ServiceModel;
using System.Runtime.InteropServices;
using System.Timers;
using System.IO;
using System.Security.AccessControl;

namespace MyCSharp.Service
{
    public class Win32API
    {
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

    public class ACL_Subject
    {
        public string subjectName;
        public UInt64 permissions;
    }

    public class ACL_Object
    {
        public string objectName;
        public UInt64 permissions;
    }

    public class ACLSubject
    {
        private IList<ACL_Subject> _aclSubject = new List<ACL_Subject>();
        public IList<ACL_Subject> aclSubject = new List<ACL_Subject>();

        public void ACLSubjectAdd(string subjectName, UInt64 permissions)
        {
            ACL_Subject aclSub = new ACL_Subject();

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

}
