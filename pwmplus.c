#include <stdlib.h>
#include <math.h>
#include "SyncedPSKASK_modulationLibrary/Freq_lib/frequency_itr.h"

int sindex;
int cindex;

const int doppler_drift=10;//drift in hz
const int doppler_mult=10;
int dasize=(doppler_drift/doppler_mult)*2+1;
int half=(doppler_drift/doppler_mult);
int* dp;
int* dav;
int* davp;

double rxamp=1;
double txamp=1;

double agctop=12000;
double agcbottom=3000;

double gainc=1;
double gup=0.001;
double gd = 0.01;

void create_transmitter(int samplerate, float frequency){
 sindex=map_frequency(frequency,samplerate,1); 
 cindex=map_frequency(frequency,samplerate,0); 
}

void create_receiver(int samplerate,float frequency){
  create_transmitter(samplerate,frequency);
  return;
  //not sure how to do this yet
  dp=malloc(sizeof(int)*2*dasize);
  dav=malloc(sizeof(int)*dasize);
  davp=malloc(sizeof(int)*dasize);
  float begin = frequency-doppler_drift;
  int i;
  int i2=0;
  for(i=0;i<dasize;i++){
    dp[i2]=map_frequency(begin,samplerate,1);
    i2++;
    dp[i2]=map_frequency(begin,samplerate,0);
    i2++;

    dav[i]=0;
    davp[i]=0;
    printf("generated fourier sequences %d/%d %g\n",i+1,dasize,begin);
    begin=begin+doppler_mult;
  }
  printf("%d\n",dasize);
}

short doppler_fourier(short input){
  short maxval=0;
  short maxval_unsigned=0;
  short hval;
  int tmp;
  short tmp_u;
  int i;
  int i2;
  int index=-1;
  for(i=0;i<dasize;i++){
    tmp=(input*value_at(dp[i2])+input*value_at(dp[i2+1]))/2;
    i2=i2+2;
    tmp_u=abs(tmp);
    if(i==half){
      hval=tmp;
    }
    //printf("average %d %d\n",dav[i],i);
    if(davp[i]>maxval_unsigned){
      maxval_unsigned=davp[i];
      maxval=tmp;
      index=i;
    }
    dav[i]=(dav[i]+tmp_u)/2;
  }
  if(index==-1){
    maxval=hval;
    index=half;
  }
      //printf("%d\n",index);
  
  return maxval;
}

void turn_to_u(short* array,int size){
  short* end= array+size;
  while(array<end){
    *array=(*array)*value_at(cindex)*txamp;
    array++;
  }
}

void demod_carrier(short* array,int size){
  short* end= array+size;
  while(array<end){
    //doppler effect compensation
    

    //*array=doppler_fourier(*array);
    *array=((((*array)*value_at(cindex))+((*array)*value_at(sindex)))/2.0)*rxamp;
    array++;
  }

  //memcpy(davp,dav,sizeof(int)*dasize);
  //bzero(dav,sizeof(int)*dasize);

}

short getmaxval(short* array,int size){
 short* end= array+size;
  int max=0;
  int maxf=0;
  int crossings=0;
  int prev=array[0];
  while(array<end){
    if(abs(*array)>max){
      max=abs(*array);
    }
    if(prev>0&&*array<0 || prev<0&&*array>0){
      if(crossings>20){

        if(maxf==0){
          maxf=max;
        }else{
          maxf=(maxf+max)/2;
        }
        max=0;
        crossings++;
      }
      crossings++;
    }
    prev=*array;
    array++;
  }
  return maxf;

}

void get_agc(short* array,int size){
  short* end= array+size;
  while(array<end){
    if(*array<agcbottom){
      gainc=gainc+gup;
    }else if(*array>agctop){
      gainc=gainc-gd;
      if(gainc<0.1){
        gainc=0.1;
      }
    }

    array++;
  }


}


