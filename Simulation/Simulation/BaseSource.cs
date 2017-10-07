using System;

namespace Simulation
{
    public class BaseSource :Lockable
    {
        public bool AddJitter = false;
        protected decimal CurrentFreq;

        protected int Jitter;

        // dt is now in 10ps
        public virtual ulong GetCount(decimal  dt)
        {
            var r= new Random();
            return (ulong) ((CurrentFreq / 1e8m) * dt);// + (ulong) (r.Next(2));
        }
        
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