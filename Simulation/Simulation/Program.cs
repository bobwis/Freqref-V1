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
            var myGPS = new GPSSource();
            var myPID = new PIDController();
            Int64 tick = 0;
            Int64 responsecount= 0;
            while (true)
            {
<<<<<<< HEAD
                tick=tick+ 10000000;
                var gpscount = myGPS.GetCount(10000000);
                var ocxcount = myOCXO.GetCount(10000000);

                Console.Write($" GPS count = {gpscount}");
                Console.Write($" OCXO count = {ocxcount}" );

=======
                myOCXO.Tick();
                tick=tick+100;
>>>>>>> a0bc5d9407e2f7e55dc403ecf9941b21ffb5acb5
                //var tweak = int.Parse(Console.ReadLine());
                responsecount = responsecount + 100;
                var gpscount = myGPS.GetCount(responsecount);
                var ocxcount = myOCXO.GetCount(responsecount);
                long tweak = myPID.GetValue(gpscount,ocxcount, tick, myOCXO.GetDACVAl());
<<<<<<< HEAD
                Console.WriteLine($" Tweak DACVAL = = {tweak}");
                
                myOCXO.TweakInput(tweak);
=======
                if (tweak != -1)
                {
                    responsecount = 0;
                    Console.Write($"Tick {tick} GPS count = {gpscount} OCXO count = {ocxcount}");
                    Console.Write($" Tweak DACVAL = {tweak}                               \r");
                    myOCXO.TweakInput(tweak);
                }

>>>>>>> a0bc5d9407e2f7e55dc403ecf9941b21ffb5acb5

            }
        }
    }

  
    public interface IFreqSource
    {
        void TweakInput(long val);
    }
<<<<<<< HEAD

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
            var roughtweak = ((double)dacVal * (double)0.001829224/10e6);
            //   var roughtweak = ((double)dacVal * (double)10e-6 * 8);

            if (dacVal >= 2048)
            {
                CurrentFreq = 1.0 + roughtweak;
            }
            else
            {
                CurrentFreq = 1.0 - roughtweak;
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
=======
>>>>>>> a0bc5d9407e2f7e55dc403ecf9941b21ffb5acb5
}
