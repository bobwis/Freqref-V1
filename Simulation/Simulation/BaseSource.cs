using System;

namespace Simulation
{
    public class BaseSource
    {
        protected bool AddJitter = false;
        protected double CurrentFreq;

        protected int Jitter;

        public virtual double GetCount(double  dt)
        {
            var r = new Random();
            return ((CurrentFreq / 1000.0000) * dt) + r.Next(2);
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






    }
}