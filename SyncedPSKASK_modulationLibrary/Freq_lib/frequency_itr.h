#include "frequency_arrays.h"
//Evan Nikitin Wed Oct  2 09:22:05 PM -00 2024

#ifndef Freq_itr

//comment this out on PC
//#define IS_MCU
// this code can run on a microcontroller but for that it needs the frequency data arrays to be pre generated
// you need to run this code on a powerfull machine or implement trigonometric functions on your MCU
//
//----
//

//the function responsible for generating the pre calculated sine cosine array
//1 for sine_cosine means sine , other value means cosine
int map_frequency(float frequency, int samplerate,int sine_cos);

//automatically increments the arrays and returns the next phase value
double value_at(int freq_index);
int is_cross(int freq_index);

void reset_counter(int freq_index);

#ifndef IS_MCU

void print_array(int freq_index);
#endif // !IS_MCU
       

void free_d_mem();//free memory
                  //
#endif
