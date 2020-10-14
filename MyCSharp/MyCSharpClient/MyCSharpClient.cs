using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using MyCSharp.MyCSharpDLL;

namespace MyCSharpClient
{
    class MyCSharpClient
    {
        static void Main(string[] args)
        {
            MyCSharpDLL myCSharpDLL = new MyCSharpDLL();

            myCSharpDLL.Start();
        }
    }
}
