using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data.OleDb;
using System.Diagnostics;
using System.Dynamic;
using System.Security.Policy;

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

        private ulong ocxointerval = 1000; /* 2048;*/
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

        private decimal olderr = decimal.MaxValue;
        public void Tick(ulong t)
        {
            if (OCXO == null || GPS == null)
            {
                // we haven't been set up yet, do nothing
                return;
            }
            ulong dt = t - oldt;
            // a unit of time has passed of size dt (10pS)
            if (dt <= ocxointerval * 100000m  /*100 to bring it to 10ps*/ ) return;

            gpscount = GPS.GetCount(dt);
            ocxocount = OCXO.GetCount(dt);

            
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
        }

        public ulong ocxocount { get; set; }

        public ulong gpscount { get; set; }
    }
}