using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MyCSharpService
{
    class CommDefine
    {

    }

    public static class RegDefine
    {
        public const string REG_KEY_ROOT = @"Software\My MiniFilter\App";
        public const string REG_KEY_CONFIG = @"Software\My MiniFilter\App\Config";
        public const string REG_NAME_ACLPOLICY = "ACLPolicy";
        public const string REG_NAME_ACLSUBJECT = "ACLSubject";
        public const string REG_NAME_ACLOBJECT = "ACLObject";
        public const string REG_NAME_SUPERSUB = "SuperProcess";
    }

    public static class LogDefine
    {
        public const string LOGLEVEL_WARN = "WARN";
        public const string LOGLEVEL_DEBUG = "DEBUG";
        public const string LOGLEVEL_INFO = "INFO";
        public const string LOGLEVEL_ALL = "ALL";

        // log file define
        public const string ServiceLog = @"log\service_log.db";
        public const string AccessLog = @"log\access_log.db";
    }

    public enum ERR_CODE : int
    {
        ERR_SUCCESS = 0x00000000,

        ERR_INVALID_VALUE = 0x00001001,
        ERR_NULL_VALUE = 0x00001002,
        ERR_NO_BUFFER = 0x00001003,
        ERR_TOO_LONG_VALUE = 0x00001004,
        ERR_INVALID_MODE = 0x00001005,
        ERR_MORE_BUFFER = 0x00001006,
        ERR_NOT_COMMAND = 0x00001007,
        ERR_NULL_BUFFER = 0x00001008,
        ERR_OPEN_REG_FAILED = 0x00001009,
        ERR_GET_REG_FAILED = 0x0000100a,

        ERR_ACLPOL_EXIST = 0x00002001,
        ERR_ACLPOL_NOT_EXIST = 0x00002002,

        ERR_ACLOBJ_EXIST = 0x00003001,
        ERR_ACLOBJ_NOT_EXIST = 0x00003002,
        ERR_ACLOBJ_INVALID_TYPE = 0x00003003,
        ERR_ACLOBJ_INVALID_PATH = 0x00003004,
        ERR_ACLOBJ_GET_KEY_FAIL = 0x00003005,

        ERR_ACLSUB_EXIST = 0x00004001,
        ERR_ACLSUB_NOT_EXIST = 0x00004002,
        ERR_ACLSUB_INVALID_TYPE = 0x00004003,
        ERR_ACLSUB_INVALID_POLICY = 0x00004004,
        ERR_ACLSUB_INVALID_NAME = 0x00004005,
        ERR_ACLSUB_GET_SID_FAIL = 0x00004006,
        ERR_ACLSUB_INVALID_EFFECT = 0x00004007,
        ERR_ACLSUB_INVALID_ACTION = 0x00004008,
        ERR_ACLSUB_SID_NOT_EXIST = 0x00004009,
        ERR_ACLSUB_NO_PROCESS = 0x0000400a,
        ERR_ACLSUB_PERM_EXIST = 0x0000400b,

        ERR_SUPERSUB_EXIST = 0x00005001,
        ERR_SUPERSUB_NOT_EXIST = 0x00005002,

        ERR_GET_KEY_FAILED = 0x00006001,
        ERR_DECRYPTION_FAILED = 0x00006002,

        ERR_ACCESS_DENY = 0x00007001,
        ERR_CONNECT_FAILED = 0x00007002,
        ERR_NOT_FOUND_COMMAND = 0x00007003,
    }
}
