using System;
using System.IO;
using System.Threading;
using ThreadState = System.Threading.ThreadState;

namespace Simulation
{
    /*
     *   Whole world in your hands
     */
    public class DEFINES
    {
        public static readonly ulong UltraFine = 1;
        public static readonly ulong Fine = 10;
        public static readonly ulong Normal = 100;
        public static readonly ulong Fast = 100000;
    }


    public static class World
    {
        public static ulong RESOLUTION = 100000; // can 'speed up' the world, but less fine grained response

        public static decimal CLOCK_RATE = 1e8m; // 10ns
        static Thread _worldSimulationThread;

        public static void BeginSimulation()
        {
            _worldSimulationThread = new Thread(Simulation);
            _worldSimulationThread.Start();
            
            Console.SetWindowSize(Console.LargestWindowWidth, Console.LargestWindowHeight);
            Console.SetWindowPosition(0, 0);
        }

        public static void EndSimulation()
        {
            if (_worldSimulationThread.ThreadState == ThreadState.Running)
            {
                _worldSimulationThread.Abort();
            }
        }

        private static WorldClock _wc;
        private static OCXO _myOcxo;
        private static GPSSource _myGPS;
        private static PIDController _controlDevice;
        private static FrequencyCounter _fc;


        static void Simulation()
        {
            _wc = new WorldClock();
            Console.WriteLine("Simulation Begins");


            #region Playground - Everything in here happens in a picosecond

            //set up the devices

            _fc = new FrequencyCounter {StartTick = 0};
            _myOcxo = new OCXO();
            _myGPS = new GPSSource();
            _controlDevice = new PIDController();
            // connect the devices

            _myOcxo.GPS = _myGPS;
            _myGPS.WorldClock = _wc;
            _myOcxo.WorldClock = _wc;
            _controlDevice.GPS = _myGPS;
            _controlDevice.OCXO = _myOcxo;

            ulong dT = 0;
            while (_wc.Tick(RESOLUTION))
            {
                // the world will be driven at 1 tick per 10nS  (100Mhz)
                // use the WorldClock to get the correct units

                //example frequency counter to test timing
                lock (_fc.Locker) // these two calls have to be atomic or it will 'look' like drift
                {
                    _fc.EndTick = (ulong) _wc.GetClock();
                    _fc.PulseCount += RESOLUTION; // 
                    // nugget of simulation

                    _myOcxo.Tick(dT);
                    _controlDevice.Tick(dT);
                }

                dT = _wc.GetClock();
            }

            #endregion

            Console.WriteLine("Simulation Ends");
        }

        public static void Join()
        {
            _worldSimulationThread.Join();
        }


        const char BLANK = ' ';
        const char DOT = '.';
        const char X = 'x';
        const int cMaxLineChars = 69;
        const int cHalf = cMaxLineChars / 2;
        static char[] LINE = new char[cMaxLineChars];

        private static Random r = new Random();


        // Bob, just toggle this to show line by line instead
        public static bool ShowStatusScreen = false;

        public static void DisplayWorldStatus(int screenrefresh)
        {
            if (ShowStatusScreen)
            {
                lock (_fc.Locker)
                {
                    Console.Clear();
                    Console.BackgroundColor = ConsoleColor.White;
                    Console.ForegroundColor = ConsoleColor.Black;

                    Console.WriteLine(
                        $"Resolution {RESOLUTION} Current Ticks {_wc.GetClock()} Sim Frequency {_fc.GetFrequencyInHertz() / 1e6m:F3} MHz Refresh {screenrefresh / 1000m}");
                    Console.WriteLine($"------------------------------------------------------------------");
                    Console.BackgroundColor = ConsoleColor.Black;
                    Console.ForegroundColor = ConsoleColor.White;
                    Console.WriteLine($"\r\n");
                    Console.Write($"GPS Frequency {_myGPS.Current:F6} ");
                    Console.WriteLine("Jitter {0}", _myGPS.AddJitter ? "ON" : "OFF");
                    Console.Write($"OCXO Current Frequency ");
                    Console.ForegroundColor = ConsoleColor.Red;
                    Console.Write($"{_myOcxo.Current:F6} ");
                    Console.ForegroundColor = ConsoleColor.White;
                    Console.WriteLine($"Target Frequency {_myOcxo.Target:F6}");
                    Console.WriteLine(
                        $"OCXO Count {_controlDevice.ocxocount:F6} GPS Count {_controlDevice.gpscount:F6}");
                    Console.WriteLine(
                        $"DAC {_myOcxo.GetDAC():F3}  Next Interval {_controlDevice.Interval}   - PRESS D = DACVAL - ANY FOR DISTURBANCE");
                    DoThePlot();
                    while (Console.KeyAvailable)
                    {
                        var info = Console.ReadKey();
                        if (info.Key == ConsoleKey.UpArrow)

                        {
                            _controlDevice.OCXO.FrequencyDrift();
                            Console.WriteLine($"DAC DRIFT!!!!!!!!!!\r");
                        }
                        else if (info.Key == ConsoleKey.D)
                        {
                            _myOcxo.SetDAC(Int32.Parse(Console.ReadLine() ?? "1000"));
                        }

                        else
                        {
                            Console.WriteLine($"Randomise!!!!!!!!!!\r");
                            _myOcxo.SetDAC(r.Next(4096));
                        }
                    }
                }
            }
        }

        static void fillUp(char[] line, char WithChar = '\0')
        {
            for (int i = 0; i < line.Length; i++)
            {
                line[i] = WithChar;
            }
        }

        static void DoThePlot()
        {
            fillUp(LINE, WithChar: DOT); // line of dots for "vertical" axis
            Console.WriteLine(LINE);
            fillUp(LINE, WithChar: BLANK); // clear the line
            PlotFunc();
        }

        delegate decimal FUNC(int X);


        private static int TARGETDAC = 2614; // graph centre


        static void PlotFunc()
        {
            var log = _controlDevice.GetLog();
            if (log.Count <= 0) return;
            int maxval = 40; //arbitrary values
            int loc;
            LINE[cHalf] = DOT; // for "horizontal" axis
            for (int x = 0; x < maxval; x++)
            {
                int pos = log.Count - maxval + x; // 50 log points
                if (pos < 0) pos = 0;

                DataPoint d = log[Math.Min(pos, log.Count)];


                // fit result (0-5000) into 

                loc = (int) (((d.data - TARGETDAC) / 128) * cHalf) + cHalf; // just screen position in characters
                if (loc < 0) loc = 0;
                if (loc > 68) loc = 68;
                LINE[loc] = X;
                Console.Write(LINE);
                Console.WriteLine("{0:F5} {1}", d.currentError, d.data);
                fillUp(LINE, WithChar: BLANK); // blank the line, remove X point
                LINE[cHalf] = DOT; // for horizontal axis
            }
        }

        public static void DumpToFile(StreamWriter file)
        {
            file.WriteLine($"{_myOcxo.GetDAC()} ,{_wc.MilliSeconds()}");

            //                                             file.WriteLine(
            //                        $"{myOCXO.GetDAC()} , {elapsedTicks}");
            file.Flush();
        }
    }
}