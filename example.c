#include "SyncedPSKASK_modulationLibrary/modulator.h"
#include "wav.c"
#include <stdio.h>
#include <stdlib.h>

//Evan Nikitin Wed Oct  2 09:22:05 PM -00 2024

void dump_to_file(short* frame, int size, int copies){
  FILE* f= fopen("output.wav","wb");
  channels = 1;
  srate =48000;
  file_size = size*copies;
  header_size=44;
  write_wav_header(f);
  int i;
  for(i=0;i<copies;i++){
    fwrite(frame,sizeof(short),size,f);
  }
  fclose(f);
}

int main(int argn, char* argv[]){

  init_modulation_scheme(48000,8,3000);

  int size = calculate_frame_size(1000,1000);
  printf("initialized\n");
  short frame[size];
  bzero(frame,sizeof(short)*size);
  unsigned int itterator = 0;
  //create_header(frame, &itterator);
  int i;
  for(i=0;i<1000;i++){
    create_sync_packet(frame,&itterator);
    create_packet(frame,96,&itterator);
  }
  //create_packet(frame,69,&itterator);
  itterator = 0;
  printf("packets generated\n");
  reset_scheme();
  prepare_array(frame,size,6);
    
  //while(wait_for_sync(frame,&itterator,size,100)==-1);
  int pcount=1;
  for(i=0;i<1000;i++){
    while(wait_for_sync(frame,&itterator,size,400)==-1);
    int output=(int)demod(frame,&itterator,size,400);
    if(output!=-1){
     printf("%d %d\n",output,pcount);
     pcount++;
    }
  }
  free_mod_mem();
  printf("array spots: %d\n",size);
  dump_to_file(frame,size,10);
  /*for(i=0;i<(48000/100);i++){
    printf("%d\n",frame[i]);
  }*/

  return 0;
  
}
