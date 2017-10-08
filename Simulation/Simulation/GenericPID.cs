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

        
        public decimal kp, ki, kd, lasterr, pvMin, pvMax,outMax, outMin,errSum;
        public ulong lasttick;
        public decimal Compute(ulong dt, decimal pv, decimal sp)
        {
            //scale the pv to +/- 100%
            pv = Clamp(pv, pvMin, pvMax);
            pv = ScaleValue(pv, pvMin, pvMax, -1.0m, 1.0m);

            //scale the setpoint
            sp = Clamp(sp, pvMin, pvMax);
            sp = ScaleValue(sp, pvMin, pvMax, -1.0m, 1.0m);

            //error is in percent...
            var err = sp - pv;

            var pTerm = err * kp;
            var iTerm = 0.0m;
            var dTerm = 0.0m;

            var partialSum = 0.0m;
          
            //compute the integral  
            if (pv >= -1 && pv <= 1)
            {
                partialSum = errSum + dt * err;
                iTerm = ki * partialSum;
            }

            if (dt != 0) dTerm = kd * (err - lasterr) / (decimal) dt;
            
            errSum = partialSum;
            lasterr= err;

            //Now we have to scale the output value to match the requested scale
            var outReal = pTerm + iTerm + dTerm;
            outReal = Clamp(outReal, -1.0m, 1.0m);
            outReal = ScaleValue(outReal, -1.0m, 1.0m, outMin, outMax);

            return outReal;
        }

    }
}