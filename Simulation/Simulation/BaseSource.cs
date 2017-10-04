namespace Simulation
{
    public class BaseSource
    {
        protected double CurrentFreq;

        protected int jitter;

        public virtual double GetCount(double  dt)
        {
            return (double) CurrentFreq * dt;
        }
    }
}