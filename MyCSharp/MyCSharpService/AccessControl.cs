using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Principal;
using System.Text;
using System.Threading.Tasks;

namespace MyCSharpService
{
    public enum ONOFF_MODE : int
    {
        OFM_ON = 0,
        OFM_OFF,
    }

    public enum LOGGING_TYPE : int
    {
        LOG_DENY = 0,
        LOG_ALLOW,
        LOG_ALL
    }

    public enum EFFECT_MODE
    {
        EFT_DENY = 0,
        EFT_ALLOW,
        EFT_UNKNOWN
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
        SUB_GROUP,
        SUB_PROC,
        SUB_UNKNOWN
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

    public enum SHARED_PERM : int
    {
        SDP_DECRYPT = 0,
        SDP_ENCRYPT,
        SDP_DENY,
        SDP_UNKNOWN
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

            sendMessageDriver(KERNEL_COMMAND.ACL_POLICY_CLEAR, null, 0, ref byteCode, (UInt32)byteCode.Length);
            errCode = (ERR_CODE)BitConverter.ToInt32(byteCode, 0);
            if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;

            g_ACLPolicy.Clear();

            return ERR_CODE.ERR_SUCCESS;
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
                     pol.PolName, CommFunc.onOffModeToStr(pol.RunMode), CommFunc.onOffModeToStr(pol.LogMode));

                offSet = copyBuf.Length;
            }

            polList = String.Copy(copyBuf);
            polList += "\0";

            return ERR_CODE.ERR_SUCCESS;
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
            ERR_CODE errCode = ERR_CODE.ERR_KE_SUCCESS;
            ACL_SUB aclSub;

            errCode = setACLSubInfo(subParam);
            if (errCode != ERR_CODE.ERR_SUCCESS) return errCode;

            aclSub = aclSubjectFind(subParam.SubType, subParam.SubKey, subParam.SubName);
            if (aclSub == null) return ERR_CODE.ERR_SUCCESS;

            errCode = sendACLSubInfo(subParam, null, KERNEL_COMMAND.ACL_SUBJECT_DELETE);
            if (errCode != ERR_CODE.ERR_KE_SUCCESS) return errCode;

            g_ACLSubject.Remove(aclSub);

            return ERR_CODE.ERR_SUCCESS;
        }

        public ERR_CODE aclSubjectClear()
        {
            ERR_CODE errCode = ERR_CODE.ERR_KE_SUCCESS;
            Byte[] byteCode = new Byte[sizeof(ERR_CODE)];

            sendMessageDriver(PSKE_COMMAND.ACL_SUBJECT_CLEAR, null, 0, ref byteCode, (UInt32)byteCode.Length);
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

                ruleID = String.Format("subtype={0} subname=\"{1}\"", CommFunc.subTypeToStr(sub.SubType), sub.SubName);
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
                            ruleID, polData.ACLPol.PolName, CommFunc.actionToStr(polData.SubPerm.Action),
                            CommFunc.effectModeToStr(polData.SubPerm.Effect), CommFunc.effectModeToStr(polData.SubPerm.DecPerm));

                        if (polData.SubPerm.ProcUser.SubType != SUB_TYPE.SUB_UNKNOWN)
                        {
                            copyBuf += String.Format(" subsubtype={0} subsubname=\"{1}\"",
                            CommFunc.subTypeToStr(polData.SubPerm.ProcUser.SubType), polData.SubPerm.ProcUser.SubName);

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
            ERR_CODE = (ERR_CODE)BitConverter.ToInt32(byteCode, 0);
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

                ruleID = String.Format("objtype={0} objpath=\"{1}\"", CommFunc.objTypeToStr(obj.ObjType), obj.ObjPath);

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

    }
}
