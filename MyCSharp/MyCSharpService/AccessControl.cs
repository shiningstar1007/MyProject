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
