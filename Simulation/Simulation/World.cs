using System;
using System.Threading;

namespace Simulation
{
    /*
     *   Whole world in your hands
     */ 
    

    public static class World
    {
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
            WorldClock wc = new WorldClock();
            Console.WriteLine("Simulation Begins");

            var fc = new FrequencyCounter();

            fc.StartTick = 0;
            while (wc.Tick())
            {
                // the world will be driven at 1 tick per pS
                // use the WorldClock to get the correct units
                fc.EndTick = (ulong)wc.GetClock();
                fc.PulseCount++;
                var f = fc.GetFrequencyInHertz();
                Console.WriteLine("Freq = {0} Hz, {1} MHz", f,f/10e6M );
            }
            Console.WriteLine("Simulation Ends");
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
            return  1000000000m * unitTime_per_pulse;
            
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
            return  GetClock() / 1e3m;
        }

        public decimal MilliSeconds()
        {
            return GetClock() / 1e6m;
        }

        public decimal Seconds()
        {
            return GetClock() / 1e9m;
        }

        public bool Tick()
        {
            try
            {
                lock (this)
                {
                    _clock++;
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
