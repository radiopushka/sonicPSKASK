#include "Freq_lib/frequency_itr.h"
#include "Freq_lib/convolution/lowpass.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "modulator.h"

//Evan Nikitin Wed Oct  2 09:22:05 PM -00 2024

int period_samples;
int bits_packet;
int packet_size;
double amplitude;
unsigned int dframe;
int clockphase = -1;
LPF* hc;


void move_to_half_cycle(){
    reset_counter(0);
    reset_counter(1);
    reset_counter(2);

}

void reset_scheme(){
  move_to_half_cycle();
}
//initial attempt was to perform OFDM with averaging has failed.
//this is too computationally heavy to pass each frequency through FIFO and a low pass convolution
//we will go with mixed keying PSK
//off indicates next bit, start of communication will have a sync packet, there will be two bits of sync
//180 degrees out of phase indicates 0, in phase indicates 1
//the sync starts with a 1,0 sequence, there first two bits are rezerved
//ASK
//
LPF** filters;
int filterc=0;
 
void init_modulation_scheme(int samplerate, int bits,  float startfreq,int lowpass_strength){

  if(bits < 1){
    printf("cannot create modulation scheme for 0 bits, ABORT\n");
    return;
  }
  dframe=0;
  clockphase=-1;
  int i;
  for(i=0;i<bits;i++){
    dframe=(dframe<<1)|1;
  }
  bits++;

  period_samples=(get_period_samples(startfreq,samplerate))/2;
  map_frequency(startfreq,samplerate,1);
  map_frequency(startfreq,samplerate,0);
  map_frequency(startfreq,samplerate,1);
  amplitude = 32000;
  bits_packet=bits;
  packet_size=bits*period_samples;

  filterc=lowpass_strength;

  filters = malloc(sizeof(LPF)*lowpass_strength);
  for(i=0;i<lowpass_strength;i++)
    filters[i]=create_LPF(samplerate,startfreq,1);


  move_to_half_cycle();
}

void prepare_array(short* data, int size,double gain){
  short* dend=data+size;
  double temp;
  short* dst = data;
  int i;
  for(i=0;i<filterc;i++){
   while(dst<dend){

      *dst=convolute((*dst)*gain,filters[i]);

    dst++;
   }
    dst=data;
  }
}

void free_mod_mem(){
  free_d_mem();

  int i;
  for(i=0;i<filterc;i++)
    free_lpf(&filters[i]);

  free(filters);
}

int get_packet_size(){
  return packet_size;
}
int calculate_frame_size(int packets,int syncs){
  return packet_size*packets+syncs*period_samples*6;
}
void create_sync_packet(short* targ_array,unsigned int* array_itterator){
  int flip_count = 0;
  clockphase=1;
  double val;
  unsigned int itop=*array_itterator;
  while(flip_count<6){
    

    val=value_at(0);
    if(is_cross(0)==1){
      if(flip_count<3){
        clockphase=0;
      }else{
        clockphase=1;
      }
      flip_count++;
    }
    if(flip_count<6){
      targ_array[itop] = val*amplitude*clockphase;
      itop++;
    }
  }
  *array_itterator=itop;

}

void create_packet(short* targ_array, unsigned long data_in, unsigned int* array_itterator){
  unsigned int itop=*array_itterator;
  int flip_count = 0;
  clockphase=1;
  double val;
  while(flip_count<bits_packet){


    val=value_at(0);
    if(is_cross(0)==1){
        clockphase=(data_in&1);
        if(clockphase==0)
          clockphase=-1;
        data_in=data_in>>1;
      flip_count++;
      
    }
    if(flip_count<bits_packet){
      targ_array[itop] = val*amplitude*clockphase;
      itop++;
    }
  }
  *array_itterator=itop;


}



// demodulation, must reset the library when switching modes


int sync_polarity = 1;


double kawaru=70;
//changing this number helps with multipath
//70

  int averaging = 0;
unsigned int phase=1;
int wait_for_sync(short* targ_array, unsigned int* array_itterator,int array_size,int squelch){
  unsigned long i;



  int going_up=0;
  int going_down=0;

  if(*array_itterator>=array_size){
    return -2;
  }
  int off_point=0;
  double prev=targ_array[*array_itterator];
  for(i=*array_itterator;i<array_size;i++){
    if(fabs(targ_array[i])<squelch){
      off_point=1;
    }else if((fabs(prev-targ_array[i])) > kawaru){
     // printf("%d\n",targ_array[i]);
      if(prev>targ_array[i]&&off_point==1){
        going_up=0;
        going_down=1;
        off_point=0;
      }else if (prev<targ_array[i]&&off_point==1){
        going_up=1;
        going_down=0;
        off_point=0;
      }else if(prev<targ_array[i] && going_down==1 && off_point==0){
        reset_counter(2);
        phase=-1;
        *array_itterator=i-1;
        return 1;

      }else if(prev>targ_array[i] && going_up==1 && off_point==0){
        reset_counter(2);
        phase=1;
        *array_itterator=i-1;
        return 1;
      }

    }
    
    prev=targ_array[i];
  }
  *array_itterator=i;
  return -1;
}

unsigned int carray=0;


long demod(short* targ_array, unsigned int* array_itterator,int array_size,int squelch){
  unsigned int i,i2;
  int value;
  unsigned long outval=0;
  unsigned long packet=0;
  char bin;

 if(*array_itterator>=array_size)
    *array_itterator=0;

  for(i=*array_itterator;i<array_size;i++){
    
    if(is_cross(2)==1){
      if(fabs(targ_array[i])>squelch){
        value=targ_array[i]*phase;
        if(value>0){
          bin=1;
        }else{
          bin=0;
        }
        outval=(outval<<1)|(bin&1);
      }else{
        outval=(outval>>1)&dframe;
        for(i2=1;i2<bits_packet;i2++){
          packet=(packet<<1)|(outval&1);
          outval=outval>>1;
        }
        //printf("%d\n",packet);
        *array_itterator=i;
        return packet;
      }
      phase=-phase;
    }
    value_at(2);
    
  }
  *array_itterator=i;

  return -1;


}
