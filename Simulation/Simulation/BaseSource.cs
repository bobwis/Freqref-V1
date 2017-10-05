using System;

namespace Simulation
{
    public class BaseSource
    {
        protected bool _addJitter = false;
        protected double CurrentFreq;

        protected int jitter;

        public virtual long GetCount(double  dt)
        {
            var r = new Random();
            return (long)((long)(CurrentFreq / 1000) * dt) + r.Next(2);
        }
    }
}