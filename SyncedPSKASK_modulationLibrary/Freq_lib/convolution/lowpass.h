#ifndef LOWPASS
#define LOWPASS

//Evan Nikitin Wed Oct  2 09:22:05 PM -00 2024 - self learning radio player

void filter_buffer(short* buffer, int buffer_size);
void destroy_filter();
void setup_low_pass(int freqhz,int sample_rate);



#endif

