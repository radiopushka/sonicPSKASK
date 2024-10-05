#include<stdlib.h>
#include <math.h>
#include "frequency_arrays.h"

//Evan Nikitin Wed Oct  2 09:22:05 PM -00 2024


#define PI 3.14159265359

double pogreshnost = 0.00001;

void trans_write(struct data_array* input,double value){
  //append to the array because we do not know when we will hit an approximate phase match
  double* new_slot = malloc(sizeof(double) * (input->size + 1));

    int i;
    for(i = 0; i < input -> size ; i++){
      new_slot[i] = input->farray[i];
    }

  new_slot[input->size ] = value;
  input -> size = input->size + 1;
  free(input -> farray);
  input -> farray = new_slot;
}


int compare_match(double one, double two){
  double diff = fabs(one - two);


  if(diff<pogreshnost)
    return 1;
  return -1;
}

struct data_array* generate_conv_array(int sample_rate, float frequency_hz, int sine_cosine){

  struct data_array* d_arr = malloc(sizeof(struct data_array));
  d_arr->farray = NULL;
  d_arr->size = 0;
  d_arr->counter = -1;
  pogreshnost = 0.00001;

  double current = 0;
  int crossed = 0;
  double shifter = (frequency_hz / sample_rate)*(2*PI);
  double starting = 1;
  if(sine_cosine == 1)
    starting = 0;

  double value =-1;

  while(1){
    if(sine_cosine)
      value = sin(current);
    else
      value = cos(current);



    if((compare_match(starting,value) == 1) && (crossed >2 ))
      break;

    current = current + shifter;

    if(current >= 2*PI){
      current = current - (2*PI);
      if(crossed > 10){
        /*if(pogreshnost < 0.0001)
          pogreshnost = pogreshnost*10;*/
        crossed = 1;
      }
      crossed++;
    }

    trans_write(d_arr, value);
  }
  d_arr->startptr=d_arr->farray;
  d_arr->endptr=d_arr->farray+d_arr->size;

  return d_arr;


}

void rm_conv_array(struct data_array** darr){
  struct data_array* unpack = *darr;

  free(unpack -> startptr);
  free(unpack);
  *darr = NULL;
}


int get_period_samples(float frequency_hz, int sample_rate){
  return sample_rate/frequency_hz;
}
