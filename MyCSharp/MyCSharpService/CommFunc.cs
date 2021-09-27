using System;
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
    }
}
