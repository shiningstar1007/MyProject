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
    }
}
