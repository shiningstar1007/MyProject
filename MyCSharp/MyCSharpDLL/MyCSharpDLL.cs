using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows.Input;
using System.Security.AccessControl;
using System.Windows;
using System.ServiceModel.Description;
using System.ServiceModel;
using MyCSharp.Service;

namespace MyCSharp.MyCSharpDLL
{
    public class MyCSharpDLL
    {
        public void Start()
        {
            var factory = new ChannelFactory<IMyCSharpService>(new NetNamedPipeBinding(),
                                                        new EndpointAddress("net.pipe://localhost/test/myservice"));

            IMyCSharpService proxy = factory.CreateChannel();

            proxy.testFunc();

            (proxy as IDisposable).Dispose();
        }
    }
}
