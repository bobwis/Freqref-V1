using System;

namespace Simulation
{
    public class OCXO :BaseSource
    {
        private readonly double riseRate = 1;   // very fast, practically instantaneous

        public void Tick()
        {
            //move incrementally towards TargetFrequency
            if (CurrentFreq != TargetFrequency) // will give chance for drift
            {
                if (Math.Abs(CurrentFreq - TargetFrequency) < riseRate)
                { CurrentFreq = TargetFrequency; }
                else
                {
                    if (CurrentFreq > TargetFrequency)
                        CurrentFreq -= riseRate;
                    else
                        CurrentFreq += riseRate;
                }
            }
        }

        public double Target => TargetFrequency;
        public double Current => CurrentFreq;

        double _maxFreq = 10e6+5;
        double _minFreq = 10e6-5;

        long _mindacVal = 1;   //mv
        long _maxdavVal = 4095;
        long _currentdacVal =1000;

        private double TargetFrequency;
        public OCXO()
        {

            CurrentFreq = _minFreq;
            TargetFrequency = _minFreq;
        }
        // not time dependent, no lag on change of voltage
        // Sets the target frequency, where we'll be moving to, but not instantaneous
        public void SetDAC(long dacVal)
        {
            var roughtweak = ((double)dacVal * (double)0.00001829224);
            //   var roughtweak = ((double)dacVal * (double)10e-6 * 8);
#if false
            // adjust target / woolly
            if (dacVal >= 2048)
            {
                TargetFrequency= 10e6 + roughtweak;
            }
            else
            {
                TargetFrequency = 10e6 - roughtweak;
            }
#else
            // assume dacVal is an absolute 0..4095 , not an offset?

            TargetFrequency = (10e6 - 5) + ((double)dacVal * (double)10 / 4096);     // 10Hz over the range
#endif
            _currentdacVal = dacVal;
        }



        internal long GetDAC()
        {
            return _currentdacVal;
        }
    }
}