using System;
using System.IO;
using System.Threading;
using Simulation;
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
        public static readonly ulong Fast = 1000;
    }


    public static class World
    {
        public static ulong RESOLUTION = DEFINES.Fast; // can 'speed up' the world, but less fine grained response

        public static decimal CLOCK_RATE = 1e8m; // 10ps
        static Thread _worldSimulationThread;

        public static void BeginSimulation()
        {
            _worldSimulationThread = new Thread(Simulation);
            _worldSimulationThread.Start();
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
                // the world will be driven at 1 tick per 10pS  (100Mhz)
                // use the WorldClock to get the correct units
                
                //example frequency counter to test timing
                lock (_fc.Locker)  // these two calls have to be atomic or it will 'look' like drift
                {
                    _fc.EndTick = (ulong)_wc.GetClock();
                    _fc.PulseCount += RESOLUTION; // 
                }
                // nugget of simulation

                _myOcxo.Tick(dT);
                _controlDevice.Tick(dT);

                dT = _wc.GetClock();
            }

            #endregion

            Console.WriteLine("Simulation Ends");
        }

        public static void Join()
        {
            _worldSimulationThread.Join();
        }

        public static void DisplayWorldStatus(int screenrefresh)
        {
            Console.SetWindowPosition(0,0);
            Console.Clear();
            Console.BackgroundColor=ConsoleColor.White;
            Console.ForegroundColor= ConsoleColor.Black;

            Console.WriteLine($"Resolution {RESOLUTION} Current Ticks {_wc.GetClock()} Sim Frequency {_fc.GetFrequencyInHertz() / 1e6m:F3} MHz Refresh {screenrefresh/1000m}");
            Console.WriteLine($"------------------------------------------------------------------");
            Console.BackgroundColor = ConsoleColor.Black;
            Console.ForegroundColor = ConsoleColor.White;
            Console.WriteLine($"\r\n");
            Console.Write($"GPS Frequency {_myGPS.Current:F3} ");
            Console.WriteLine("Jitter {0}", _myGPS.AddJitter ? "ON" : "OFF");
            Console.WriteLine($"OCXO Current Frequency {_myOcxo.Current:F3} Target Frequency {_myOcxo.Target:F3}");
            Console.WriteLine($"DAC {_myOcxo.GetDAC():F3}  ");
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