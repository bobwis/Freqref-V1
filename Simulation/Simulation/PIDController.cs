using System.Threading;

namespace Simulation
{
    public class PIDController
    {
        private long lastTick = 0;
        private int ocxointerval = 204800;


        // return a dacValue 
        internal long GetValue(double gpscount, double ocxcount, long tick, long currentVal)
        {
            //time since last count 
          
            if (tick == 0) return currentVal;
            
            var err = (long)gpscount - (long)ocxcount;

            var timediff = tick - lastTick;
            lastTick = tick;
            if (timediff <= ocxointerval) return -1;
            var magerr = (int)((err > 0) ? err : -err);

            if (magerr > 3)
            {
                ocxointerval = (ocxointerval > 409600) ? ocxointerval >> 1 : 204800;    // reduce time by half
            }
            else
            if (magerr <= 2)
            {
                ocxointerval = (ocxointerval <= 25600000L) ? (ocxointerval << 1) : 42000000;        // add 100% more time
            }

            var scale = (420 - ((ocxointerval * 4) / 1000)) / 8;
            if (scale < 1)
            {
                scale = 1;
            }
            magerr = magerr * scale;        // scale

            if (magerr > 4000)
            {
                magerr = 1000;      // limit dac step size
            }
            if (err == 0) return -1;
            var dacval = (uint)currentVal;
            dacval = err < 0 ? (uint) (dacval - magerr) : (uint) (dacval + magerr);

            if (dacval > 0xfff)
                dacval = 0xfff;
            return dacval;
        }
    }

}