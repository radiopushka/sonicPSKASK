#include "Freq_lib/frequency_itr.h"
#include "Freq_lib/convolution/lowpass.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "modulator.h"
#include "../alsa/alsa.h"

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

double kawaru;
int rblk;
//changing this number helps with multipath
//70 is best for 400 hz rate i will try the formula
//kawaru = minamp/(samplerate/switching_freq)
//keep minimum amplitude around 14000 and max around 15000 for the AGC
 
void init_modulation_scheme(int samplerate, int bits,  float startfreq,int lowpass_strength,int tight_filter){

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

  float ffreq=startfreq;
  filters = malloc(sizeof(LPF)*lowpass_strength);
  for(i=0;i<lowpass_strength;i++)
    filters[i]=create_LPF(samplerate,ffreq+(ffreq*0.2),1);


  move_to_half_cycle();

  kawaru=14000/(samplerate/startfreq);
  rblk=14000/(period_samples);
}

void prepare_array(short* data, int size,float gain){
  short* dend=data+size;
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

int get_packet_size_buffer(){
  return packet_size + period_samples*5;
}
int calculate_frame_size(int packets,int syncs){
  return packet_size*packets+syncs*period_samples*10;
}
void create_sync_packet(short* targ_array,unsigned int* array_itterator){
  //creates the packet with the phase sync
  int flip_count = 0;
  clockphase=1;
  double val;
  unsigned int itop=*array_itterator;
  reset_counter(0);
  while(flip_count<10){
    

    val=value_at(0);
    if(is_cross(0)==1){
      if(flip_count<7){
        clockphase=0;
      }else{
        clockphase=1;
      }
      flip_count++;
    }
    if(flip_count<10){
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

      //printf("%d ",(int)data_in&1);
        data_in=data_in>>1;
      flip_count++;
      
    }
    if(flip_count<bits_packet){
      targ_array[itop] = val*amplitude*clockphase;
      itop++;
    }
  }
  *array_itterator=itop;
    //printf("\n");


}



// demodulation, must reset the library when switching modes

int periodv=0;
int clock=0;

int sync_polarity = 1;




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
  int downtime=0;
  int uptime=0;
  int closest_0=squelch;
  int quarter_cycle=period_samples/4;
  int closeindex=-1;
  double prev=targ_array[*array_itterator];
  int vabs;
  for(i=*array_itterator;i<array_size;i++){
    vabs=abs(targ_array[i]);
    if(vabs<squelch){
      downtime++;
      if(downtime>quarter_cycle)
      uptime=0;
    }else{
      if(downtime>period_samples*3 && uptime> quarter_cycle){
        //clock=0;
               
        //printf("%d %d\n",targ_array[i-(squelch/rblk)],targ_array[i]);
        //*array_itterator=i-squelch/rblk;
        //*array_itterator=i;
        //*array_itterator=i+((14000-squelch)/rblk)*2;
        int peak=0;
        int peak_index=-1;
        int bindex=0;
        int stopping=period_samples-(quarter_cycle);
        while(i<array_size){
          vabs=abs(targ_array[i]);
          if(bindex>=stopping){
            if(targ_array[peak_index]<0){
              phase=-1;
            }else{
              phase=1;
            }
            *array_itterator=peak_index;
            clock=periodv;
            periodv=period_samples;
            return 1;
          }
          if(vabs>peak){
            peak=vabs;
            peak_index=i;
          }
          bindex++;
          i++;
        }

        *array_itterator=i;
        return -1;
      }else if(uptime>quarter_cycle){
        downtime=0;
      }
      uptime++;
    }
    
    prev=targ_array[i];
  }
  *array_itterator=i;
  return -1;
}

unsigned int carray=0;


long demod(short* targ_array, unsigned int* array_itterator,int array_size,int sq){
  //demodulation function
  unsigned int i,i2;
  int value;
  unsigned long outval=0;
  unsigned long packet=0;
  char bin;
  int shifts=0;

  short* tp=targ_array;

  int bp=bits_packet+3;

  i=*array_itterator;
  targ_array=targ_array+i;
  while(1){
  
    if(clock>=periodv){
      if(shifts<bp){
        value=(*targ_array)*(phase);
        //printf("%d ",value);

        if(abs(*targ_array)<sq){
          
          *array_itterator=targ_array-tp;
          return -2;
        }

        if(value>0){
          bin=1;
        }else{
          bin=0;
        }
        //printf("%d ",bin);
        outval=(outval<<1)|(bin&1);
      }else{
        outval=(outval>>1)&dframe;
        for(i2=1;i2<bits_packet;i2++){
          packet=(packet<<1)|(outval&1);
          outval=outval>>1;
        }
        //printf("\n");
        //printf("%d\n",packet);
        *array_itterator=targ_array-tp;
        return packet;
      }

      phase=-phase;
      clock=0;
      shifts++;

    }
    //prev=targ_array[i];
    //value_at(2);
    clock++;
    targ_array++;
    
  }
  *array_itterator=targ_array-tp;

  return -1;


}

long demod2(short* targ_array,int array_size,int sq){

  int i;
  int pause_count=0;
  int hasprint=0;
  int buffer;
  int count=0;
  for(i=0;i<array_size;i++){

    if(abs(targ_array[i])>sq){
      if(pause_count>period_samples*3){
        buffer=0;
        printf("\n size: %d\n",count);
        count=0;
      }
      if(targ_array[i]<0){
        if(hasprint==0){
          buffer=buffer<<1;
          printf("-1");
          count++;
          hasprint=1;
        }
      }else{
        if(hasprint==0){
          printf("1");
          buffer=(buffer<<1)|1;
          count++;
          hasprint=1;
        }

      }
      pause_count=0;
    }
    hasprint=0;
    pause_count++;
  }
  printf("\n");

}
