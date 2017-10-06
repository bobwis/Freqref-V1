using System;
using System.Diagnostics;
using System.IO;
using System.Threading;

namespace Simulation
{
    

    public class Program
    {
        private static void Main(string[] args)
        {
            var refreshDelay = 1000;

            World.BeginSimulation();

            // joining the world will halt this thread
            //World.Join();

            using (var file = new StreamWriter("output.csv"))
            {
                // this is it, nice and tidy
                while (true)
                {
                    Thread.Sleep(refreshDelay);
                    World.DisplayWorldStatus(refreshDelay);
                    World.DumpToFile(file);
                }
            }
        }
    }
}