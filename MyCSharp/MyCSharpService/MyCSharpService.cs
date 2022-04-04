using IMyCSharpService;
using System.Collections.Generic;
using System.ServiceModel;

namespace MyCSharp.Service
{

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

        public string getUNCPathFromHostName(string UNCPath)
        {
            string hostName = "";
            CommFunc func = new CommFunc();
            hostName = func.GetUNCPathFromHostName(UNCPath);

            return hostName;
        }

        public string getUNCPath(string originalPath)
        {
            string UNCName = "";
            CommFunc func = new CommFunc();
            UNCName = func.GetUNCPath(originalPath);

            return UNCName;
        }

        public string getIPInfo()
        {
            string IPString = "";
            CommFunc func = new CommFunc();
            IPString = func.GetIPInfo();

            return IPString;
        }

        public List<string> getProcessInfo()
        {
            List<string> processList;
            CommFunc func = new CommFunc();

            processList = func.GetProcessInfo();

            return processList;
        }

        public List<string> getGroupList()
        {
            List<string> groupList;
            CommFunc func = new CommFunc();

            groupList = func.GetGroupList();

            return groupList;
        }

        public List<string> getUserList()
        {
            List<string> userList;
            CommFunc func = new CommFunc();

            userList = func.userList();

            return userList;
        }

        public bool checkProcessExt(string procExt)
        {
            CommFunc func = new CommFunc();

            return func.CheckProcessExt(procExt);
        }

        public bool createDir(string dirPath)
        {
            CommFunc func = new CommFunc();

            return func.CreateDir(dirPath);
        }

        public ERR_CODE aclPolicyAdd(ACL_POL polParam)
        {
            AccessControl acl = new AccessControl();

            return acl.aclPolicyAdd(polParam);
        }

        public ERR_CODE aclPolicyModify(ACL_POL polParam)
        {
            AccessControl acl = new AccessControl();

            return acl.aclPolicyModify(polParam);
        }

        public ERR_CODE aclPolicyDelete(ACL_POL polParam)
        {
            AccessControl acl = new AccessControl();

            return acl.aclPolicyDelete(polParam);
        }

        public ERR_CODE aclPolicyClear()
        {
            AccessControl acl = new AccessControl();

            return acl.aclPolicyClear();
        }
    }
}
