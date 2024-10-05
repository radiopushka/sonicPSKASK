
//Evan Nikitin Wed Oct  2 09:22:05 PM -00 2024
#ifndef FARRAYS

struct data_array{
  double* farray;
  double* endptr;
  double* startptr;
  int size;
  int counter;
};


//the function responsible for generating the pre calculated sine cosine array
//1 for sine_cosine means sine , other value means cosine
struct data_array* generate_conv_array(int sample_rate, float frequency_hz, int sine_cosine);

//nullifies and releases the structure memory
void rm_conv_array(struct data_array** darr);


int get_period_samples(float frequency_hz, int sample_rate);
#endif

