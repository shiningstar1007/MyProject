using IMyCSharpService;
using System.Collections.Generic;
using System.ServiceModel;

namespace MyCSharp.Service
{

    [ServiceContract()]
    public interface IMyCSharpService
    {
        [OperationContract()]
        string getUNCPathFromHostName(string UNCPath);

        [OperationContract()]
        string getUNCPath(string originalPath);

        [OperationContract()]
        string getIPInfo();

        [OperationContract()]
        List<string> getProcessInfo();

        [OperationContract()]
        List<string> getGroupList();

        [OperationContract()]
        List<string> getUserList();

        [OperationContract()]
        ERR_CODE aclPolicyAdd(ACL_POL polParam);

        [OperationContract()]
        ERR_CODE aclPolicyModify(ACL_POL polParam);

        [OperationContract()]
        ERR_CODE aclPolicyDelete(ACL_POL polParam);

        [OperationContract()]
        ERR_CODE aclPolicyList(ONOFF_MODE addCmd, ONOFF_MODE listOnly, string polName, out string polList);

        [OperationContract()]
        ERR_CODE aclObjectAdd(ACL_OBJ objParam);

        [OperationContract()]
        ERR_CODE aclObjectModify(ACL_OBJ objParam);

        [OperationContract()]
        ERR_CODE aclObjectDelete(ACL_OBJ objParam);

        [OperationContract()]
        ERR_CODE aclObjectList(ONOFF_MODE addCmd, string objPath, out string objList);

        [OperationContract()]
        ERR_CODE aclSubjectAdd(ACL_SUB SubParam);

        [OperationContract()]
        ERR_CODE aclSubjectDelete(ACL_SUB SubParam);

        [OperationContract()]
        ERR_CODE aclSubjectList(ONOFF_MODE addCmd, ONOFF_MODE listOnly, string subName, SUB_TYPE subType, out string subList);

        [OperationContract()]
        ERR_CODE superSubAdd(SUPER_SUB superParam);

        [OperationContract()]
        ERR_CODE superSubDelete(SUPER_SUB superParam);

        [OperationContract()]
        ERR_CODE superSubList(ONOFF_MODE addCmd, out string superSubList);

        [OperationContract()]
        ERR_CODE addACLObjFromPol(ACL_OBJ objParam, ACL_POL polParam);

        [OperationContract()]
        ERR_CODE delACLObjFromPol(ACL_OBJ objParam, ACL_POL polParam);

        [OperationContract()]
        ERR_CODE addACLSubFromPol(ACL_SUB subParam, ACL_POL polParam);

        [OperationContract()]
        ERR_CODE delACLSubFromPol(ACL_SUB subParam, ACL_POL polParam);
    }

    [ServiceBehavior(InstanceContextMode = InstanceContextMode.Single)]
    public class MyCSharpService : IMyCSharpService
    {
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
            return CommFunc.CheckProcessExt(procExt);
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

        public ERR_CODE aclPolicyList(ONOFF_MODE addCmd, ONOFF_MODE listOnly, string polName, out string polList)
        {
            AccessControl acl = new AccessControl();

            return acl.aclPolicyList(addCmd, listOnly, polName, out polList);
        }

        public ERR_CODE aclSubjectAdd(ACL_SUB subParam)
        {
            AccessControl acl = new AccessControl();

            return acl.aclSubjectAdd(subParam);
        }

        public ERR_CODE aclSubjectDelete(ACL_SUB subParam)
        {
            AccessControl acl = new AccessControl();

            return acl.aclSubjectDelete(subParam);
        }

        public ERR_CODE aclSubjectClear()
        {
            AccessControl acl = new AccessControl();

            return acl.aclSubjectClear();
        }

        public ERR_CODE aclSubjectList(ONOFF_MODE addCmd, ONOFF_MODE listOnly, string subName, SUB_TYPE subType, out string subList)
        {
            AccessControl acl = new AccessControl();

            return acl.aclSubjectList(addCmd, listOnly, subName, subType, out subList);
        }

        public ERR_CODE AddACLSubFromPol(ACL_SUB subParam, SUB_PERM subPerm)
        {
            AccessControl acl = new AccessControl();

            return acl.AddACLSubFromPol(subParam, subPerm);
        }

        public ERR_CODE DelACLSubFromPol(ACL_SUB subParam, SUB_PERM subPerm)
        {
            AccessControl acl = new AccessControl();

            return acl.DelACLSubFromPol(subParam, subPerm);
        }

        public ERR_CODE aclObjectAdd(ACL_OBJ objParam)
        {
            AccessControl acl = new AccessControl();

            return acl.aclObjectAdd(objParam);
        }

        public ERR_CODE aclObjectModify(ACL_OBJ objParam)
        {
            AccessControl acl = new AccessControl();

            return acl.aclObjectModify(objParam);
        }

        public ERR_CODE aclObjectDelete(ACL_OBJ objParam)
        {
            AccessControl acl = new AccessControl();

            return acl.aclObjectDelete(objParam);
        }

        public ERR_CODE aclObjectClear()
        {
            AccessControl acl = new AccessControl();

            return acl.aclObjectClear();
        }

        public ERR_CODE aclObjectList(ONOFF_MODE addCmd, string objPath, out string objList)
        {
            AccessControl acl = new AccessControl();

            return acl.aclObjectList(addCmd, objPath, out objList);
        }

        public ERR_CODE addACLObjFromPol(ACL_OBJ objParam, ACL_POL polParam)
        {
            AccessControl acl = new AccessControl();

            return acl.AddACLObjFromPol(objParam, polParam);
        }

        public ERR_CODE delACLObjFromPol(ACL_OBJ objParam, ACL_POL polParam)
        {
            AccessControl acl = new AccessControl();

            return acl.DelACLObjFromPol(objParam, polParam);
        }

        public ERR_CODE superSubAdd(SUPER_SUB superParam)
        {
            AccessControl acl = new AccessControl();

            return acl.superSubAdd(superParam);
        }

        public ERR_CODE superSubDelete(SUPER_SUB superParam)
        {
            AccessControl acl = new AccessControl();

            return acl.SuperSubDelete(superParam);
        }

        public ERR_CODE superSubList(ONOFF_MODE addCmd, out string superSubList)
        {
            AccessControl acl = new AccessControl();

            return acl.superSubList(addCmd, out superSubList);
        }

        public ERR_CODE superSubClear()
        {
            AccessControl acl = new AccessControl();

            return acl.superSubClear();
        }
    }
}
