#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "lowpass.h"

#define PI 3.141592653589793


LPF* create_LPF(int sample_rate, int freq, double strength){
  int cbsize=(sample_rate/freq)/2;



  LPF* l   = malloc(sizeof(LPF));
  l->buffer= malloc(sizeof(float)*cbsize);
  l->cw    = malloc(sizeof(float)*cbsize);

  l->cw_end   = l->cw+cbsize;
  l->bend     = l->buffer+cbsize;
  l->strength = strength;

  bzero(l->buffer,sizeof(float)*cbsize);

  float* ct=l->cw;

  float cv=PI/(cbsize-1);
  float stc=0;

  while(ct<l->cw_end){
    *ct = sin(stc) * strength; 
    stc = stc + cv;
    ct++;
  }
  return l;
}

float convolute(float inval,LPF* filter){
  float* stc    = filter->cw;
  float* stcend = filter->cw_end;
  float* buff   = filter->buffer;


  float tsuyo   = filter->strength;


  float at;

  float* stct = stc+1;
  float* bt   = buff+1;
  float avg     = (*stc)*(*bt);
  //printf("%g * %g\n",*stc,*bt);

  float* swp = stcend-1;

  for(;stct < stcend;stct++){
    *(bt-1) = *bt;
    bt++;

    if(stct == swp){
      bt--;
      at  = (*stct)*inval;
      //printf("%g * %g\n",*stct,inval);
    }else{
      at  = (*stct)*(*bt);
      //printf("%g * %g\n",*stct,*bt);
    }

    avg = (avg + at)/2;

    

  }
  //printf("\n");
  *bt = inval*3;

  return avg/tsuyo;
  

}


void free_lpf(LPF** lin){

  free((*lin)->cw);
  free((*lin)->buffer);

  free(*lin);
  *lin=NULL;

}
