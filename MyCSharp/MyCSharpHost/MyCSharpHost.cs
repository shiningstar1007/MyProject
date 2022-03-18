using System.ServiceProcess;
using System.ServiceModel;
using MyCSharp.Service;

namespace MyCSharpHost
{
    partial class MyCSharpHost : ServiceBase
    {
        public ServiceHost serviceHost = null;
        public MyCSharpHost()
        {
            InitializeComponent();
        }

        protected override void OnStart(string[] args)
        {
            // TODO: 여기에 서비스를 시작하는 코드를 추가합니다.
            if (serviceHost != null)
            {
                serviceHost.Close();
            }

            MyCSharp.Service.IMyCSharpService myService = new MyCSharpService();

            serviceHost = new ServiceHost(myService);
            serviceHost.Open();
        }

        protected override void OnStop()
        {
            // TODO: 서비스를 중지하는 데 필요한 작업을 수행하는 코드를 여기에 추가합니다.
            if (serviceHost != null)
            {  
                serviceHost.Close();
                serviceHost = null;
            }
        }
    }
}
