#include "SyncedPSKASK_modulationLibrary/modulator.h"
#include "alsa/alsa.h"
#include<string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pwmplus.c"

//Evan Nikitin Wed Oct  2 09:22:05 PM -00 2024


int main(int argn, char* argv[]){


  int charsum=0;

  char* frametxt=argv[1];
  if(strlen(frametxt)>27){
    frametxt[28]=0;
  }
  int framesize=strlen(frametxt)+1;
  printf("string frames: %d %s\n",framesize,frametxt);

  int i;
  for(i=0;i<framesize;i++){
    charsum=charsum+frametxt[i];
  }

  init_modulation_scheme(48000,21,500,6,0);
  create_transmitter(48000,24000);

  int size = calculate_frame_size(framesize+1,framesize+1);
  printf("initialized\n");
  short frame[size];
  bzero(frame,sizeof(short)*size);
  unsigned int itterator = 0;
  //create_header(frame, &itterator);
  //
  //
  if(setup_alsa(NULL,"default",size,48000)<0)
    return 0;

  printf("alsa ready\n");
  short prev;
  short mval;
  double gain=1;
  int top =4400;
  while(1){
    int i;
    for(i=0;i<framesize+1;i++){
      create_sync_packet(frame,&itterator);
      if(frametxt[i]==0 && i<framesize){

        create_packet(frame,((framesize-1)*257)|(1<<16),&itterator);
      }else if(i==framesize){
        create_packet(frame,charsum*273,&itterator);
        //printf("charsum: %d\n",charsum);
      }else{
        create_packet(frame,(frametxt[i]*257)|((i+2)<<16),&itterator);
      }
    }
    prepare_array(frame,itterator-1,4);
    turn_to_u(frame,itterator-1);
    mval=getmaxval(frame,itterator-1);
    if(mval<top){
      if(mval<top/2){
        gain=gain+1;
      }
      gain=gain+0.01;
    }
      
    awrite(frame,itterator-1);
    prev=frame[itterator-1];
    itterator=0;
  }
  free_alsa();
  /*
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
*/
  return 0;
  
}
