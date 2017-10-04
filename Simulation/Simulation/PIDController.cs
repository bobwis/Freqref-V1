using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Simulation
{
    public class PIDController
    {
        long lastTick = 0;
        int ocxointerval = 0;

        // return a dacValue 
        internal long GetValue(double gpscount, double ocxcount, long tick, long currentVal)
        {
            //time since last count 
            // tick - lastTick;
            if (tick == 0) return currentVal;

            // insert cool stuff here :
            //
            long err = (long)gpscount - (long)ocxcount;

            var timediff = tick - lastTick;
            lastTick = tick;
            if (timediff > ocxointerval)
            {
                var magerr = (int)((err > 0) ? err : -err);

                if (magerr > 3)
                {
                    ocxointerval = (ocxointerval > 4096) ? ocxointerval >> 1 : 2048;    // reduce time by half
                }
                else
                if (magerr <= 2)
                {
                    ocxointerval = (ocxointerval <= 256000L) ? (ocxointerval << 1) : 420000;        // add 100% more time
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

                var dacval = (uint)currentVal;

                if (err < 0)
                {
                    dacval = (uint) (dacval - magerr);
                }
                else if (err > 0)
                {
                    dacval = (uint) (dacval + magerr);
                }

                if (dacval > 0xfff)
                    dacval = 0xfff;
<<<<<<< HEAD
                Console.Write($" err={err}");
                Console.Write($" interval={ocxointerval}");
                Console.Write($" magerr={magerr}");

=======
>>>>>>> a0bc5d9407e2f7e55dc403ecf9941b21ffb5acb5
                return dacval;

            }
            return -1;

        }
    }

}
