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
                myOCXO.Tick();
                tick=tick+100;
                //var tweak = int.Parse(Console.ReadLine());
                responsecount = responsecount + 100;
                var gpscount = myGPS.GetCount(responsecount);
                var ocxcount = myOCXO.GetCount(responsecount);
                long tweak = myPID.GetValue(gpscount,ocxcount, tick, myOCXO.GetDACVAl());
                if (tweak != -1)
                {
                    responsecount = 0;
                    Console.Write($"Tick {tick} GPS count = {gpscount} OCXO count = {ocxcount}");
                    Console.Write($" Tweak DACVAL = {tweak}                               \r");
                    myOCXO.TweakInput(tweak);
                }


            }
        }
    }

  
    public interface IFreqSource
    {
        void TweakInput(long val);
           

    }
}
