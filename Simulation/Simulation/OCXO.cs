using System;

namespace Simulation
{
    public class OCXO :BaseSource
    {
        private readonly decimal riseRate =  0.001m;   // very fast, practically instantaneous

        public void Tick(ulong dT)
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

        public decimal Target => TargetFrequency;
        public GPSSource GPS { get; set; }
        public WorldClock WorldClock { get; set; }

        decimal _maxFreq = 10e6m+5m;
        decimal _minFreq = 10e6m-5m;

        long _mindacVal = 1;   //mv
        long _maxdacVal = 4095;
        long _currentdacVal =1000;

        private decimal TargetFrequency;
        public OCXO()
        {

            CurrentFreq = _minFreq;
            TargetFrequency = _minFreq;
        }
        // not time dependent, no lag on change of voltage
        // Sets the target frequency, where we'll be moving to, but not instantaneous
        public void SetDAC(long dacVal)
        {
            TargetFrequency = (_minFreq) + (dacVal * 10m / 4096m);     // 10Hz over the range
            _currentdacVal = dacVal;
        }

        public long GetDAC()
        {
            return _currentdacVal;
        }
    }
}