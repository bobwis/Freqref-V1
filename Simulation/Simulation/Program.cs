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

        static void Main(string[] args)
        {
            var timerThread = new Thread(TickTock);
            OCXO myOCXO = new OCXO();
            var myGPS = new GPSSource();
            var myPID = new PIDController();
            timerThread.Start(myOCXO);
           
            
            using (StreamWriter file = new StreamWriter("output.csv"))
            {
                while (true)
                {
                    NOP(100);
                     //simulate processing time
                     var currentSimulatedTime = tick/ SIMULATION_CLOCK_MS;  // simulate the latch
                    
                    var gpscount = myGPS.GetCount(currentSimulatedTime);
                    var ocxcount = myOCXO.GetCount(currentSimulatedTime);
                    long tweak = myPID.GetValue(gpscount, ocxcount, currentSimulatedTime, myOCXO.GetDACVAl());
                    if (tweak != -1)
                    {
                        myOCXO.TweakInput(tweak);
                        DateTime centuryBegin = new DateTime(2017, 10, 1);
                        DateTime currentDate = DateTime.Now;

                        long elapsedTicks = currentDate.Ticks - centuryBegin.Ticks;

                        Console.Write($"Tick {currentSimulatedTime } ");

                        Console.Write($" {elapsedTicks} ");

                        Console.ForegroundColor = ConsoleColor.Green;
                        Console.Write($"GPS count = {gpscount}");
                        Console.ForegroundColor = ConsoleColor.Blue;
                        Console.Write($"OCXO count = {ocxcount}");
                        Console.ForegroundColor = ConsoleColor.Yellow;

                        Console.ForegroundColor = ConsoleColor.Yellow;
                        Console.Write($" Tweak DACVAL = {tweak} ");
                    
                        Console.Write($"Current = {myOCXO.Current} ");
                        Console.ForegroundColor = ConsoleColor.Blue;
                        Console.Write($"Target = {myOCXO.Target} ");
                        Console.Write($"\r\n");

                        file.WriteLine(
                            $"{myOCXO.GetDACVAl()} ,{tweak}, {myOCXO.Target}, {myOCXO.Current},{DateTime.Now}, {currentSimulatedTime },{elapsedTicks}");

                  
                        lock (myPID)
                        {
                            tick = 0;
                        }
                    }
                }
            }
            return;
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