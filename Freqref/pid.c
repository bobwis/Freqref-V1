/*
 * PID.c
 *
 * Created: 3/10/2017 6:06:56 PM
 *  Author: paul
 */ 
 #include <pid.h>
 #include <stdio.h>

 #define history_count   100
 static unsigned int current_buffer_pos = 0;
 static  int error_history[history_count];

 // store/forward proportional error values  
  void storeError(int error)
 {
	 error_history[current_buffer_pos] = error;
	 current_buffer_pos = (++current_buffer_pos )% history_count;
 }

 int sum_error()
 {
	 int pos = 0, sum = 0;
	 int count = history_count;
	 for (count = history_count; count >= 0; count--)
	 {
		 pos = (current_buffer_pos - (history_count - count)) % history_count;
		 sum += error_history[pos];
	 }
	 return sum;
 }
 // Conceptually, the weighted sum errors will 'fade' out, previous errors and emphasise negative gradients, experimental

 /* return a proportional value related to the age of the reading, more recent, more weight */
 int weighted_sum_error()
 {
	 int pos = 0, sum = 0;
	 int count = history_count;
	 for (count = history_count; count >= 0; count--)
	 {
		 pos = (current_buffer_pos - (history_count - count)) % history_count;
		 sum += error_history[pos] * count;
	 }
	 return sum;
 }
