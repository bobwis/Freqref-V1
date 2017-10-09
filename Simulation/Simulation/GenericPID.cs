using System;

namespace Simulation
{
    public class GenericPID :Lockable
    {
        private decimal ScaleValue(decimal value, decimal valuemin, decimal valuemax, decimal scalemin, decimal scalemax)
        {
            var vPerc = (value - valuemin) / (valuemax - valuemin);
            var bigSpan = vPerc * (scalemax - scalemin);
            return scalemin + bigSpan;
        }

        private decimal Clamp(decimal value, decimal min, decimal max)
        {
            if (value > max) return max;
            return value < min ? min : value;
        }

        public decimal integral,derivative;
        public decimal kp, ki, kd, lasterr, pvMin, pvMax,outMax, outMin,errSum;
        public ulong lasttick;
        public decimal Compute(ulong interval, decimal pv, decimal sp, Action<long> setDAC)
        {
            // locked to 2sec
            var ratio = (2 * World.CLOCK_RATE) / interval;

         
            sp *= ratio;
            pv *= ratio;

            //scale the pv to +/- 100%
            pv = Clamp(pv, pvMin, pvMax);
            pv = ScaleValue(pv, pvMin, pvMax, -1.0m, 1.0m);

            //scale the setpoint
            sp = Clamp(sp, pvMin, pvMax);
            sp = ScaleValue(sp, pvMin, pvMax, -1.0m, 1.0m);

            //error is in percent...
            var err = sp - pv;
            
            integral = integral + (err /* *dt */ );

            // determine the amount of change from the last time checked
            derivative = (err - lasterr) /* /dt */;
            // calculate how much to drive the output in order to get to the 
            // desired setpoint. 
            var output = (kp * err) + (ki* integral) + (kd * derivative);
            // remember the error for the next time around.
            lasterr = err;

            //Now we have to scale the output value to match the requested scale
            var outReal = Clamp(output, -1.0m, 1.0m);
            outReal = ScaleValue(outReal, -1.0m, 1.0m, outMin, outMax);


            //outReal = 2576;
            setDAC((long)outReal);
            return err;
        }

    }
}