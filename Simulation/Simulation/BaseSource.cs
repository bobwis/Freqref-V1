using System;

namespace Simulation
{
    public class BaseSource :Lockable
    {
        public bool AddJitter = false;
        protected decimal CurrentFreq;

        // dt is now in 10ns
        public virtual ulong GetCountX(decimal  dt)
        {
            var r= new Random();



            var count = (ulong) ((CurrentFreq / 1e8m) * dt);
            if (AddJitter)
            {
                count += (ulong) (r.Next(100)>95?1:0);
            }
            return count;
        }
        public virtual ulong GetCount(decimal dt)
        {
            var r = new Random();
            return (ulong)((CurrentFreq / 1e8m) * dt);// + (ulong) (r.Next(2));
             
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