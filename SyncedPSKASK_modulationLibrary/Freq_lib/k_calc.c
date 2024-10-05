#include "frequency_itr.h"
//Evan Nikitin Wed Oct  2 09:22:05 PM -00 2024

#include<stdlib.h>

#include <stdio.h>

int main(int argn, char* argv[]){
  if(argn < 4){
    printf("specify: <frequency> <sample rate> <1 for sine, 0 for cos>\n");
    return 0;
  }
  double input = atof(argv[1]);
  int rate = atoi(argv[2]);
  int sine_cos = atoi(argv[3]);

  map_frequency(input,rate,sine_cos);
  print_array(0);
  free_d_mem();
  return 0;
}
