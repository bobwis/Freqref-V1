/*
 * PID.c
 *
 * Created: 3/10/2017 6:06:56 PM
 *  Author: paul
 */ 
 static const unsigned int history_count = 100;
 static unsigned int current_buffer_pos = 0;
 static int error_history[history_count];

 // store/forward proportional error values (not dacvals!!)

 void storeError(int error)
 {
	 error_history[current_buffer_pos] = error;
	 current_buffer_pos = ++current_buffer_pos % history_count;
 }

 int sum_error()
 {
	 int i = 0, sum = 0;
	 for (i = 0; i<history_count; i++)
	 {
		 sum += error_history[i];
	 }
	 return sum;
 }

 /* return a proportional value related to the age of the reading, more recent, more weight */
 int weighted_sum_error()
 {
	 int i = 0, sum = 0;
	 int count = history_count;
	 int pos = current_buffer_pos;
	 for (count = 50; count >= 0; count--)
	 {
		 i = (current_buffer_pos - (history_count - count)) % history_count;
		 sum += error_history[i] * count;
	 }
	 return sum;
 }
