using System;

namespace Simulation
{
    public class GPSSource : BaseSource
    {
        public GPSSource()
        {
            CurrentFreq = 1.0000000;
        }

        public override long GetCount(double dt)
        {
            var r = new Random();
            return base.GetCount(dt) + r.Next(2);
        }

        public void TweakInput(long val)
        {
            throw new NotImplementedException();
        }
    }
}