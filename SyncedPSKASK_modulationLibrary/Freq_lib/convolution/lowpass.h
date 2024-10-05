#ifndef HIGHCUT
#define HIGHCUT

struct Low_pass{
	double* buffer;
	double* cw;
  double* cw_end;
  double* bend;
  double strength;
};

typedef struct Low_pass LPF;

LPF* create_LPF(int sample_rate, int freq, double strength);



double convolute(double inval,LPF* filter);


void free_lpf(LPF** lin);

#endif // !HIGHCUT
