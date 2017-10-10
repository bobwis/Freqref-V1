/*
 * genpid.c
 *
 * Created: 9/10/2017 11:46:49 AM
 *  Author: paul
 */ 

 #include <stdio.h>
 #include "genpid.h"

 float scalevalue(float value, float valuemin, float valuemax, float scalemin, float scalemax )
 {
 	//printf("SCALEVALUE %f, %f , %f\n\r", value, valuemin, valuemax);
     float vPerc = (value - valuemin) / (valuemax - valuemin);
     float bigSpan = vPerc * (scalemax - scalemin);
     return scalemin + bigSpan;
 }

 // just a useful func
 float clamp ( float value, float min, float max)  
 {
	//printf("CLAMP %f, %f , %f\n\r", value, min, max);
	if( value > max) return max;
	return value < min ? min :value;
 }


 // core values, normally set by the user of module
 static float pvMin = 20e6-1e3f;
 static float pvMax = 20e6 +1e3f;    // this will need to change, this is the max possible count (ceiling of the time units in 2 seconds)

 static float outMin = 0.0f;
 static float outMax = 4096.0f;

 static float kp= 1.0f;
 static float ki= 1.0f;
 static float kd= 1.0f;

 static float integral = 0.0f;
 static float derivative = 0.0f;
 float lasterr = 0.0;

 static uint64_t baseunit =1000;           // units in a second (in the sim thats the CLOCK), here if the intervals are in ms, it's 1000. 

 // sp and pv are the raw counts, in that interval, no need to touch them outside of this method
 float compute(uint64_t interval, float apv, float asp)
 {
 float pv = apv;
 float sp = asp;

	float ratio = (2.0f * baseunit) / interval;	
	// is the interval in ms? This needs to be lined up with what pvMin/pvMax is.
	
	
	sp *= ratio;
	pv *= ratio;
	// everything below is bog standard PID, nothing to tweak
	float err;
	pv = clamp(pv,pvMin,pvMax);
	pv = scalevalue(pv, pvMin, pvMax, -1.0f, 1.0f);
	sp = clamp(sp, pvMin, pvMax);
	sp = scalevalue(sp, pvMin, pvMax, -1.0f, 1.0f);
	
	err = sp - pv;	    
	integral = integral + (err  *interval  );
	derivative = (err - lasterr)  /interval;
	float output = (kp * err) + (ki* integral) + (kd * derivative);
	
	
	
	lasterr = err;

	float outReal = clamp(output, -1.0f, 1.0f);
	
	outReal = scalevalue(outReal, -1.0f, 1.0f, outMin, outMax);
	printf("STARTING -%f, %f , %f, %f, %f, %f, %f\n\r", pv,sp,ratio, integral, derivative,output,outReal);
	return outReal;
 }