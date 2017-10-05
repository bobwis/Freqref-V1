using System;
using System.Runtime.Remoting.Messaging;

namespace Simulation
{
    public class PIDController
    {
        private ulong lastTick = 0;
        private ulong ocxointerval = 2048;


        /* problem with discrete error count in the current system is it doesn't account for the time it took to get the error 
         *  
         *    get instantaneous error, then change dt
         * 
         * 
         * What if the error calc was dependent on dt
         * 
         *    err = (ocxocount / dt) - (gpscount /dt)
         *   
         * This simple change means the error is smaller if the same error happens over greater time, which is what we're looking for
         * 
         * Small sample rates (low dt) means faster checking, larger error ratios, faster response but still being able to cope with long term drift
         * 
         *  
         *  
         */
        internal long GetValue(double gpscount, double ocxcount, ulong tick, long currentVal)
        {
            //time since last count 
            if (tick == 0) return currentVal;
            var timediff = tick - lastTick;

            if (ocxointerval > timediff)
            {
                return -1;
                
            }

                lastTick = tick;
            var err = (gpscount / timediff )- (ocxcount/ timediff );

            // err is now proportional to the error over time  (doesn't matter what the timediff units are)
            var magerr =((err > 0) ? err : -err);

            if (magerr > 0.1)
            {
                ocxointerval = (ocxointerval > 4096) ? ocxointerval >> 1 : 2048;    // reduce time by half
            }
            else
            if (magerr <= 0.001)
            {
                ocxointerval = (ocxointerval <= 256000L) ? (ocxointerval << 1) : 420000;        // add 100% more time
            }
            if (err==0) return -1;
            currentVal = (uint)(currentVal + (currentVal * err));

            if (currentVal > 0xfff)
                currentVal = 0xfff;
            return Math.Max(currentVal, 1);
        }

        // return a dacValue 
        internal long GetValuex(double gpscount, double ocxcount, ulong tick, long currentVal)
        {
            //time since last count 
          
            if (tick == 0) return currentVal;
            
            var err = (long)gpscount - (long)ocxcount;

            ulong timediff = tick - lastTick;
            lastTick = tick;
            if (timediff <= ocxointerval) return -1;
            ulong magerr = (ulong)((err > 0) ? err : -err);

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