using System;
using System.Collections.Generic;
using System.Configuration.Install;
using System.Reflection;
using System.ServiceProcess;
using System.Text;

namespace MyCSharpHost
{
    class Install
    {
        static void Main(string[] args)
        {
            if (System.Environment.UserInteractive)
            {
                string parameter = string.Concat(args);
                try
                {
                    switch (parameter)
                    {
                        case "-install":
                            ManagedInstallerClass.InstallHelper(new string[] { "/LogFile=", Assembly.GetExecutingAssembly().Location });
                            break;
                        case "-uninstall":
                            ManagedInstallerClass.InstallHelper(new string[] { "/LogFile=", "/u", Assembly.GetExecutingAssembly().Location });
                            break;
                    }
                }
                catch (Exception)
                {
                    // ignore error
                }
            }
            else
            {
                ServiceBase[] ServicesToRun;
                ServicesToRun = new ServiceBase[]
                {
                    new MyCSharpHost()
                };
                ServiceBase.Run(ServicesToRun);
            }
        }
    }
}
