#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "lowpass.h"

#define PI 3.141592653589793


LPF* create_LPF(int sample_rate, int freq, double strength){
  int cbsize=(sample_rate/freq)/2;



  LPF* l   = malloc(sizeof(LPF));
  l->buffer= malloc(sizeof(double)*cbsize);
  l->cw    = malloc(sizeof(double)*cbsize);

  l->cw_end   = l->cw+cbsize;
  l->bend     = l->buffer+cbsize;
  l->strength = strength;

  bzero(l->buffer,sizeof(double)*cbsize);

  double* ct=l->cw;

  double cv=PI/(cbsize-1);
  double stc=0;

  while(ct<l->cw_end){
    *ct = sin(stc) * strength; 
    stc = stc + cv;
    ct++;
  }
  return l;
}

double convolute(double inval,LPF* filter){
  double* stc    = filter->cw;
  double* stcend = filter->cw_end;
  double* buff   = filter->buffer;


  double tsuyo   = filter->strength;


  double at;

  double* stct = stc+1;
  double* bt   = buff+1;
  double avg     = (*stc)*(*bt);
  //printf("%g * %g\n",*stc,*bt);

  double* swp = stcend-1;

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
