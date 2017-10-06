using System;

namespace Simulation
{
    public class WorldClock
    {
        private ulong _clock;

        public ulong GetClock()
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

        public bool Tick(ulong interval)
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