#ifndef MOD_LIB

#include "Freq_lib/convolution/lowpass.h"
struct Modulator{

  //basics
  int period_samples;
  int bits_packet;
  int packet_size;
  double amplitude;
  unsigned int dframe;
  int clockphase;

  int period_samples_t3;
  int quarter_cycle;

  //the 500khz PSK sine
  int m_freq;
  int detected_period;

  //filters
  LPF** filters;
  int filterc;
  int phase;

  //sync detect:
  int downtime;
  int uptime;

};
typedef struct Modulator* Modulator;

//note: this modulation scheme is pretty spectrum efficient 
//there might be plans to allow the creation of multiple instances of 
//init_modulation_scheme
//you cannot run several modulators on one wire/line/carrier
//you can however, create carriers and amplitude modulate them with this

//Evan Nikitin Wed Oct  2 09:22:05 PM -00 2024

Modulator init_modulation_scheme(int samplerate,int bits,  float startfrequency, int lpf_strenght, int filter_tight);
//this function sets up the modulation params for the specific frequency and sample rate
//put one for filter_tight for a stricter low pass, better for reception
//put zero for transmission

void create_sync_packet(Modulator m,short* targ_array,unsigned int* itterator);
//this function modulates a synchronization trigger
//it is recommended to call this everytime before a packet is created

void create_packet(Modulator m,short* targ_array,unsigned long data_in,unsigned int* itterator);
//this creates the packet of *bits* length

int calculate_frame_size(Modulator m,int packets,int syncs);
int get_packet_size_buffer(Modulator m);
//this calculates the frame size that you write to file or send to alsa
//the number of packets and syncs should be the same, change them as appropriate
//syncs is create_sync_packet packets is create_packet

int wait_for_sync(Modulator m,short* targ_array, unsigned int* itterator,int array_size,int squelch);
//this will read the buffer and returns one of several things, 1 if the sync succeds
//-2 if the end of the buffer is reached
//-1 if the current frame failed or the end of the buffer is reached
//see example.c for proper usage
//usually after it returns -1 you can acquire the next buffer and not worry about anything else

long demod(Modulator m,short* targ_array, unsigned int* array_itterator,int array_size);
//this function gets called right after wait for sync passes
//it returns -1 if there is a failure, else it returns the demodulated data
//we will take care of the seemless buffer in the main application utilizing memcpy

long demod2(Modulator m,short* targ_array,int size,int sq);
void free_mod_mem(Modulator m);
//this frees alocated memory

void prepare_array(Modulator m,short* samples,int size,float gain);
//this needs to be called before a frame is sent to alsa or after a frame is picked up from alsa
//this is the band pass filter

void reset_scheme(Modulator m);
//this needs to be called if switching from demodulation to modulation or vice versa

//this is called at the very end to free the synthesizer library, call this after all modulator objects have been freed
void free_global_mem();

#endif // !MOD_LIB
