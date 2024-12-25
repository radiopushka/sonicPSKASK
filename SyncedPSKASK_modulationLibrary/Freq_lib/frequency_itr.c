#include "frequency_itr.h"
#include <stdlib.h>
#include <stdio.h>
//Evan Nikitin Wed Oct  2 09:22:05 PM -00 2024

//begin here section of code for microcontrollers only
#ifdef IS_MCU




#endif /* ifdef IS_MCU */
//end section of code

struct data_array** list_arrays = NULL;;
int d_size = 0;

int append_array(struct data_array* darr){

    struct data_array** lar = malloc(sizeof(struct data_array*) * (d_size + 1));
    
    int i;
    for(i = 0; i < d_size;i++){
      lar[i]=list_arrays[i];
    }
    lar[i] = darr;
    free(list_arrays);
    list_arrays = lar;
    d_size = d_size + 1;
    return d_size - 1;
}

void free_d_mem(){
  #ifndef IS_MCU
  struct data_array* dcp;
  int i;
  for(i = 0; i < d_size; i++){
    dcp = list_arrays[i];
    rm_conv_array(&dcp);
  }
  free(list_arrays);
  list_arrays = NULL;

  
  #endif /* ifndef IS_MCU */
}

int map_frequency(float frequency, int sample_rate, int sine_cos){
  #ifndef IS_MCU
   return append_array(generate_conv_array(sample_rate , frequency, sine_cos)); 
  #endif /* ifndef IS_MCU , computer code only*/

  #ifdef IS_MCU
  
  #endif /* ifdef IS_MCU */
}

void reset_counter(int freq_index){
  #ifndef IS_MCU
    struct data_array* scut =  list_arrays[freq_index];
    scut->farray=scut->startptr;

  #endif /* ifndef IS_MCU */

  #ifdef IS_MCU
  
  #endif /* ifdef IS_MCU */
}
int is_cross(int freq_index){
 #ifndef IS_MCU
    struct data_array* scut =  list_arrays[freq_index];

    if(*(scut->farray) == 0)
      return 1;

    if((*(scut->farray-1)>0)&&(*(scut->farray)<0))
      return 1;

   if((*(scut->farray-1)<0)&&(*(scut->farray)>0))
      return 1;

    return -1;
  #endif /* ifndef IS_MCU */
  #ifdef IS_MCU
  
  #endif /* ifdef IS_MCU */

}
double value_at(int freq_index){
  #ifndef IS_MCU
    struct data_array* scut =  list_arrays[freq_index];
    scut->farray = scut->farray+1;
    if(scut->farray >= scut->endptr ){
      scut->farray = scut->startptr;
    }
    return *(scut->farray);
  #endif /* ifndef IS_MCU */
  #ifdef IS_MCU
  
  #endif /* ifdef IS_MCU */
}

#ifndef IS_MCU

void print_array(int freq_index){

    struct data_array* scut =  list_arrays[freq_index];
    int i;
    for(i = 0;i < scut->size;i++){
      printf("%g,",scut->farray[i]);
    }
  printf("\nsize:%d\n",scut->size);
}

#endif /* ifndef IS_MCU */
  
