#ifndef HIGHCUT
#define HIGHCUT

struct Low_pass{
	float* buffer;
	float* cw;
  float* cw_end;
  float* bend;
  float strength;
};

typedef struct Low_pass LPF;

LPF* create_LPF(int sample_rate, int freq, double strength);



float convolute(float inval,LPF* filter);


void free_lpf(LPF** lin);

#endif // !HIGHCUT
