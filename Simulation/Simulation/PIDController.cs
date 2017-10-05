using System;
using System.Runtime.Remoting.Messaging;

namespace Simulation
{
    public class PIDController
    {
        private ulong ocxointerval = 2000; /* 2048;*/


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
#if false
        internal long GetValuex(double gpscount, double ocxcount, ulong tick, long currentVal)
        {
            //time since last count 
            if (tick == 0) return currentVal;
            if (ocxointerval > tick)
            {
                return -1;

            }

            var err = (gpscount / tick) - (ocxcount/ tick);

            // err is now proportional to the error over time  (doesn't matter what the timediff units are)
            var magerr =((err > 0) ? err : -err);

            if (magerr > 0.3)
            {
                ocxointerval = (ocxointerval > 4096) ? ocxointerval >> 1 : 2048;    // reduce time by half
            }
            else
            if (magerr <= 0.3)
            {
                ocxointerval = (ocxointerval <= 256000L) ? (ocxointerval << 1) : 420000;        // add 100% more time
            }
            if (err==0) return -1;
            currentVal = (uint)(currentVal + (currentVal * err));

            if (currentVal > 0xfff)
                currentVal = 0xfff;
            return Math.Max(currentVal, 1);
        }
#else
        // return a dacValue 
        internal long Process(OCXO ocxo, double gpscount, double ocxcount, double dT)
        {
            // if we're here too soon and don't want to do anything, just return
            if (dT <= ocxointerval) return -1;

            // TODO: This is the nugget, is this quantisation useful enough to get accuracy
            double err = (gpscount - ocxcount);    

            ulong magerr = (ulong)((err > 0) ? err : -err);

            if (magerr > 3)
            {
                ocxointerval = (ocxointerval > 4096) ? ocxointerval >> 1 : 2048;    // reduce ms time by half
            }
            else
            if (magerr <= 2)
            {
                ocxointerval = (ocxointerval <= 256000L) ? (ocxointerval << 1) : 4200000;    // add 100% more time
            }

            //get the existing value
            var dacval = (uint)ocxo.GetDAC();
            

            // we need to do something to err to bring it in proportion to dacval. Two different entities, if we want to model it properly
            // In PID the err is the 'Correct' dacval (SP) - the 'current' dacval (PV), and the simple PID formula should give us the change in dacval
            
            // so we need a function which, given our count difference, returns an offset dacval
            
            dacval = (uint) (dacval + CountErrToOffset(err));


            if (dacval > 0xfff)
                dacval = 0xfff;


            // SetDac sets absolute value, which results in absolute frequency, which isn't realistic
            ocxo.SetDAC(dacval);
            return dacval;
        }

        int CountErrToOffset(double counterror)
        {
            // pasted existing
            return (int) (counterror < 0 ? -counterror : counterror);

        }

#endif
    }
}