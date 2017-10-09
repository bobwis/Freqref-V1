/*
 * genpid.c
 *
 * Created: 9/10/2017 11:46:49 AM
 *  Author: paul
 */ 

 #include <stdio.h>

 float scalevalue(float value, float valuemin, float valuemax, float scalemin, float scalemax )
 {
     float vPerc = (value - valuemin) / (valuemax - valuemin);
     float bigSpan = vPerc * (scalemax - scalemin);
     return scalemin + bigSpan;
 }

 // just a useful func
 float clamp ( float value, float min, float max)  
 {
	if( value > max) return max;
	return value < min ? min :value;
 }


 // core values, normally set by the user of module
 static float pvMin = 20e6-1e3;
 static float pvMax = 20e6 +1e3;    // this will need to change, this is the max possible count (ceiling of the time units in 2 seconds)

 static float outMin = 0;
 static float outMax = 4096;

 static float kp= 1;
 static float ki= 1;
 static float kd= 1;

 static float integral;
 static float derivative;
 static float lasterr;

 float compute(uint64_t interval, float pv, float sp)
 {
	float err;
	pv = clamp(pv,pvMin,pvMax);
	pv = scalevalue(pv, pvMin, pvMax, -1.0, 1.0);

	//scale the setpoint
	sp = clamp(sp, pvMin, pvMax);
	sp = scalevalue(sp, pvMin, pvMax, -1.0, 1.0);
	err = sp - pv;
		    
	integral = integral + (err  *interval  );
	derivative = (err - lasterr)  /interval;
	float output = (kp * err) + (ki* integral) + (kd * derivative);
	lasterr = err;
	float outReal = clamp(output, -1.0, 1.0);
	outReal = scalevalue(outReal, -1.0, 1.0, outMin, outMax);
	return outReal;
 }