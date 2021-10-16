using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MyCSharpService
{
    class CommFunc
    {
        public enum RUN_MODE : int
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

        public RUN_MODE StrToRunMode(String RunModeStr)
        {
            if (RunModeStr == RUN_MODE_STR.RUN_TEST_STR) return RUN_MODE.RUN_TEST;
            else if (RunModeStr == RUN_MODE_STR.RUN_DISABLE_STR) return RUN_MODE.RUN_DISABLE;

            return RUN_MODE.RUN_NORMAL;
        }

        public String RunModeToStr(RUN_MODE RunMode)
        {
            if (RunMode == RUN_MODE.RUN_TEST) return RUN_MODE_STR.RUN_TEST_STR;
            else if (RunMode == RUN_MODE.RUN_DISABLE) return RUN_MODE_STR.RUN_DISABLE_STR;

            return RUN_MODE_STR.RUN_NORMAL_STR;
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

        public SUB_TYPE StrToSubType(String SubTypeStr)
        {
            if (SubTypeStr == SUB_TYPE_STR.SUB_USER_STR) return SUB_TYPE.SUB_USER;
            else if (SubTypeStr == SUB_TYPE_STR.SUB_PROC_STR) return SUB_TYPE.SUB_PROC;
            else if (SubTypeStr == SUB_TYPE_STR.SUB_SHARE_STR) return SUB_TYPE.SUB_SHARE;
            else if (SubTypeStr == SUB_TYPE_STR.SUB_GROUP_STR) return SUB_TYPE.SUB_GROUP;

            return SUB_TYPE.SUB_UNKNOWN;
        }

        public String SubTypeToStr(SUB_TYPE SubType)
        {
            if (SubType == SUB_TYPE.SUB_USER) return SUB_TYPE_STR.SUB_USER_STR;
            else if (SubType == SUB_TYPE.SUB_PROC) return SUB_TYPE_STR.SUB_PROC_STR;
            else if (SubType == SUB_TYPE.SUB_SHARE) return SUB_TYPE_STR.SUB_SHARE_STR;
            else if (SubType == SUB_TYPE.SUB_GROUP) return SUB_TYPE_STR.SUB_GROUP_STR;

            return "";
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

        public OBJ_TYPE StrToObjType(String ObjTypeStr)
        {
            if (ObjTypeStr == OBJ_TYPE_STR.OBJ_FILE_STR) return OBJ_TYPE.OBJ_FILE;
            else if (ObjTypeStr == OBJ_TYPE_STR.OBJ_DIR_STR) return OBJ_TYPE.OBJ_DIR;

            return OBJ_TYPE.OBJ_UNKNOWN;
        }

        public String ObjTypeToStr(OBJ_TYPE ObjType)
        {
            if (ObjType == OBJ_TYPE.OBJ_FILE) return OBJ_TYPE_STR.OBJ_FILE_STR;
            else if (ObjType == OBJ_TYPE.OBJ_DIR) return OBJ_TYPE_STR.OBJ_DIR_STR;

            return "";
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

        public EFFECT_MODE StrToEffectMode(String EffectModeStr)
        {
            if (EffectModeStr == EFFECT_MODE_STR.EFT_ALLOW_STR) return EFFECT_MODE.EFT_ALLOW;
            else if (EffectModeStr == EFFECT_MODE_STR.EFT_DENY_STR) return EFFECT_MODE.EFT_DENY;

            return EFFECT_MODE.EFT_UNKNOWN;
        }

        public String EffectModeToStr(EFFECT_MODE EffectMode)
        {
            if (EffectMode == EFFECT_MODE.EFT_ALLOW) return EFFECT_MODE_STR.EFT_ALLOW_STR;
            else if (EffectMode == EFFECT_MODE.EFT_DENY) return EFFECT_MODE_STR.EFT_DENY_STR;

            return "";
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

        public LOGGING_TYPE StrToLoggingType(String LoggingTypeStr)
        {
            if (LoggingTypeStr == LOGGING_TYPE_STR.LOG_ALLOW_STR) return LOGGING_TYPE.LOG_ALLOW;
            else if (LoggingTypeStr == LOGGING_TYPE_STR.LOG_DENY_STR) return LOGGING_TYPE.LOG_DENY;

            return LOGGING_TYPE.LOG_ALL;
        }

        public String LoggingTypeToStr(LOGGING_TYPE LoggingType)
        {
            if (LoggingType == LOGGING_TYPE.LOG_ALLOW) return LOGGING_TYPE_STR.LOG_ALLOW_STR;
            else if (LoggingType == LOGGING_TYPE.LOG_DENY) return LOGGING_TYPE_STR.LOG_DENY_STR;

            return LOGGING_TYPE_STR.LOG_ALL_STR;
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

        public SHARED_PERM StrToSharedPerm(String SharedPermStr)
        {
            if (SharedPermStr == SHARED_PERM_STR.SDP_DECRYPT_STR) return SHARED_PERM.SDP_DECRYPT;
            else if (SharedPermStr == SHARED_PERM_STR.SDP_ENCRYPT_STR) return SHARED_PERM.SDP_ENCRYPT;
            else if (SharedPermStr == SHARED_PERM_STR.SDP_DENY_STR) return SHARED_PERM.SDP_DENY;

            return SHARED_PERM.SDP_UNKNOWN;
        }

        public String SharedPermToStr(SHARED_PERM SharedPerm)
        {
            if (SharedPerm == SHARED_PERM.SDP_DECRYPT) return SHARED_PERM_STR.SDP_DECRYPT_STR;
            else if (SharedPerm == SHARED_PERM.SDP_ENCRYPT) return SHARED_PERM_STR.SDP_ENCRYPT_STR;
            else if (SharedPerm == SHARED_PERM.SDP_DENY) return SHARED_PERM_STR.SDP_DENY_STR;

            return "";
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

        public ONOFF_MODE StrToOnOffMode(String OnOffModeStr)
        {
            if (OnOffModeStr == ONOFF_MODE_STR.OFM_ON_STR) return ONOFF_MODE.OFM_ON;

            return ONOFF_MODE.OFM_OFF;
        }

        public String OnOffModeToStr(ONOFF_MODE OnOffMode)
        {
            if (OnOffMode == ONOFF_MODE.OFM_ON) return ONOFF_MODE_STR.OFM_ON_STR;

            return ONOFF_MODE_STR.OFM_OFF_STR;
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

        public ACL_ACTION StrToAction(String ActionStr)
        {
            String[] ActionBuf;
            ACL_ACTION Action = 0;

            if (ActionStr == ACL_ACTION_STR.ACT_ALL_STR) return ACL_ACTION.ACT_ALL;

            ActionBuf = ActionStr.Split(new char[] { ',' });

            foreach (var data in ActionBuf)
            {
                if (data == ACL_ACTION_STR.ACT_READ_STR) Action |= ACL_ACTION.ACT_READ;
                else if (data == ACL_ACTION_STR.ACT_WRITE_STR) Action |= ACL_ACTION.ACT_WRITE;
                else if (data == ACL_ACTION_STR.ACT_TRAVERSE_STR) Action |= ACL_ACTION.ACT_TRAVERSE;
                else if (data == ACL_ACTION_STR.ACT_EXECUTE_STR) Action |= ACL_ACTION.ACT_EXECUTE;
                else if (data == ACL_ACTION_STR.ACT_DELETE_STR) Action |= ACL_ACTION.ACT_DELETE;
                else if (data == ACL_ACTION_STR.ACT_CREATE_STR) Action |= ACL_ACTION.ACT_CREATE;
                else if (data == ACL_ACTION_STR.ACT_DEC_STR) Action |= ACL_ACTION.ACT_DEC;
            }

            return Action;
        }

        public String ActionToStr(ACL_ACTION Action)
        {
            var ActionStr = new StringBuilder();

            if (Action == ACL_ACTION.ACT_ALL) return ACL_ACTION_STR.ACT_ALL_STR;
            else
            {
                if ((Action & ACL_ACTION.ACT_READ) == ACL_ACTION.ACT_READ) ActionStr.Append(ACL_ACTION_STR.ACT_READ_STR);
                if ((Action & ACL_ACTION.ACT_WRITE) == ACL_ACTION.ACT_WRITE) ActionStr.Append("," + ACL_ACTION_STR.ACT_WRITE_STR);
                if ((Action & ACL_ACTION.ACT_TRAVERSE) == ACL_ACTION.ACT_TRAVERSE) ActionStr.Append("," + ACL_ACTION_STR.ACT_TRAVERSE_STR);
                if ((Action & ACL_ACTION.ACT_EXECUTE) == ACL_ACTION.ACT_EXECUTE) ActionStr.Append("," + ACL_ACTION_STR.ACT_EXECUTE_STR);
                if ((Action & ACL_ACTION.ACT_DELETE) == ACL_ACTION.ACT_DELETE) ActionStr.Append("," + ACL_ACTION_STR.ACT_DELETE_STR);
                if ((Action & ACL_ACTION.ACT_CREATE) == ACL_ACTION.ACT_CREATE) ActionStr.Append("," + ACL_ACTION_STR.ACT_CREATE_STR);
                if ((Action & ACL_ACTION.ACT_DEC) == ACL_ACTION.ACT_DEC) ActionStr.Append("," + ACL_ACTION_STR.ACT_DEC_STR);
            }

            return ActionStr.ToString();
        }
    }
}
