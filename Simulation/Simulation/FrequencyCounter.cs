using System;
using System.Diagnostics;

namespace Simulation
{
    public class FrequencyCounter :Lockable
    {
        private decimal _pulseCount;
        private decimal _endTick;
        private decimal _startTick;

        public decimal StartTick
        {
            get
            {
                lock (Locker)
                {
                    return _startTick;
                }
            }
            set
            {
                lock (Locker)
                {

                    _startTick = value;

                }

            }
        }

        public decimal EndTick
        {
            get
            {
                lock (Locker)
                {
                    return _endTick;
                }
            }
            set
            {
                lock (Locker)
                {
                    _endTick = value;
                }
            }
        }

        public decimal PulseCount
        {
            get
            {
                lock (Locker)
                {
                    return _pulseCount;
                }
            }
            set
            {
                lock (Locker)
                {
                    _pulseCount = value;
                }
            }
        }

        public decimal GetFrequencyInHertz()
        {
            decimal freq = 0M;
            lock (Locker)   //makes sure the variables used cannot be altered during the calculation (which was happening alot!)
            {
                Debug.Assert(EndTick==PulseCount);
                //picoseconds per pulse
                decimal unitTimePerPulse = Decimal.Divide(EndTick, PulseCount);
                freq = World.CLOCK_RATE * unitTimePerPulse;
                Debug.Assert(freq == 100000000M, $"PC {PulseCount}  ET {EndTick} Rate {unitTimePerPulse}");

            }
            return freq;
        }
    }
}