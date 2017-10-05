using System;

namespace Simulation
{
    public class GPSSource : BaseSource
    {
        public GPSSource()
        {
            CurrentFreq = 10e6;
            _addJitter = true;
        }

    }
}