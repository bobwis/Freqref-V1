using System;
using System.Diagnostics;
using System.IO;
using System.Threading;



namespace Simulation
{
    

    public class Program
    {
        public static uint SIMULATION_CLOCK_MS = 10;  // 10 ticks = 1ms   , dictates speed of simulation

        private static ulong tick = 0;

        private static void Main(string[] args)
        {
           
            
            using (var file = new StreamWriter("output.csv"))
            {
                var timerThread = new Thread(TickTock);
                var myOCXO = new OCXO();
                var myGPS = new GPSSource();
                var myPID = new PIDController();
                timerThread.Start(myOCXO);

                while (true)
                {
                    NOP(100);
                     //simulate processing time
                    var currentSimulatedTime = tick/ SIMULATION_CLOCK_MS;  // simulate the latch
                    var gpscount = myGPS.GetCount(currentSimulatedTime);
                    var ocxcount = myOCXO.GetCount(currentSimulatedTime);


                    //myOCXO.SetDAC(2500);

                    long tweak = myPID.Process(myOCXO, gpscount, ocxcount, currentSimulatedTime);
                    if (tweak != -1)
                    {
                        // everything has been done, this is just data output
                        var startDateTime= new DateTime(2017, 10, 1);
                        var currentDate = DateTime.Now;
                        var elapsedTicks = currentDate.Ticks - startDateTime.Ticks;
                        Console.Write($"Interval {currentSimulatedTime }\t");

                        Console.Write($" {elapsedTicks}\t");

                        Console.ForegroundColor = ConsoleColor.Green;
                        Console.Write($"GPS count = {gpscount}\t");
                        Console.ForegroundColor = ConsoleColor.Cyan;
                        Console.Write($"OCXO count = {ocxcount}\t");
                        Console.ForegroundColor = ConsoleColor.Yellow;
                        Console.ForegroundColor = ConsoleColor.Yellow;
                        Console.Write($" Tweak DACVAL = {tweak}\t");
                        Console.Write($"Current = {myOCXO.Current}\t");
                        Console.ForegroundColor = ConsoleColor.Cyan;
                        Console.Write($"Target = {myOCXO.Target} ");
                        Console.Write($"\r\n");

                        file.WriteLine(
                            $"{myOCXO.GetDAC()} ,{tweak}, {myOCXO.Target}, {myOCXO.Current},{DateTime.Now}, {currentSimulatedTime },{elapsedTicks}");

                  
                        lock (myPID)
                        {
                            tick = 0;
                        }
                    }
                }
            }
        }

        // model time as a physical constant outside the model
        // dont want delays in processing to slow down time :-)
        private static void TickTock(object ocxo)
        {
            while (true)
            {
                //  Thread.Sleep(1);
                lock (ocxo)
                {
                    tick++;
                    (ocxo as OCXO)?.Tick();
                }
            }
        }

        private static void NOP(double duration)  //higher rate than thread.sleep
        {
            var sw = Stopwatch.StartNew();
            while (sw.ElapsedTicks < duration)
            {
            }
        }
    }
}