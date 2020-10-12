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
        public IList<ACL_Subject> aclSubject = new List<ACL_Subject>();

        private ACL_Subject SearchSubject(string subjectName)
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

    public class ACLObject
    {
        public IList<ACL_Object> aclObject = new List<ACL_Object>();

        private ACL_Object SearchObject(string objectName)
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
