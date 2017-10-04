using System;

namespace Simulation
{
    public class OCXO :BaseSource , IFreqSource
    {
        private double riseRate = 0.00001;

        public void Tick()
        {
            //move incrementally towards TargetFrequency
            if (CurrentFreq != TargetFrequency) // will give chance for drift
            {
                if (CurrentFreq > TargetFrequency)
                    CurrentFreq -= riseRate;
                else
                    CurrentFreq += riseRate;
            }
                

        }
        double _maxFreq = 10.0000005;
        double _minFreq =  0.8999995;

        long _mindacVal = 0;   //mv
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
        public void TweakInput(long dacVal)
        {

            dacVal = Math.Min(_maxdavVal, Math.Max(dacVal, 0));
            var roughtweak = ((double)dacVal * (double)0.0000000008);

            // what should the freq change be?
            if (dacVal > _currentdacVal)
            {
                TargetFrequency += roughtweak;
                TargetFrequency = Math.Min(TargetFrequency, _maxFreq);
            }
            else if(dacVal > _currentdacVal)
            {
                TargetFrequency -= roughtweak;
                TargetFrequency = Math.Max(TargetFrequency, _minFreq);
            }

            _currentdacVal = Math.Min(_maxdavVal,Math.Max(dacVal,0)) ;

        }

        internal long GetDACVAl()
        {
            return _currentdacVal;
        }
    }
}