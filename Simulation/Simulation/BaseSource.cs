namespace Simulation
{
    public class BaseSource
    {
        protected double CurrentFreq;

        protected int jitter;

        public virtual long GetCount(double  dt)
        {
            return (long) (CurrentFreq * dt);
        }
    }
}