using System;

namespace Simulation
{
    public class OCXO :BaseSource, IWorldObject
    {
        private readonly decimal riseRate = 10m;// 0.001m;   // very fast, practically instantaneous

        public void Tick(ulong dT)
        {
            CurrentFreq = TargetFrequency;
            return;
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

        decimal _maxFreq = (10e6m - 0.964001048m) + 3.74625m; /* 3.615m; */
        decimal _minFreq = (10e6m - 0.964001048m) - 3.74625m; /* 3.8775m; */

        long _mindacVal = 1;   //mv
        long _maxdacVal = 4095;
        long _currentdacVal = 1000;

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
            var responseStep = ((_maxFreq-_minFreq) / 4096m);
            TargetFrequency = (_minFreq) + (dacVal * responseStep)  - responseStep;     // 10Hz over the range
            _currentdacVal = dacVal;
        }

        public long GetDAC()
        {
            return _currentdacVal;
        }
    }
}