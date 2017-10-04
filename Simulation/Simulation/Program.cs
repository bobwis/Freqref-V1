﻿using System;
using System.IO;
using System.Runtime.CompilerServices;
using System.Threading;

namespace Simulation
{



    public class Program
    {
        private static long tick = 0;

        static void Main(string[] args)
        {
            var timerThread = new Thread(TickTock);
            OCXO myOCXO = new OCXO();
            var myGPS = new GPSSource();
            var myPID = new PIDController();
            timerThread.Start(myOCXO);
            Int64 responsecount = 0;

            using (System.IO.StreamWriter file = new StreamWriter("output.csv"))
            {
                while (true)
                {
                    //var tweak = int.Parse(Console.ReadLine());
                    responsecount = responsecount + tick;
                    var gpscount = myGPS.GetCount(responsecount);
                    var ocxcount = myOCXO.GetCount(responsecount);
                    long tweak = myPID.GetValue(gpscount, ocxcount, responsecount , myOCXO.GetDACVAl());
                    if (tweak != -1)
                    {

                        Console.Write($"Tick {tick} ");
                        Console.ForegroundColor = ConsoleColor.Green;
                                            Console.Write($"GPS count = {gpscount}");
                                            Console.ForegroundColor = ConsoleColor.Blue;
                                            Console.Write($"OCXO count = {ocxcount}");
                                            Console.ForegroundColor = ConsoleColor.Yellow;
                        
                        Console.ForegroundColor = ConsoleColor.Yellow;
                        Console.Write($" Tweak DACVAL = {tweak} ");
                        Console.Write($"{responsecount}");
                        Console.Write($"Current = {myOCXO.Current} ");
                        Console.ForegroundColor = ConsoleColor.Blue;
                        Console.Write($"Target = {myOCXO.Target} ");
                        Console.Write($"\r\n");
                        file.WriteLine($"{myOCXO.GetDACVAl()} ,{tweak}, {myOCXO.Target}, {myOCXO.Current},{responsecount},{DateTime.Now}");
                        
                        myOCXO.TweakInput(tweak);
                        responsecount = 0;
                    }


                }

            }
            return;
        }


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
    }     
}