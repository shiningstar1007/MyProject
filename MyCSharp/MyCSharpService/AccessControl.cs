using MyCSharp.Service;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Security.Principal;
using System.Text;
using System.Threading.Tasks;

namespace MyCSharpService
{
    public enum KERNEL_COMMAND : int
    {
        ACL_POLICY_ADD,
        ACL_POLICY_MODIFY,
        ACL_POLICY_DELETE,
        ACL_POLICY_CLEAR,
        ACL_POLICY_SUB_CLEAR,
        ACL_POLICY_OBJ_CLEAR,
        ACL_POLICY_LIST,

        ACL_SUBJECT_ADD,
        ACL_SUBJECT_MODIFY,
        ACL_SUBJECT_DELETE,
        ACL_SUBJECT_CLEAR,
        ACL_SUBJECT_POL_ADD,
        ACL_SUBJECT_POL_DEL,
        ACL_SUBJECT_LIST,

        ACL_OBJECT_ADD,
        ACL_OBJECT_MODIFY,
        ACL_OBJECT_DELETE,
        ACL_OBJECT_CLEAR,
        ACL_OBJECT_POL_ADD,
        ACL_OBJECT_POL_DEL,
        ACL_OBJECT_LIST,

        SUPER_PROCESS_ADD,
        SUPER_PROCESS_DELETE,
        SUPER_PROCESS_CLEAR,
        SUPER_PROCESS_LIST,

