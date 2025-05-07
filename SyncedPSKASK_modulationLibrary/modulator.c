#include "Freq_lib/frequency_itr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "modulator.h"
#include "../alsa/alsa.h"

//Evan Nikitin Wed Oct  2 09:22:05 PM -00 2024

/*int period_samples;
int bits_packet;
int packet_size;
double amplitude;
unsigned int dframe;
int clockphase = -1;
*/

void move_to_half_cycle(Modulator m){
    reset_counter(m->m_freq);

}

void reset_scheme(Modulator m){
  move_to_half_cycle(m);
}
//initial attempt was to perform OFDM with averaging has failed.
//this is too computationally heavy to pass each frequency through FIFO and a low pass convolution
//we will go with mixed keying PSK
//off indicates next bit, start of communication will have a sync packet, there will be two bits of sync
//180 degrees out of phase indicates 0, in phase indicates 1
//the sync starts with a 1,0 sequence, there first two bits are rezerved
//ASK
//


//changing this number helps with multipath
//70 is best for 400 hz rate i will try the formula
//kawaru = minamp/(samplerate/switching_freq)
//keep minimum amplitude around 14000 and max around 15000 for the AGC
 
Modulator init_modulation_scheme(int samplerate, int bits,  float startfreq,int lowpass_strength,int tight_filter){

  if(bits < 1){
    printf("cannot create modulation scheme for 0 bits, ABORT\n");
    return NULL;
  }
  Modulator mod=malloc(sizeof(struct Modulator));
  mod->dframe=0;
  mod->clockphase=-1;
  int i;
  for(i=0;i<bits;i++){
    mod->dframe=((mod->dframe)<<1)|1;
  }
  bits++;

  mod->uptime=0;
  mod->downtime=0;

  mod->period_samples=(get_period_samples(startfreq,samplerate))>>1;
  mod->period_samples_t3=mod->period_samples*4;
  mod->quarter_cycle=mod->period_samples>>2;
  mod->m_freq=map_frequency(startfreq,samplerate,1);
  mod->phase=1;
  mod->detected_period=(mod->period_samples);

  mod->amplitude = 22000;
  mod->bits_packet=bits;
  mod->packet_size=bits*mod->period_samples;

  mod->filterc=lowpass_strength;

  float ffreq=startfreq;
  mod->filters = malloc(sizeof(LPF)*lowpass_strength);
  for(i=0;i<lowpass_strength;i++)
    mod->filters[i]=create_LPF(samplerate,ffreq*3,1);


  reset_counter(mod->m_freq);


  return mod;
}

void prepare_array(Modulator m,short* data, int size,float gain){
  short* dend=data+size;
  short* dst = data;
  int i;
  for(i=0;i<m->filterc;i++){
   while(dst<dend){

      *dst=convolute((*dst)*gain,m->filters[i]);

    dst++;
   }
    dst=data;
  }
}

void free_global_mem(){
  free_d_mem();
}
void free_mod_mem(Modulator m){

  int i;
  for(i=0;i<m->filterc;i++){
    LPF* ftmp=m->filters[i];
    free_lpf(&ftmp);
    m->filters[i]=ftmp;
  }
  free(m->filters);
}

int get_packet_size_buffer(Modulator m){
  return m->packet_size + m->period_samples*5;
}
int calculate_frame_size(Modulator m,int packets,int syncs){
  return m->packet_size*packets+syncs*m->period_samples*10;
}
void create_sync_packet(Modulator m,short* targ_array,unsigned int* array_itterator){
  //creates the packet with the phase sync
  int flip_count = 0;
  m->clockphase=1;
  double val;
  unsigned int itop=*array_itterator;
  reset_counter(m->m_freq);
  while(flip_count<10){
    

    val=value_at(m->m_freq);
    if(is_cross(m->m_freq)==1){
      if(flip_count<7){
        m->clockphase=0;
      }else{
        m->clockphase=1;
      }
      flip_count++;
    }
    if(flip_count<10){
      targ_array[itop] = val*m->amplitude*m->clockphase;
      itop++;
    }
  }
  *array_itterator=itop;

}

void create_packet(Modulator m,short* targ_array, unsigned long data_in, unsigned int* array_itterator){
  unsigned int itop=*array_itterator;
  int flip_count = 0;
  m->clockphase=1;
  double val;
  while(flip_count<m->bits_packet){


    val=value_at(m->m_freq);
    if(is_cross(m->m_freq)==1){
        m->clockphase=(data_in&1);
        if(m->clockphase==0)
          m->clockphase=-1;

      //printf("%d ",(int)data_in&1);
        data_in=data_in>>1;
        flip_count++;
      
    }
    if(flip_count<m->bits_packet){
      targ_array[itop] = val*m->amplitude*m->clockphase;
      itop++;
    }
  }
  *array_itterator=itop;
    //printf("\n");


}



// demodulation, 

int get_largest_value(short* array, int size){
  int largest=-1;
  int indexp=-1;
  short* end=array+size;
  for(short* ptr=array; ptr<end; ptr++){
    int absv=abs(*ptr);
    if(absv>largest){
      largest=absv;
      indexp=ptr-array;
    }
  }
  return indexp;
}

