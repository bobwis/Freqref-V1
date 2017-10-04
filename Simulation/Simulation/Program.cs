using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Simulation
{
    class Program
    {
        static void Main(string[] args)
        {
            OCXO myOCXO = new OCXO();
            GPSSource myGPS = new GPSSource();
            PIDController myPID = new PIDController();
            Int64 tick = 0;
            while (true)
            {
                tick=tick+ 10000000;
                var gpscount = myGPS.GetCount(10000000);
                var ocxcount = myOCXO.GetCount(10000000);

                Console.WriteLine($" GPS count = {gpscount}");
                Console.WriteLine($" OCXO count = {ocxcount}" );

                //var tweak = int.Parse(Console.ReadLine());

                long tweak = myPID.GetValue(gpscount,ocxcount, tick, myOCXO.GetDACVAl());
                Console.WriteLine($" Tweak DACVAL = = {tweak}");
                
                myOCXO.TweakInput(tweak);

            }


        }
    }

  
    public interface IFreqSource
    {
        void TweakInput(long val);
           

    }

    public class OCXO :BaseSource , IFreqSource
    {
        double _maxFreq = 1.0000005;
        double _minFreq =  0.9999995;

        long _mindacVal = 0;   //mv
        long _maxdavVal = 4095;
        long _currentdacVal =1000;

        public OCXO()
        {

            CurrentFreq = _minFreq;
        }

        public void TweakInput(long dacVal)
        {
            var roughtweak = ((double)dacVal * (double)0.00000000008);

            // what should the freq change be?
            if (dacVal > _currentdacVal)
            {
                CurrentFreq += roughtweak;
            }
            else
            {
                CurrentFreq -= roughtweak;
            }
            _currentdacVal = dacVal;
        }

        internal long GetDACVAl()
        {
            return _currentdacVal;
        }
    }

    public class BaseSource
    {
        protected double CurrentFreq;

        protected int jitter;

        public virtual double GetCount(double  dt)
        {
            return (double) CurrentFreq * dt;
        }
    }



    public class GPSSource : BaseSource, IFreqSource
    {

        public GPSSource()
        {
            CurrentFreq = 1.0000000;
        }    

        public override double  GetCount(double dt)
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
