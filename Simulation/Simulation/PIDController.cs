using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace Simulation
{
    public struct DataPoint
    {
        public ulong tick;
        public decimal data;
        public decimal currentError;

        public DataPoint(decimal err, decimal d, ulong t)
        {
            currentError = err;
            tick = t;
            data = d;
        }
    }

    public class PIDController : Lockable
    {
        // 40 point buffer
        private List<DataPoint> pointlog = new List<DataPoint>(100);

        public List<DataPoint> GetLog()
        {
            lock (Locker)
            {
                return pointlog;
            }
        }

        public void AddLog(DataPoint d)
        {
            lock(Locker)
            {
                pointlog.Add(d);
            }
        }


    // other real world devices we know about
        public OCXO OCXO { get; set; }
        public GPSSource GPS { get; set; }

        private ulong ocxointerval = 2500; /* 2048;*/
        long scale;
        
       /* internal long Process(decimal err, decimal dT)
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
                magerr = 4000;      // limit dac step size
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
            Console.Write($" err={err} magerr={magerr} ocxointerval={ocxointerval}");

            OCXO.SetDAC(dacval);

            // SetDac sets absolute value, which results in absolute frequency, which isn't realistic
            return dacval;
        }
        */
        private ulong oldt;

        private decimal olderr = decimal.MaxValue;

        readonly GenericPID genPID = new GenericPID();

        public PIDController()
        {
            // set up the pid's limits
            genPID.pvMin = 20e6m-1e3m;   // min for a 2 sec count
            genPID.pvMax = 20e6m +1e3m;

            genPID.outMax = 4096;
            genPID.outMin = 0;

            genPID.kp = 50;   // rapid ramp
            genPID.ki = 5m;   // great fun experimenting, > 1 and it will overshoot but dampen
            genPID.kd = 1m;  // gentle deceleration

        }


        // this is an interesting concept where we denote how close is close enough
        private decimal TOLERANCE = 1e-5m;

        public void Tick(ulong t)
        {
            if (OCXO == null || GPS == null) return;
            var dt = t - oldt;
            // a unit of time has passed of size dt (10nS)
            if (dt <= (ocxointerval * 100000m ) /*100 to bring it to 10ns*/ ) return;

            gpscount = GPS.GetCount(dt);
            ocxocount = OCXO.GetCount(dt);

#if true

            // this is the PID call, pass the two values, the interval and what to do with the result, nothing inside genPID knows anything about anything
            var err = genPID.Compute(dt, ocxocount , gpscount , OCXO.SetDAC);

            if(!World.ShowStatusScreen) Console.WriteLine($"dt = {dt} Interval = {ocxointerval}  ocxocount={ocxocount} gps={gpscount}  dacval={OCXO.GetDAC()} Err {err}" );
            if (Math.Abs(err) < TOLERANCE)
            {
                ocxointerval += ocxointerval*2 ;
            }
            else
            {
                ocxointerval = 2000;
            }
            oldt = t;



#else


            decimal err = (decimal)gpscount - ocxocount;
            Console.WriteLine($"ocxocount={ocxocount} gps={gpscount}  dacval={OCXO.GetDAC()}");
            Process(err, dt);
            
            oldt = t;
            olderr = err;
#endif
            AddLog(new DataPoint(err,OCXO.GetDAC(), t));
        }

        public ulong ocxocount { get; set; }

        public ulong gpscount { get; set; }
        public ulong Interval => ocxointerval;
    }
}