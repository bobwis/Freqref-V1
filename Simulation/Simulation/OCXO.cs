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
            var roughtweak = ((double)dacVal * (double)0.001829224 / 10e6);
            //   var roughtweak = ((double)dacVal * (double)10e-6 * 8);

            if (dacVal >= 2048)
            {
                TargetFrequency= 1.0 + roughtweak;
            }
            else
            {
                TargetFrequency = 1.0 - roughtweak;
            }

            _currentdacVal = dacVal;
        }



        internal long GetDACVAl()
        {
            return _currentdacVal;
        }
    }
}