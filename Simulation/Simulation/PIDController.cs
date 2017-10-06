using System;
using System.Data.OleDb;

namespace Simulation
{
    public class PIDController : Lockable
    {
        // other real world devices we know about
        public OCXO OCXO { get; set; }
        public GPSSource GPS { get; set; }

        private ulong ocxointerval = 2000; /* 2048;*/
        long scale;
        
        internal long Process(decimal err, decimal dT)
        {
            var magerr = ((err > 0) ? err : -err);

            if (magerr > 3)
            {
                ocxointerval = (ocxointerval > 4096) ? ocxointerval >> 1 : 2048;    // reduce ms time by half
            }
            else
            if (magerr <= 2)
            {
                ocxointerval = (ocxointerval <= 256000L) ? (ocxointerval << 1) : 4200000;    // add 100% more time
            }

            scale = (int)((420L - ((ocxointerval * 4L) / 1000L)) / 8);
            if (scale < 1)
            {
                scale = 1;
            }

            magerr = magerr * (ulong)Math.Abs(scale);        // scale

            if (magerr > 4000)
            {
                magerr = 1000;      // limit dac step size
            }

            //get the existing value
            var dacval = (uint)OCXO.GetDAC();

            if (err < 0)
            {
                dacval = dacval - (uint)magerr;
            }
            else if (err > 0)
            {
                dacval = dacval + (uint)magerr;
            }
            
            if (dacval > 0xfff)
                dacval = 0xfff;
            //TODO: move these to different output
            Console.Write($" err={err} magerr={magerr} ocxointerval={ocxointerval} dacval={dacval} ");

            OCXO.SetDAC(dacval);

            // SetDac sets absolute value, which results in absolute frequency, which isn't realistic
            return dacval;
        }

        private ulong oldt;


        public void Tick(ulong t)
        {
            if (OCXO == null || GPS == null)
            {
                // we haven't been set up yet, do nothing
                return;
            }
            ulong dt = t - oldt;
            // a unit of time has passed of size dt (10pS)

            // ported from previous solution (kept ocxointerval at ms, but not required to be)
//            if (dt <= ocxointerval *100 /*100 to bring it to 10ps*/ ) return;
            if (dt <= ocxointerval * 100000 /*100 to bring it to 10ps*/ ) return;

            var gpscount = GPS.GetCount(dt);
            var ocxocount = OCXO.GetCount(dt);
            decimal err = gpscount - ocxocount;

         //TODO: move these to different output
         Console.WriteLine($"ocxocount={ocxocount} gps={gpscount}");

            Process(err, dt);

            oldt = t;
        }
    }
}