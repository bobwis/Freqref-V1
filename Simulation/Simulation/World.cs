using System;
using System.Threading;
using Simulation;

namespace Simulation
{
    /*
     *   Whole world in your hands
     */
    public class DEFINES
    {
        public static readonly decimal UltraFine = 1m;
        public static readonly decimal Fine = 10m;
        public static readonly decimal Normal = 100;
        public static readonly decimal Fast = 1000;
    }


    public static class World
    {
        public static decimal RESOLUTION = DEFINES.Normal; // can 'speed up' the world, but less fine grained response

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

        static void Simulation()
        {
            var wc = new WorldClock();
            Console.WriteLine("Simulation Begins");


            #region Playground - Everything in here happens in a picosecond

            //set up the devices
            var fc = new FrequencyCounter {StartTick = 0};
            var myOCXO = new OCXO();
            var myGPS = new GPSSource();
            var controlDevice = new PIDController();
            // connect the devices

            myOCXO.GPS = myGPS;
            myGPS.WorldClock = wc;
            myOCXO.WorldClock = wc;
            controlDevice.GPS = myGPS;
            controlDevice.OCXO = myOCXO;

            while (wc.Tick(RESOLUTION))
            {
                // the world will be driven at 1 tick per 10pS  (100Mhz)
                // use the WorldClock to get the correct units

                //example frequency counter to test timing
                fc.EndTick = (ulong) wc.GetClock();
                fc.PulseCount += RESOLUTION; // 
                if (wc.GetClock() % (CLOCK_RATE / 1e3m) == 0)
                {
                    //This is called every msecond in sim time
                    var f = fc.GetFrequencyInHertz();
                    Console.WriteLine("Freq = {0} Hz, {1} MHz", f, f / 1e6M);
                    // purely to check the clock and the maths is good before we start relying on it
                }

                Console.Write("{0}\r", wc.GetClock());

                // nugget of simulation
            }

            #endregion

            Console.WriteLine("Simulation Ends");
        }

        public static void Join()
        {
            _worldSimulationThread.Join();
        }
    }


    public class FrequencyCounter
    {
        public decimal StartTick;

        public decimal EndTick;

        public decimal PulseCount;

        public decimal GetFrequencyInHertz()
        {
            //picoseconds per pulse
            var unitTime_per_pulse = (EndTick - StartTick) / PulseCount;
            return World.CLOCK_RATE * unitTime_per_pulse;
        }
    }

    // 1 tick = 1 picosecond
    public class WorldClock
    {
        private decimal _clock;

        public decimal GetClock()
        {
            lock (this)
            {
                return _clock;
            }
        }

        public decimal MicroSeconds()
        {
            return GetClock() / 1e2m;
        }

        public decimal MilliSeconds()
        {
            return GetClock() / 1e5m;
        }

        public decimal Seconds()
        {
            return GetClock() / 1e8m;
        }

        public bool Tick(decimal interval)
        {
            try
            {
                lock (this)
                {
                    _clock += interval;
                }
            }
            catch (Exception e)
            {
                return false;
            }
            return true;
        }
    }
}