using System.IO;
using System.Net.Mime;
using System.Runtime.CompilerServices;
using System.Threading;

namespace Simulation
{
    

    public class Program
    {
        private static void Main(string[] args)
        {
            var refreshDelay = 250;

            World.BeginSimulation();


            
            // joining the world will halt this thread
            //World.Join();

                // this is it, nice and tidy
                while (true)
                {
                    Thread.Sleep(refreshDelay);
                    World.DisplayWorldStatus(refreshDelay);
                }
         
        }

         
    }
}