        ACL_SET,
        ACL_MODE,
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
    public struct COMMAND_MESSAGE
    {
        public KERNEL_COMMAND command;
        public UInt32 reserved;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4096)]
        public Byte[] data;
    }


    public enum EFFECT_MODE
    {
        EFT_DENY = 0,
        EFT_ALLOW,
        EFT_UNKNOWN
    }

    public static class EFFECT_MODE_STR
    {
        public const String EFT_ALLOW_STR = "allow";
        public const String EFT_DENY_STR = "deny";
        public const String EFT_UNKNOWN_STR = "unknown";
    }

    public enum RUN_MODE : ulong
    {
        RUN_NORMAL = 0,
        RUN_TEST,
        RUN_DISABLE
    }

    public static class RUN_MODE_STR
    {
        public const String RUN_NORMAL_STR = "normal";
        public const String RUN_TEST_STR = "test";
        public const String RUN_DISABLE_STR = "disable";
    }

    public enum ACL_ACTION : ulong
    {
        ACT_READ = 0x00000001,
        ACT_WRITE = 0x00000002,
        ACT_TRAVERSE = 0x00000004,
        ACT_EXECUTE = 0x00000008,
        ACT_DELETE = 0x00000010,
        ACT_CREATE = 0x00000020,
        ACT_DEC = 0x00000040,
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
        public const String ACT_DEC_STR = "dec";
        public const String ACT_ALL_STR = "all";
    }

    public enum ONOFF_MODE : int
    {
        OFM_ON = 0,
        OFM_OFF,
    }

    public static class ONOFF_MODE_STR
    {
        public const String OFM_ON_STR = "on";
        public const String OFM_OFF_STR = "off";
    }

    public enum LOGGING_TYPE : int
    {
        LOG_DENY = 0,
        LOG_ALLOW,
        LOG_ALL
    }

    public static class LOGGING_TYPE_STR
    {
        public const String LOG_ALLOW_STR = "allow";
        public const String LOG_DENY_STR = "deny";
        public const String LOG_ALL_STR = "all";
    }

    public enum SHARED_PERM : int
    {
        SDP_DECRYPT = 0,
        SDP_ENCRYPT,
        SDP_DENY,
        SDP_UNKNOWN
    }

    public static class SHARED_PERM_STR
    {
        public const String SDP_DECRYPT_STR = "decrypt";
        public const String SDP_ENCRYPT_STR = "encrypt";
        public const String SDP_DENY_STR = "deny";
        public const String SDP_UNKNOWN_STR = "unknown";
    }

    public class ACL_DATA
    {
        public ACL_POL ACLPol;
        public ACL_SUB ACLSub;
        public ACL_OBJ ACLObj;
        public SUB_PERM SubPerm;
    }

    public class ACL_LINK
    {
        public IList<ACL_DATA> ACLData = new List<ACL_DATA>();
    }

    public class ACL_POL
    {
        public String PolName;
        public ONOFF_MODE LogMode;
        public ONOFF_MODE RunMode;

        public ACL_LINK ACLSubs = new ACL_LINK();
        public ACL_LINK ACLObjs = new ACL_LINK();

        public ACL_POL()
        {
            this.PolName = "";
            this.LogMode = ONOFF_MODE.OFM_OFF;
            this.RunMode = ONOFF_MODE.OFM_OFF;
        }

        public ACL_POL(ACL_POL polParam)
        {
            this.PolName = polParam.PolName;
            this.LogMode = polParam.LogMode;
            this.RunMode = polParam.RunMode;
        }

        public void Copy(ACL_POL polParam)
        {
            this.LogMode = polParam.LogMode;
            this.RunMode = polParam.RunMode;
        }
    }

    public enum SUB_TYPE
    {
        SUB_USER = 0,
        SUB_PROC,
        SUB_SHARE,
        SUB_GROUP,
        SUB_UNKNOWN
    }

    public static class SUB_TYPE_STR
    {
        public const String SUB_USER_STR = "user";
        public const String SUB_GROUP_STR = "group";
        public const String SUB_PROC_STR = "proc";
        public const String SUB_SHARE_STR = "share";
    }

    public class ACL_SUB
    {
        public UInt64 SubKey;
        public SUB_TYPE SubType;
        public String SubName;

        public SecurityIdentifier UserSId;

        public ACL_LINK AllowPols = new ACL_LINK();
        public ACL_LINK DenyPols = new ACL_LINK();

        public ACL_SUB()
        {
            this.SubKey = 0;
            this.SubType = SUB_TYPE.SUB_UNKNOWN;
            this.SubName = "";
            this.UserSId = null;
        }

        public ACL_SUB(ACL_SUB subParam)
        {
            this.SubKey = subParam.SubKey;
            this.SubType = subParam.SubType;
            this.SubName = subParam.SubName;
            this.UserSId = subParam.UserSId;
        }

        public void Copy(ACL_SUB subParam)
        {
        }
    }

    public class PROC_USER
    {
        public SUB_TYPE SubType;
        public String SubName;
        public SecurityIdentifier UserSId;

        public PROC_USER()
        {
            this.SubType = SUB_TYPE.SUB_UNKNOWN;
            this.SubName = "";
            this.UserSId = null;
        }

        public PROC_USER(PROC_USER procParam)
        {
            this.SubType = procParam.SubType;
            this.SubName = procParam.SubName;
            this.UserSId = procParam.UserSId;
        }
    }
    public class SUB_PERM
    {
        public ACL_POL ACLPol;
        public EFFECT_MODE Effect;
        public UInt32 Action;

        public EFFECT_MODE DecPerm;
        public PROC_USER ProcUser;

        public SUB_PERM()
        {
            this.ACLPol = null;
            this.Effect = EFFECT_MODE.EFT_UNKNOWN;
            this.Action = 0;
            this.DecPerm = EFFECT_MODE.EFT_UNKNOWN;
        }

        public SUB_PERM(SUB_PERM permParam)
        {
            this.ACLPol = new ACL_POL(permParam.ACLPol);
            this.Effect = permParam.Effect;
            this.Action = permParam.Action;
            this.DecPerm = permParam.DecPerm;
        }
    }

    public enum OBJ_TYPE : int
    {
        OBJ_FILE = 0,
        OBJ_DIR,
        OBJ_UNKNOWN
    }

    public static class OBJ_TYPE_STR
    {
        public const String OBJ_FILE_STR = "file";
        public const String OBJ_DIR_STR = "dir";
    }

    public class ACL_OBJ
    {
        public UInt64 ObjKey;
        public OBJ_TYPE ObjType;
        public String ObjPath;
        public ONOFF_MODE SubDir;
        public UInt32 DefAction;
        public String CrossPath;

        public ONOFF_MODE RunMode;
        public ONOFF_MODE LogMode;
        public LOGGING_TYPE LoggingType;
        public SHARED_PERM SharedPerm;

        public ACL_LINK ACLPols = new ACL_LINK();

        public ACL_OBJ()
        {
            ObjKey = 0;
            ObjType = OBJ_TYPE.OBJ_UNKNOWN;
            ObjPath = @"";
            SubDir = ONOFF_MODE.OFM_OFF;
            DefAction = 0;
            CrossPath = @"";
            RunMode = ONOFF_MODE.OFM_OFF;
            LogMode = ONOFF_MODE.OFM_OFF;
            LoggingType = LOGGING_TYPE.LOG_DENY;
            SharedPerm = SHARED_PERM.SDP_UNKNOWN;
        }

        public ACL_OBJ(ACL_OBJ objParam)
        {
            ObjKey = objParam.ObjKey;
            ObjType = objParam.ObjType;
            ObjPath = objParam.ObjPath;
            SubDir = objParam.SubDir;
            DefAction = objParam.DefAction;
            CrossPath = objParam.CrossPath;
            RunMode = objParam.RunMode;
            LogMode = objParam.LogMode;
            LoggingType = objParam.LoggingType;
            SharedPerm = objParam.SharedPerm;
        }

        public void Copy(ACL_OBJ objParam)
        {
            SubDir = objParam.SubDir;
            DefAction = objParam.DefAction;
            CrossPath = objParam.CrossPath;
            RunMode = objParam.RunMode;
            LogMode = objParam.LogMode;
            LoggingType = objParam.LoggingType;
            SharedPerm = objParam.SharedPerm;
        }
    }

    public class SUPER_SUB
    {
        public UInt64 SubKey;
        public SUB_TYPE SubType;
        public String SubName;

        public SecurityIdentifier UserSId;
        public String UserSIdBuf;

        public EFFECT_MODE DecPerm;

        public SUPER_SUB()
        {
            this.SubKey = 0;
            this.SubType = SUB_TYPE.SUB_PROC;
            this.SubName = "";

            this.UserSId = null;
            this.UserSIdBuf = "";

            this.DecPerm = EFFECT_MODE.EFT_ALLOW;
        }

        public SUPER_SUB(SUPER_SUB superParam)
        {
            this.SubKey = superParam.SubKey;
            this.SubType = superParam.SubType;
            this.SubName = superParam.SubName;

            this.UserSId = superParam.UserSId;
            this.UserSIdBuf = superParam.UserSIdBuf;

            this.DecPerm = superParam.DecPerm;
        }
    }
    class AccessControl
    {
        IList<ACL_POL> g_ACLPolicy = new List<ACL_POL>();
        IList<ACL_SUB> g_ACLSubject = new List<ACL_SUB>();
        IList<ACL_OBJ> g_ACLObject = new List<ACL_OBJ>();
        IList<SUPER_SUB> g_SuperSub = new List<SUPER_SUB>();

        public ERR_CODE sendACLPolInfo(ACL_POL polParam, KERNEL_COMMAND cmdParam)
        {
            ERR_CODE ErrCode = ERR_CODE.ERR_SUCCESS;
            String dataBuf;
            Byte[] byteCode = new Byte[sizeof(ERR_CODE)];

            dataBuf = String.Format("polname={0} runmode={1} logmode={2}", polParam.PolName,
            CommFunc.OnOffModeToStr(polParam.RunMode), CommFunc.OnOffModeToStr(polParam.LogMode));

            sendMessageDriver(cmdParam, dataBuf, ref byteCode, (UInt32)byteCode.Length);
            ErrCode = (ERR_CODE)BitConverter.ToInt32(byteCode, 0);

            return ErrCode;
        }

        public ACL_POL aclPolicyFind(String polName)
        {
            foreach (var aclPol in g_ACLPolicy)
            {
                if (String.Equals(aclPol.PolName, polName) == false) continue;

                return aclPol;
            }

            return null;
        }

        public ERR_CODE aclPolicyAdd(ACL_POL polParam)
        {
            ERR_CODE errCode = ERR_CODE.ERR_SUCCESS;
            ACL_POL aclPol;

            aclPol = aclPolicyFind(polParam.PolName);
            if (aclPol != null) return ERR_CODE.ERR_ACLPOL_EXIST;

            errCode = sendACLPolInfo(polParam, KERNEL_COMMAND.ACL_POLICY_ADD);
            if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;

            aclPol = new ACL_POL(polParam);
            g_ACLPolicy.Add(aclPol);

            return errCode;
        }

        public ERR_CODE aclPolicyModify(ACL_POL polParam)
        {
            ERR_CODE errCode = ERR_CODE.ERR_SUCCESS;
            ACL_POL aclPol;

            aclPol = aclPolicyFind(polParam.PolName);
            if (aclPol == null) return ERR_CODE.ERR_ACLPOL_NOT_EXIST;

            errCode = sendACLPolInfo(polParam, KERNEL_COMMAND.ACL_POLICY_MODIFY);
            if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;

            aclPol.Copy(polParam);

            return ERR_CODE.ERR_SUCCESS;
        }

        public ERR_CODE aclPolicyDelete(ACL_POL polParam)
        {
            ERR_CODE errCode = ERR_CODE.ERR_SUCCESS;
            ACL_POL aclPol;

            aclPol = aclPolicyFind(polParam.PolName);
            if (aclPol == null) return ERR_CODE.ERR_SUCCESS;

            errCode = sendACLPolInfo(polParam, KERNEL_COMMAND.ACL_POLICY_DELETE);
            if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;

            g_ACLPolicy.Remove(aclPol);

            return ERR_CODE.ERR_SUCCESS;
        }

        public ERR_CODE aclPolicyClear()
        {
            ERR_CODE errCode = ERR_CODE.ERR_SUCCESS;
            Byte[] byteCode = new Byte[sizeof(ERR_CODE)];

            sendMessageDriver(KERNEL_COMMAND.ACL_POLICY_CLEAR, null, ref byteCode, (UInt32)byteCode.Length);
            errCode = (ERR_CODE)BitConverter.ToInt32(byteCode, 0);
            if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;

            g_ACLPolicy.Clear();

            return ERR_CODE.ERR_SUCCESS;
        }

        public ERR_CODE sendACLSubInfo(ACL_SUB subParam, SUB_PERM subPerm, KERNEL_COMMAND cmdParam)
        {
            ERR_CODE ErrCode = ERR_CODE.ERR_SUCCESS;
            String dataBuf;
            Byte[] byteCode = new Byte[sizeof(ERR_CODE)];

            dataBuf = String.Format("subkey={0} subtype={1} subname=\"{2}\"",
                subParam.SubKey, CommFunc.SubTypeToStr(subParam.SubType), subParam.SubName);

            if (subPerm != null)
            {
                dataBuf += String.Format(" polname=\"{0}\" action={1} effect={2} decrypt={3}",
                    subPerm.ACLPol.PolName, subPerm.Action, CommFunc.EffectModeToStr(subPerm.Effect), CommFunc.EffectModeToStr(subPerm.DecPerm));

                if (subParam.SubType == SUB_TYPE.SUB_PROC && subPerm.ProcUser.SubType != SUB_TYPE.SUB_UNKNOWN)
                {
                    dataBuf += String.Format(" subsubtype={0} subsubname=\"{1}\"",
                        CommFunc.SubTypeToStr(subPerm.ProcUser.SubType), subPerm.ProcUser.SubName);
                }
            }

            sendMessageDriver(cmdParam, dataBuf, ref byteCode, (UInt32)byteCode.Length);
            ErrCode = (ERR_CODE)BitConverter.ToInt32(byteCode, 0);

            return ErrCode;
        }

        public ERR_CODE aclPolicyList(ONOFF_MODE addCmd, ONOFF_MODE listOnly, String polName, out String polList)
        {
            String copyBuf = "";
            Int32 offSet = 0;

            foreach (var pol in g_ACLPolicy)
            {
                if (String.IsNullOrEmpty(polName) == false && String.Equals(polName, pol.PolName)) continue;

                if (offSet > 0) copyBuf += "\n";

                if (addCmd == ONOFF_MODE.OFM_ON) copyBuf += String.Format("aclpoladd ");

                copyBuf += String.Format("polname={0} runmode={1} logmode={2}",
                     pol.PolName, CommFunc.OnOffModeToStr(pol.RunMode), CommFunc.OnOffModeToStr(pol.LogMode));

                offSet = copyBuf.Length;
            }

            polList = String.Copy(copyBuf);
            polList += "\0";

            return ERR_CODE.ERR_SUCCESS;
        }

        public ERR_CODE setACLSubInfo(ACL_SUB subParam)
        {
            if (String.IsNullOrEmpty(subParam.SubName) == true) return ERR_CODE.ERR_ACLSUB_INVALID_NAME;
            else if (subParam.SubType == SUB_TYPE.SUB_UNKNOWN) return ERR_CODE.ERR_ACLSUB_INVALID_TYPE;

            if (subParam.SubType == SUB_TYPE.SUB_USER || subParam.SubType == SUB_TYPE.SUB_GROUP)
            {
                subParam.UserSId = CommFunc.GetUserSId(subParam.SubName);
                if (subParam.UserSId != null)
                {
                    subParam.SubKey = CommFunc.GetSIdKey(subParam.UserSId);
                }
                else return ERR_CODE.ERR_ACLSUB_GET_SID_FAIL;
            }
            else if (subParam.SubType == SUB_TYPE.SUB_PROC)
            {
                int lastIndex = subParam.SubName.LastIndexOf('.');
                if (lastIndex == -1) return ERR_CODE.ERR_ACLSUB_NO_PROCESS;
                else if (CommFunc.CheckProcessExt(subParam.SubName.Substring(lastIndex)) == false) return ERR_CODE.ERR_ACLSUB_NO_PROCESS;

                subParam.SubKey = CommFunc.GetObjKey(OBJ_TYPE.OBJ_DIR, subParam.SubName);
            }
            else if (subParam.SubType == SUB_TYPE.SUB_SHARE)
            {
                subParam.SubKey = (UInt64)SUB_TYPE.SUB_SHARE;
                subParam.SubName = SUB_TYPE_STR.SUB_SHARE_STR;
            }

            if (subParam.SubKey == 0) return ERR_CODE.ERR_ACLSUB_GET_SID_FAIL;

            return ERR_CODE.ERR_SUCCESS;
        }

        public ACL_SUB aclSubjectFind(SUB_TYPE subType, UInt64 subKey, String subName)
        {
            foreach (var aclSub in g_ACLSubject)
            {
                if (aclSub.SubType != subType) continue;

                if (aclSub.SubKey != subKey) continue;

                if (String.Equals(aclSub.SubName, subName) == false) continue;

                return aclSub;
            }

            return null;
        }

        public ERR_CODE aclSubjectAdd(ACL_SUB subParam)
        {
            ERR_CODE errCode = ERR_CODE.ERR_SUCCESS;
            ACL_SUB aclSub;

            errCode = setACLSubInfo(subParam);
            if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;

            aclSub = aclSubjectFind(subParam.SubType, subParam.SubKey, subParam.SubName);
            if (aclSub != null) return ERR_CODE.ERR_ACLSUB_EXIST;

            errCode = sendACLSubInfo(subParam, null, KERNEL_COMMAND.ACL_SUBJECT_ADD);
            if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;

            aclSub = new ACL_SUB(subParam);
            g_ACLSubject.Add(aclSub);

            return errCode;
        }

        public ERR_CODE aclSubjectModify(ACL_SUB subParam)
        {
            ERR_CODE errCode = ERR_CODE.ERR_SUCCESS;
            ACL_SUB aclSub;

            errCode = setACLSubInfo(subParam);
            if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;

            aclSub = aclSubjectFind(subParam.SubType, subParam.SubKey, subParam.SubName);
            if (aclSub == null) return ERR_CODE.ERR_ACLSUB_NOT_EXIST;

            errCode = sendACLSubInfo(subParam, null, KERNEL_COMMAND.ACL_SUBJECT_MODIFY);
            if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;

            aclSub.Copy(subParam);

            return ERR_CODE.ERR_SUCCESS;
        }

        public ERR_CODE aclSubjectDelete(ACL_SUB subParam)
        {
            ERR_CODE errCode = ERR_CODE.ERR_SUCCESS;
            ACL_SUB aclSub;

            errCode = setACLSubInfo(subParam);
            if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;

            aclSub = aclSubjectFind(subParam.SubType, subParam.SubKey, subParam.SubName);
            if (aclSub == null) return ERR_CODE.ERR_SUCCESS;

            errCode = sendACLSubInfo(subParam, null, KERNEL_COMMAND.ACL_SUBJECT_DELETE);
            if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;

            g_ACLSubject.Remove(aclSub);

            return ERR_CODE.ERR_SUCCESS;
        }

        public ERR_CODE aclSubjectClear()
        {
            ERR_CODE errCode = ERR_CODE.ERR_SUCCESS;
            Byte[] byteCode = new Byte[sizeof(ERR_CODE)];

            sendMessageDriver(KERNEL_COMMAND.ACL_SUBJECT_CLEAR, null, ref byteCode, (UInt32)byteCode.Length);
            errCode = (ERR_CODE)BitConverter.ToInt32(byteCode, 0);
            if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;

            g_ACLSubject.Clear(); ;

            return ERR_CODE.ERR_SUCCESS;
        }

        public ERR_CODE aclSubjectList(ONOFF_MODE addCmd, ONOFF_MODE listOnly, String subName, SUB_TYPE subType, out String subList)
        {
            String copyBuf = "", ruleID;
            Int32 offSet = 0, repeat;
            IList<ACL_DATA> targetData;

            foreach (var sub in g_ACLSubject)
            {
                if (String.IsNullOrEmpty(subName) == false && String.Equals(subName, sub.SubName)) continue;

                if (subType != SUB_TYPE.SUB_UNKNOWN && subType != sub.SubType) continue;

                if (offSet > 0) copyBuf += "\n";

                if (addCmd == ONOFF_MODE.OFM_ON)
                {
                    copyBuf += String.Format("aclsubadd ");
                }

                ruleID = String.Format("subtype={0} subname=\"{1}\"", CommFunc.SubTypeToStr(sub.SubType), sub.SubName);
                copyBuf += ruleID;

                if (addCmd == ONOFF_MODE.OFM_ON)
                {
                    copyBuf += String.Format(" subkey={0}", sub.SubKey);
                }

                offSet = copyBuf.Length;

                if (listOnly == ONOFF_MODE.OFM_ON) continue;

                for (repeat = 0; repeat < 2; repeat++)
                {
                    if (repeat == 0) targetData = sub.AllowPols.ACLData;
                    else if (repeat == 1) targetData = sub.DenyPols.ACLData;
                    else break;

                    foreach (var polData in targetData)
                    {
                        if (offSet > 0) copyBuf += "\n";

                        if (addCmd == ONOFF_MODE.OFM_ON) copyBuf += String.Format("aclsubpoladd ");

                        copyBuf += String.Format("{0} polname=\"{1}\" action={2} effect={3} decrypt={4}",
                            ruleID, polData.ACLPol.PolName, CommFunc.ActionToStr(polData.SubPerm.Action),
                            CommFunc.EffectModeToStr(polData.SubPerm.Effect), CommFunc.EffectModeToStr(polData.SubPerm.DecPerm));

                        if (polData.SubPerm.ProcUser.SubType != SUB_TYPE.SUB_UNKNOWN)
                        {
                            copyBuf += String.Format(" subsubtype={0} subsubname=\"{1}\"",
                            CommFunc.SubTypeToStr(polData.SubPerm.ProcUser.SubType), polData.SubPerm.ProcUser.SubName);

                        }

                        if (addCmd == ONOFF_MODE.OFM_ON)
                            copyBuf += String.Format(" subkey={0}", sub.SubKey);

                        offSet = copyBuf.Length;
                    }
                }
            }

            subList = String.Copy(copyBuf);
            subList += "\0";

            return ERR_CODE.ERR_SUCCESS;
        }

        public ERR_CODE AddACLSubFromPol(ACL_SUB subParam, SUB_PERM subPerm, Boolean autoLoad = true)
        {
            ERR_CODE errCode = ERR_CODE.ERR_SUCCESS;

            if (subParam.SubType == SUB_TYPE.SUB_PROC && subPerm.ProcUser.SubType != SUB_TYPE.SUB_UNKNOWN)
            {
                if (subPerm.ProcUser.SubType != SUB_TYPE.SUB_USER && subPerm.ProcUser.SubType != SUB_TYPE.SUB_GROUP)
                    return ERR_CODE.ERR_ACLSUB_INVALID_TYPE;
                else
                {
                    subPerm.ProcUser.UserSId = CommFunc.GetUserSId(subPerm.ProcUser.SubName);
                }
            }

            if (autoLoad == true)
            {
                errCode = sendACLSubInfo(subParam, subPerm, KERNEL_COMMAND.ACL_SUBJECT_POL_ADD);
                if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;
            }

            ACL_DATA newData = new ACL_DATA();
            newData.SubPerm = subPerm;
            newData.ACLSub = subParam;
            newData.ACLPol = subPerm.ACLPol;

            if (subPerm.Effect == EFFECT_MODE.EFT_ALLOW) subParam.AllowPols.ACLData.Add(newData);
            else subParam.DenyPols.ACLData.Add(newData);

            subPerm.ACLPol.ACLSubs.ACLData.Add(newData);

            return errCode;
        }

        public ERR_CODE DelACLSubFromPol(ACL_SUB subParam, SUB_PERM subPerm)
        {
            ERR_CODE errCode = ERR_CODE.ERR_SUCCESS;

            if (subParam.SubType == SUB_TYPE.SUB_PROC)
            {
                if (subPerm.ProcUser.SubType != SUB_TYPE.SUB_USER && subPerm.ProcUser.SubType != SUB_TYPE.SUB_GROUP)
                    return ERR_CODE.ERR_ACLSUB_INVALID_TYPE;
                else
                {
                    subPerm.ProcUser.UserSId = CommFunc.GetUserSId(subPerm.ProcUser.SubName);
                }
            }

            errCode = sendACLSubInfo(subParam, subPerm, KERNEL_COMMAND.ACL_SUBJECT_POL_DEL);
            if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;

            IList<ACL_DATA> aclDataList;

            if (subPerm.Effect == EFFECT_MODE.EFT_ALLOW) aclDataList = subParam.AllowPols.ACLData;
            else aclDataList = subParam.DenyPols.ACLData;

            foreach (ACL_DATA polData in aclDataList)
            {
                if (String.Equals(polData.ACLSub.SubName, subParam.SubName) == true)
                {
                    if (String.Equals(polData.ACLPol.PolName, subPerm.ACLPol.PolName) == true)
                    {
                        if (String.Equals(polData.SubPerm.ProcUser.SubName, subPerm.ProcUser.SubName) == true)
                        {
                            if (subPerm.Effect == EFFECT_MODE.EFT_ALLOW) subParam.AllowPols.ACLData.Remove(polData);
                            else subParam.DenyPols.ACLData.Remove(polData);

                            subPerm.ACLPol.ACLSubs.ACLData.Remove(polData);

                            break;
                        }
                    }
                }
            }

            return errCode;
        }

        public void ClearACLSubFromPol(ACL_SUB subParam)
        {
            IList<ACL_DATA> delDataList = new List<ACL_DATA>();
            IList<ACL_DATA> aclDataList;

            for (int repeat = 0; repeat < 2; repeat++)
            {
                if (repeat == 0) aclDataList = subParam.AllowPols.ACLData;
                else if (repeat == 1) aclDataList = subParam.DenyPols.ACLData;
                else break;

                foreach (ACL_DATA polData in aclDataList)
                {
                    foreach (ACL_DATA subData in polData.ACLPol.ACLSubs.ACLData)
                    {
                        if (String.Equals(subData.ACLSub.SubName, subParam.SubName) == true)
                        {
                            delDataList.Add(subData);
                        }
                    }

                    if (delDataList.Count > 0)
                    {
                        foreach (var delData in delDataList)
                        {
                            polData.ACLPol.ACLSubs.ACLData.Remove(delData);
                        }
                        delDataList.Clear();
                    }
                }
            }

            subParam.AllowPols.ACLData.Clear();
            subParam.DenyPols.ACLData.Clear();
        }

        public ERR_CODE sendACLObjInfo(ACL_OBJ objParam, ACL_POL polParam, KERNEL_COMMAND cmdParam)
        {
            ERR_CODE ErrCode = ERR_CODE.ERR_SUCCESS;
            String dataBuf;
            Byte[] byteCode = new Byte[sizeof(ERR_CODE)];

            dataBuf = String.Format("objkey={0} objtype={1} objpath=\"{2}\" action={3} subdir={4} runmode={5} logmode={6} logging={7} sharedperm={8} crosspath=\"{9}\"",
            objParam.ObjKey, CommFunc.ObjTypeToStr(objParam.ObjType), objParam.ObjPath, CommFunc.ActionToStr(objParam.DefAction),
            CommFunc.OnOffModeToStr(objParam.SubDir), CommFunc.OnOffModeToStr(objParam.RunMode), CommFunc.OnOffModeToStr(objParam.LogMode),
            CommFunc.LoggingTypeToStr(objParam.LoggingType), CommFunc.SharedPermToStr(objParam.SharedPerm), objParam.CrossPath);

            if (polParam != null)
            {
                dataBuf += String.Format(" polname=\"{0}\"", polParam.PolName);
            }

            sendMessageDriver(cmdParam, dataBuf, ref byteCode, (UInt32)byteCode.Length);
            ErrCode = (ERR_CODE)BitConverter.ToInt32(byteCode, 0);

            return ErrCode;
        }

        public ERR_CODE setACLObjInfo(ACL_OBJ objParam)
        {
            if (String.IsNullOrEmpty(objParam.ObjPath) == true) return ERR_CODE.ERR_ACLOBJ_INVALID_PATH;
            else if (objParam.ObjType == OBJ_TYPE.OBJ_UNKNOWN) return ERR_CODE.ERR_ACLOBJ_INVALID_TYPE;

            objParam.ObjKey = CommFunc.GetObjKey(objParam.ObjType, objParam.ObjPath);
            if (objParam.ObjKey == 0) return ERR_CODE.ERR_ACLOBJ_GET_KEY_FAIL;

            if (objParam.ObjType == OBJ_TYPE.OBJ_DIR) objParam.SubDir = ONOFF_MODE.OFM_OFF;

            return ERR_CODE.ERR_SUCCESS;
        }

        public ACL_OBJ aclObjectFind(OBJ_TYPE objType, UInt64 objKey, String objPath)
        {
            foreach (var aclObj in g_ACLObject)
            {
                if (aclObj.ObjType != objType) continue;

                if (aclObj.ObjKey != objKey) continue;

                if (String.Equals(aclObj.ObjPath, objPath) == false) continue;

                return aclObj;
            }

            return null;
        }

        public ERR_CODE aclObjectAdd(ACL_OBJ objParam)
        {
            ERR_CODE errCode = ERR_CODE.ERR_SUCCESS;
            ACL_OBJ aclObj;

            errCode = setACLObjInfo(objParam);
            if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;

            aclObj = aclObjectFind(objParam.ObjType, objParam.ObjKey, objParam.ObjPath);
            if (aclObj != null) return ERR_CODE.ERR_ACLOBJ_EXIST;

            errCode = sendACLObjInfo(objParam, null, KERNEL_COMMAND.ACL_OBJECT_ADD);
            if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;

            aclObj = new ACL_OBJ(objParam);
            g_ACLObject.Add(aclObj);

            return errCode;
        }

        public ERR_CODE aclObjectModify(ACL_OBJ objParam)
        {
            ERR_CODE errCode = ERR_CODE.ERR_SUCCESS;
            ACL_OBJ aclObj;

            errCode = setACLObjInfo(objParam);
            if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;

            aclObj = aclObjectFind(objParam.ObjType, objParam.ObjKey, objParam.ObjPath);
            if (aclObj == null) return ERR_CODE.ERR_ACLOBJ_NOT_EXIST;

            errCode = sendACLObjInfo(objParam, null, KERNEL_COMMAND.ACL_OBJECT_MODIFY);
            if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;

            aclObj.Copy(objParam);

            return ERR_CODE.ERR_SUCCESS;
        }

        public ERR_CODE aclObjectDelete(ACL_OBJ objParam)
        {
            ERR_CODE errCode = ERR_CODE.ERR_SUCCESS;
            ACL_OBJ aclObj;

            errCode = setACLObjInfo(objParam);
            if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;

            aclObj = aclObjectFind(objParam.ObjType, objParam.ObjKey, objParam.ObjPath);
            if (aclObj == null) return ERR_CODE.ERR_SUCCESS;

            errCode = sendACLObjInfo(objParam, null, KERNEL_COMMAND.ACL_OBJECT_DELETE);
            if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;

            g_ACLObject.Remove(aclObj);

            return ERR_CODE.ERR_SUCCESS;
        }

        public ERR_CODE aclObjectClear()
        {
            ERR_CODE errCode = ERR_CODE.ERR_SUCCESS;
            Byte[] byteCode = new Byte[sizeof(ERR_CODE)];

            sendMessageDriver(KERNEL_COMMAND.ACL_OBJECT_CLEAR, String.Empty, ref byteCode, (UInt32)byteCode.Length);
            errCode = (ERR_CODE)BitConverter.ToInt32(byteCode, 0);
            if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;

            g_ACLObject.Clear(); ;

            return ERR_CODE.ERR_SUCCESS;
        }

        public ERR_CODE aclObjectList(ONOFF_MODE addCmd, String objPath, out String objList)
        {
            String copyBuf = "", ruleID;
            Int32 offSet = 0;

            objList = "";

            foreach (var obj in g_ACLObject)
            {
                if (String.IsNullOrEmpty(objPath) == false && String.Equals(objPath, obj.ObjPath) == false) continue;

                if (offSet > 0) copyBuf += "\n";

                if (addCmd == ONOFF_MODE.OFM_ON) copyBuf += String.Format("aclobjadd objkey={0} ", obj.ObjKey);

                ruleID = String.Format("objtype={0} objpath=\"{1}\"", CommFunc.ObjTypeToStr(obj.ObjType), obj.ObjPath);

                if (String.IsNullOrEmpty(obj.CrossPath) == false) ruleID += String.Format(" crosspath=\"{0}\"", obj.CrossPath);

                copyBuf += String.Format("{0} action={1} subdir={2} runmode={3} logmode={4} logging={5} sharedperm={6}", ruleID,
                     CommFunc.ActionToStr(obj.DefAction), CommFunc.OnOffModeToStr(obj.SubDir), CommFunc.OnOffModeToStr(obj.RunMode),
                     CommFunc.OnOffModeToStr(obj.LogMode), CommFunc.LoggingTypeToStr(obj.LoggingType), CommFunc.SharedPermToStr(obj.SharedPerm));

                offSet = copyBuf.Length;
            }

            if (copyBuf != "")
            {
                objList = String.Copy(copyBuf);
                objList += "\0";
            }

            return ERR_CODE.ERR_SUCCESS;
        }

        public ERR_CODE AddACLObjFromPol(ACL_OBJ objParam, ACL_POL polParam, Boolean autoLoad = true)
        {
            ERR_CODE errCode = ERR_CODE.ERR_SUCCESS;

            if (autoLoad == true)
            {
                errCode = sendACLObjInfo(objParam, polParam, KERNEL_COMMAND.ACL_OBJECT_POL_ADD);
                if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;
            }

            ACL_DATA newData = new ACL_DATA();
            newData.ACLPol = polParam;
            newData.ACLObj = objParam;

            objParam.ACLPols.ACLData.Add(newData);
            polParam.ACLObjs.ACLData.Add(newData);

            return errCode;
        }

        public ERR_CODE DelACLObjFromPol(ACL_OBJ objParam, ACL_POL polParam)
        {
            ERR_CODE errCode = ERR_CODE.ERR_SUCCESS;

            errCode = sendACLObjInfo(objParam, polParam, KERNEL_COMMAND.ACL_OBJECT_POL_DEL);
            if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;

            foreach (ACL_DATA aclData in objParam.ACLPols.ACLData)
            {
                if (String.Equals(aclData.ACLObj.ObjPath, objParam.ObjPath) == true)
                {
                    if (String.Equals(aclData.ACLPol.PolName, polParam.PolName) == true)
                    {
                        objParam.ACLPols.ACLData.Remove(aclData);
                        polParam.ACLObjs.ACLData.Remove(aclData);

                        break;
                    }
                }
            }

            return errCode;
        }

        public ERR_CODE sendSuperSubInfo(SUPER_SUB superParam, KERNEL_COMMAND cmdParam)
        {
            ERR_CODE ErrCode = ERR_CODE.ERR_SUCCESS;
            String dataBuf;
            Byte[] byteCode = new Byte[sizeof(ERR_CODE)];

            dataBuf = String.Format("subkey={0} subtype={1} subname=\"{2}\" decrypt={3}",
                superParam.SubKey, CommFunc.SubTypeToStr(superParam.SubType),
                superParam.SubName, CommFunc.EffectModeToStr(superParam.DecPerm));

            sendMessageDriver(cmdParam, dataBuf, ref byteCode, (UInt32)byteCode.Length);
            ErrCode = (ERR_CODE)BitConverter.ToInt32(byteCode, 0);

            return ErrCode;
        }

        public ERR_CODE setSuperSubInfo(SUPER_SUB superParam)
        {
            if (String.IsNullOrEmpty(superParam.SubName) == true) return ERR_CODE.ERR_ACLSUB_INVALID_NAME;
            else if (superParam.SubType == SUB_TYPE.SUB_UNKNOWN) return ERR_CODE.ERR_ACLSUB_INVALID_TYPE;
            else if (superParam.DecPerm == EFFECT_MODE.EFT_UNKNOWN) return ERR_CODE.ERR_ACLSUB_INVALID_EFFECT;

            if (superParam.SubType == SUB_TYPE.SUB_USER || superParam.SubType == SUB_TYPE.SUB_GROUP)
            {
                superParam.UserSId = CommFunc.GetUserSId(superParam.SubName);
                if (superParam.UserSId != null)
                {
                    superParam.SubKey = CommFunc.GetSIdKey(superParam.UserSId);
                    superParam.UserSIdBuf = superParam.UserSId.ToString();
                }
                else return ERR_CODE.ERR_ACLSUB_GET_SID_FAIL;
            }
            else if (superParam.SubType == SUB_TYPE.SUB_PROC)
            {
                int lastIndex = superParam.SubName.LastIndexOf('.');
                if (lastIndex == -1) return ERR_CODE.ERR_ACLSUB_NO_PROCESS;
                else if (CommFunc.CheckProcessExt(superParam.SubName.Substring(lastIndex)) == false) return ERR_CODE.ERR_ACLSUB_NO_PROCESS;

                superParam.SubKey = CommFunc.GetObjKey(OBJ_TYPE.OBJ_DIR, superParam.SubName);
            }
            else if (superParam.SubType == SUB_TYPE.SUB_SHARE)
            {
                superParam.SubKey = (UInt64)SUB_TYPE.SUB_SHARE;
                superParam.SubName = SUB_TYPE_STR.SUB_SHARE_STR;
            }

            if (superParam.SubKey == 0) return ERR_CODE.ERR_ACLSUB_GET_SID_FAIL;

            return ERR_CODE.ERR_SUCCESS;
        }

        public SUPER_SUB superSubFind(SUB_TYPE subType, UInt64 subKey, String subName)
        {
            foreach (var superSub in g_SuperSub)
            {
                if (superSub.SubType != subType) continue;

                if (superSub.SubKey != subKey) continue;

                if (String.Equals(superSub.SubName, subName) == false) continue;

                return superSub;
            }

            return null;
        }

        public ERR_CODE superSubAdd(SUPER_SUB superParam, Boolean autoLoad = true)
        {
            ERR_CODE errCode = ERR_CODE.ERR_SUCCESS;
            SUPER_SUB superSub;

            errCode = setSuperSubInfo(superParam);
            if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;

            superSub = superSubFind(superParam.SubType, superParam.SubKey, superParam.SubName);
            if (superSub != null) return ERR_CODE.ERR_ACLSUB_EXIST;

            if (autoLoad == true)
            {
                errCode = sendSuperSubInfo(superParam, KERNEL_COMMAND.SUPER_PROCESS_ADD);
                if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;
            }

            superSub = new SUPER_SUB(superParam);
            g_SuperSub.Add(superSub);

            return errCode;
        }

        public ERR_CODE superSubDelete(SUPER_SUB superParam)
        {
            ERR_CODE errCode = ERR_CODE.ERR_SUCCESS;
            SUPER_SUB superSub;

            errCode = setSuperSubInfo(superParam);
            if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;

            superSub = superSubFind(superParam.SubType, superParam.SubKey, superParam.SubName);
            if (superSub == null) return ERR_CODE.ERR_SUCCESS;

            errCode = sendSuperSubInfo(superParam, KERNEL_COMMAND.SUPER_PROCESS_DELETE);
            if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;

            g_SuperSub.Remove(superSub);

            return ERR_CODE.ERR_SUCCESS;
        }

        public ERR_CODE superSubList(ONOFF_MODE addCmd, out String superSubList)
        {
            String copyBuf = "";
            Int32 offSet = 0;

            foreach (var superSub in g_SuperSub)
            {
                if (offSet > 0) copyBuf += "\n";

                if (addCmd == ONOFF_MODE.OFM_ON)
                {
                    copyBuf += String.Format("suprocadd subkey={0} ", superSub.SubKey);
                }

                copyBuf += String.Format("subtype={0} subname=\"{1}\" decrypt={2}",
                    CommFunc.SubTypeToStr(superSub.SubType), superSub.SubName, CommFunc.EffectModeToStr(superSub.DecPerm));

                offSet = copyBuf.Length;
            }

            superSubList = String.Copy(copyBuf);

            return ERR_CODE.ERR_SUCCESS;
        }

        public ERR_CODE superSubClear()
        {
            ERR_CODE errCode = ERR_CODE.ERR_SUCCESS;
            Byte[] byteCode = new Byte[sizeof(ERR_CODE)];

            sendMessageDriver(KERNEL_COMMAND.SUPER_PROCESS_CLEAR, String.Empty, ref byteCode, (UInt32)byteCode.Length);
            errCode = (ERR_CODE)BitConverter.ToInt32(byteCode, 0);
            if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;

            g_SuperSub.Clear(); ;

            return ERR_CODE.ERR_SUCCESS;
        }

        public Int64 sendMessageDriver(KERNEL_COMMAND cmd, String inString, ref Byte[] outByte, UInt32 outSize)
        {
            const String portName = "\\miniPort";
            Int64 hResult = 0;
            Int32 outBufferSize = 0;
            IntPtr outBuffer = IntPtr.Zero;
            IntPtr pBuffer = IntPtr.Zero;
            IntPtr hPort = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(IntPtr)));
            UInt32 byteResult = 0, inByteLen = 0;
            COMMAND_MESSAGE cmdMsg;
            const UInt32 maxRegSize = 0x100000;

            try
            {
                hResult = NativeAPI.FilterConnectCommunicationPort(portName, 0, IntPtr.Zero, 0, IntPtr.Zero, out hPort);
                if (hResult != 0) return hResult;

                if (outByte != null)
                {
                    outBufferSize = (Int32)maxRegSize;
                    outBuffer = Marshal.AllocHGlobal(outBufferSize);

                    if (outBuffer == IntPtr.Zero) return hResult;
                }

                cmdMsg = new COMMAND_MESSAGE();
                cmdMsg.command = cmd;
                cmdMsg.data = new Byte[4096];
                if (string.IsNullOrEmpty(inString) == false)
                {
                    Array.Copy(Encoding.Default.GetBytes(inString), cmdMsg.data, inString.Length);
                    inByteLen = (UInt32)inString.Length;
                }

                Int32 pBufferSize = Marshal.SizeOf(typeof(COMMAND_MESSAGE));
                pBuffer = Marshal.AllocHGlobal(pBufferSize);

                Marshal.StructureToPtr(cmdMsg, pBuffer, false);

                hResult = NativeAPI.FilterSendMessage(hPort, pBuffer, (UInt32)pBufferSize,
                    outBuffer, (UInt32)outBufferSize, out byteResult);

                if (hResult != 0) return hResult;

                if (outByte != null && byteResult > 0)
                {
                    Marshal.Copy(outBuffer, outByte, 0, (int)outSize);
                }
            }
            finally
            {
                if (hPort != IntPtr.Zero)
                {
                    NativeAPI.CloseHandle(hPort);
                    Marshal.FreeHGlobal(hPort);
                }

                if (outBuffer != IntPtr.Zero) Marshal.FreeHGlobal(outBuffer);

                if (pBuffer != IntPtr.Zero) Marshal.FreeHGlobal(pBuffer);
            }

            return hResult;
        }

    }
}