int peak_detector(Modulator m, unsigned int array_size,short* targ_array, unsigned int start_index){

  //constant:
  const int number_half_cycles=3;
  /*
  * step 1:
  * rectify and find average, estimate length of sampling using given period samples, 3 period samples
  */
  unsigned int buffer_time = m->period_samples * number_half_cycles;

  unsigned int bound = start_index + buffer_time;
  
  //check if we are out of bounds T(1):
  if(bound > array_size)
    return -2;

  short check_buffer[buffer_time+m->period_samples];
  memcpy(check_buffer,targ_array+start_index,buffer_time<<1);
  int lindex1=get_largest_value(check_buffer,buffer_time);
  short* start=check_buffer + lindex1 - m->quarter_cycle;
  if(start<check_buffer){
    start=check_buffer;
  }
  memset(start,0,m->period_samples<<1);
  int lindex2=get_largest_value(check_buffer,buffer_time);
  start=check_buffer + lindex2 - m->quarter_cycle;
  if(start<check_buffer){
    start=check_buffer;
  }
  memset(start,0,m->period_samples<<1);
  int lindex3=get_largest_value(check_buffer,buffer_time);

  //order the three largest values by position in array
  int tmp;
  if(lindex2<lindex1 && lindex2<lindex3){
    tmp=lindex1;
    lindex1 = lindex2;
    lindex2=tmp;
  }
  if(lindex3<lindex1 && lindex3<lindex2){
    tmp=lindex1;
    lindex1 = lindex3;
    lindex3=tmp;
  }

  if(lindex3<lindex2){
    tmp=lindex2;
    lindex2=lindex3;
    lindex3=tmp;
  }


  //determine the period:
  unsigned int period1 = lindex2 - lindex1;
  unsigned int period2 = lindex3 - lindex2; 

  int ip1=lindex1+start_index;
  int ip2=lindex2+start_index;
  int ip3=lindex3+start_index;

  //printf("periods: %d %d %d  %d %d %d \n",period1, period2, m->period_samples,start_index, ip2 ,ip3);

  int difference_p1 = abs((m->period_samples) - period1);
  int difference_p2 = abs((m->period_samples) - period2);


  m->detected_period=period1;
  int start_point=ip1;
  if(targ_array[ip1]>0){
      m->phase=1;
  }else{
      m->phase=-1;
  }

  if(difference_p2<difference_p1){
    //than peak 2 to 1 is the optimal point to analyze
    start_point=ip2-m->period_samples;
    m->detected_period=period2;
    if(targ_array[ip3]>0){
      m->phase=1;
    }else{
      m->phase=-1;
    }
  }
  if(start_point<0){
    start_point=0;
  }
  //printf("startloc: %d\n",start_point);

  return start_point;
}





  int averaging = 0;
int wait_for_sync(Modulator m,short* targ_array, unsigned int* array_itterator,int array_size,int squelch){
  unsigned long i;




  if(*array_itterator>=array_size){
    return -2;
  }
  int downtime=m->downtime;
  int uptime=m->uptime;
  //downtime=0;

  //uptime=0;
  int quarter_cycle=(m->period_samples)>>2;
  double prev=targ_array[*array_itterator];
  int vabs;
  for(i=*array_itterator;i<array_size;i++){
    vabs=abs(targ_array[i]);
    if(vabs<squelch){
      downtime++;
      if(downtime>quarter_cycle)
      uptime=0;
    }else{
      if(downtime>m->period_samples_t3 && uptime> quarter_cycle){
        //clock=0;
               
        //downtime=0;
        //uptime=0;
        int peak=peak_detector(m,array_size,targ_array,i);
        if(peak > -1){

          //printf("synced\n");
          *array_itterator=peak;
          m->downtime=0;
          m->uptime=0;
          return 1;
        }else if (peak==-2){
          
          m->uptime=uptime;
          m->downtime=downtime;

          *array_itterator=i;
          return -1;
        }        

        //printf("%d %d\n",targ_array[i-(squelch/rblk)],targ_array[i]);
        //*array_itterator=i-squelch/rblk;
        //*array_itterator=i;
        //*array_itterator=i+((14000-squelch)/rblk)*2;
        //need to write a better peak detector function
        /*int peak=0;
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
        return -1;*/

      }else if(uptime>quarter_cycle){
        downtime=0;
      }
      uptime++;
    }
    
    prev=targ_array[i];
  }
  *array_itterator=i;
  m->uptime=uptime;
  m->downtime=downtime;
  return -1;
}

unsigned int carray=0;


long demod(Modulator m,short* targ_array, unsigned int* array_itterator,int array_size){
  //demodulation function
  unsigned int i,i2;
  int value;
  unsigned long outval=0;
  unsigned long packet=0;
  char bin;
  int shifts=0;
  int period=m->period_samples;
  int phase=m->phase;

  short* tp=targ_array;

  int bp=m->bits_packet+3;

  short* final_l=targ_array+array_size;
  i=*array_itterator;
  targ_array=targ_array+i;
  while(targ_array < final_l){
  
      if(shifts<bp){
        value=(*targ_array)*(phase);
        //printf("%d ",value);

        if(value>0){
          bin=1;
        }else{
          bin=0;
        }
        //printf("%d ",bin);
        outval=(outval<<1)|(bin&1);
      }else{
        outval=(outval>>1)&(m->dframe);
        for(i2=1;i2<m->bits_packet;i2++){
          packet=(packet<<1)|(outval&1);
          outval=outval>>1;
        }
        //printf("\n");
        //printf("%d\n",packet);
        *array_itterator=targ_array-tp;
        return packet;
      }

      phase=-phase;
      shifts++;

    //prev=targ_array[i];
    //value_at(2);
    targ_array=targ_array+period;
    
  }
  *array_itterator=targ_array-tp;

  return -1;


}

long demod2(Modulator m,short* targ_array,int array_size,int sq){

  int i;
  int pause_count=0;
  int hasprint=0;
  int buffer;
  int count=0;
  for(i=0;i<array_size;i++){

    if(abs(targ_array[i])>sq){
      if(pause_count>m->period_samples_t3){
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
