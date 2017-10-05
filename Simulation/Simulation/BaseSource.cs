namespace Simulation
{
    public class BaseSource
    {
        protected double CurrentFreq;

        protected int jitter;

        public virtual long GetCount(double  dt)
        {
          // having a moment...
            // counts in 1 sec = CurrentFreq

            // how much time is dt

            // (dt/SIMULATION_CLOCK_MS) * 1000 = number of seconds
            return (long) (CurrentFreq * (dt/Program.SIMULATION_CLOCK_MS)) ;
        }
    }
}