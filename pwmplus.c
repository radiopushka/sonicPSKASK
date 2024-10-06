#include <stdlib.h>
#include <math.h>
#include "SyncedPSKASK_modulationLibrary/Freq_lib/frequency_itr.h"

int sindex;
int cindex;

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

    *array=((((*array)*value_at(cindex))+((*array)*value_at(sindex)))/2.0)*rxamp;
    array++;
  }

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


