﻿using IMyCSharpService;
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
    }
}
