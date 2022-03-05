using System;
using System.Collections.Generic;
using System.Text;
using System.ServiceModel;
using System.Runtime.InteropServices;
using System.IO;
using System.Security.AccessControl;
using System.Text.RegularExpressions;
using System.Linq;
using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
using System.Management;
using System.Security.Principal;
using Microsoft.Win32;
using MyCSharpService;

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
    }
}
