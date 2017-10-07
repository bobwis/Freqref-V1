using System;

namespace Simulation
{
    public class WorldClock :Lockable
    {
        private ulong _clock;

        public ulong GetClock()
        {
            lock (Locker)
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
                lock (Locker)
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