﻿using System;
using System.Diagnostics;
using System.IO;
using System.Threading;

namespace Simulation
{
    

    public class Program
    {
        public static uint SIMULATION_CLOCK_MS = 10;  // 10 ticks = 1ms   , dictates speed of simulation

        private static ulong tick;

        private static void Main(string[] args)
        {
           World.BeginSimulation();


            
            using (var file = new StreamWriter("output.csv"))
            {
                var startDateTime = DateTime.Now;
                var timerThread = new Thread(TickTock);
                var myOCXO = new OCXO();
                var myGPS = new GPSSource();
                myGPS.AddJitter = false;
                var myPID = new PIDController();
                timerThread.Start(myOCXO);
   myOCXO.SetDAC(100);
                while (true)
                {
                    NOP(100);
                     //simulate processing time
                    var currentSimulatedTime = tick/ SIMULATION_CLOCK_MS;  // simulate the latch
                    ulong gpscount = (ulong) myGPS.GetCount(currentSimulatedTime);
                    ulong ocxcount = (ulong) myOCXO.GetCount(currentSimulatedTime);



                 
//                    long tweak = myPID.Process(myOCXO, gpscount, ocxcount, currentSimulatedTime);
                 
//                    if (tweak != -1)
//                    {
//                        // everything has been done, this is just data output

//                        var elapsedTicks = DateTime.Now.Ticks - startDateTime.Ticks;
//#if false
//                        Console.Write($"Interval {currentSimulatedTime }\t");

//                        Console.Write($" {elapsedTicks}\t");

//                        Console.ForegroundColor = ConsoleColor.Green;
//                        Console.Write($"GPS count = {gpscount}\t");
//                        Console.ForegroundColor = ConsoleColor.Cyan;
//                        Console.Write($"OCXO count = {ocxcount}\t");
//                        Console.ForegroundColor = ConsoleColor.Red;
//                        Console.Write($"diff count = {ocxcount-gpscount}\t");
//                        Console.ForegroundColor = ConsoleColor.Yellow;
//                        Console.Write($" DACVAL = {myOCXO.GetDAC()}\t");
//                        Console.Write($"Current = {myOCXO.Current}\t");
//                        Console.ForegroundColor = ConsoleColor.Cyan;
//                        Console.Write($"Target = {myOCXO.Target} ");
//                        Console.Write($"\r\n");
//#endif
//                        //                     file.WriteLine(
//                        //                         $"{myOCXO.GetDAC()} ,{tweak}, {myOCXO.Target}, {myOCXO.Current},{DateTime.Now}, {currentSimulatedTime },{elapsedTicks}");

//                                             file.WriteLine(
//                        $"{myOCXO.GetDAC()} , {elapsedTicks}");
//file.Flush();
//                        lock (myPID)
//                        {
//                            tick = 0;
//                        }
//                    }
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