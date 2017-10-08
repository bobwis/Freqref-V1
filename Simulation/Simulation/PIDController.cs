using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace Simulation
{
    public struct DataPoint
    {
        public ulong tick;
        public decimal data;

        public DataPoint(decimal d, ulong t)
        {
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

        public void AddLog(decimal val, ulong tick)
        {
            lock(Locker)
            {
                pointlog.Add(new DataPoint(val,tick));
            }
        }


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
            Console.Write($" err={err} magerr={magerr} ocxointerval={ocxointerval} dacval={dacval} ");

            OCXO.SetDAC(dacval);

            // SetDac sets absolute value, which results in absolute frequency, which isn't realistic
            return dacval;
        }

        private ulong oldt;

        private decimal olderr = decimal.MaxValue;

        GenericPID genPID = new GenericPID();

        public PIDController()
        {
            // set up the pid's limits
            genPID.pvMin = 20e6m-1e5m;   // min for a 2 sec count
            genPID.pvMax = 20e6m +1e5m;

            genPID.outMax = 4096;
            genPID.outMin = 0;

            genPID.kp = 1.0m;
            genPID.ki = 1.0m;
            genPID.kd = 1.0m;

        }

        public void Tick(ulong t)
        {
            if (OCXO == null || GPS == null)
            {
                // we haven't been set up yet, do nothing
                return;
            }
            ulong dt = t - oldt;
            // a unit of time has passed of size dt (10nS)
            if (dt <= ocxointerval * 100000m  /*100 to bring it to 10ns*/ ) return;

            gpscount = GPS.GetCount(dt);
            ocxocount = OCXO.GetCount(dt);

#if false


            // fix the dt to 2 seconds for the generic PID
            var ratio = (2*World.CLOCK_RATE)/dt;
            var res = genPID.Compute((ulong)(2 * World.CLOCK_RATE), ocxocount * ratio, 2 * 10e6m);
            Console.WriteLine("PID RESULT = {0}", res);

            OCXO.SetDAC((long)res);
#else


            decimal err = (decimal)gpscount - ocxocount;

            if (Math.Abs(err) > Math.Abs(olderr))
            {
                Debugger.Launch();
            }
            
           

            //TODO: move these to different output
            Console.WriteLine($"ocxocount={ocxocount} gps={gpscount}");

            
            Process(err, dt);


            AddLog(OCXO.GetDAC(), t);
            oldt = t;
            olderr = err;
#endif
        }

        public ulong ocxocount { get; set; }

        public ulong gpscount { get; set; }
    }
}