﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MyCSharpService
{
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
    class CommFunc
    {
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

        public SHARED_PERM StrToSharedPerm(String SharedPermStr)
        {
            if (SharedPermStr == SHARED_PERM_STR.SDP_DECRYPT_STR) return SHARED_PERM.SDP_DECRYPT;
            else if (SharedPermStr == SHARED_PERM_STR.SDP_ENCRYPT_STR) return SHARED_PERM.SDP_ENCRYPT;
            else if (SharedPermStr == SHARED_PERM_STR.SDP_DENY_STR) return SHARED_PERM.SDP_DENY;

            return SHARED_PERM.SDP_UNKNOWN;
        }
    }
}
