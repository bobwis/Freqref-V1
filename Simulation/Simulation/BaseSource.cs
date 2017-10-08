using System;

namespace Simulation
{
    public class BaseSource :Lockable
    {
        public bool AddJitter = false;
        protected decimal CurrentFreq;

        // dt is now in 10ns
        public virtual ulong GetCount(decimal  dt)
        {
            var r= new Random();
            var count = (ulong) ((CurrentFreq / 1e8m) * dt);
            if (false)
            {
                count += (ulong) (r.Next(100)>95?1:0);
            }
            return count;
        }

        // scenario.  Frequency set to 
        // 10000000.0036582537578125

        // dt is 256000100000 ns
        // so number of ticks is 
        // 2560001000.93651332782537578125
        // therefore rounded down to units
        // 2560001000

        // NOTE // a fundamental issue that we cannot distinguish between the difference of 1 dacval at until period over 256 seconds 
        // so all algorithms get longer and longer, looking for the +1, but then the error offset is so small, it doesn't equate back to 1 dacval
        // so it loops around again, never possible to settle
        // if I decrease the tolerance, I can get it to not care so, so the artificial linear simulation is stable, the real world wouldn't have this problem since the noise would compensate

        public void FrequencyDrift()
        {
            var r = new Random();
            if (r.Next(3) == 2)
                CurrentFreq = CurrentFreq + (1 / CurrentFreq);
            else if (r.Next(3) == 1)
            {
                CurrentFreq = CurrentFreq - (1 / CurrentFreq);
            }
        }

        public decimal Current => CurrentFreq;





    }
}