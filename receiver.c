#include "SyncedPSKASK_modulationLibrary/modulator.h"
#include "alsa/alsa.h"
#include<string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pwmplus.c"

//Evan Nikitin Wed Oct  2 09:22:05 PM -00 2024


int calculate_message_chrsum(char* message,int size){
  int i;
  int csum=0;
  for(i=0;i<size;i++){
    csum=csum+message[i];
  }
  return csum;
}

int loop[10];
int checkchar_loop(int input){

}

int main(int argn, char* argv[]){

  init_modulation_scheme(48000,21,400,6);
  create_transmitter(48000,20000);

  char tbuff[29];
  bzero(tbuff,sizeof(char)*22);

  int size = calculate_frame_size(30,30);
  printf("initialized\n");
  short frame[size];
  bzero(frame,sizeof(short)*size);
  unsigned int itterator = 0;
  //create_header(frame, &itterator);
  //
  //
  if(setup_alsa("default",NULL,size,48000)<0)
    return 0;

  printf("alsa ready\n");

  int bsize=-1;
  int rxcount=0;
  double gaincont = 1;
  short mval;
  int chrsrx;

  while(1){
    aread(frame);
    demod_carrier(frame,size);
    prepare_array(frame,size,gaincont);
    
    mval=getmaxval(frame,size);
    if(mval<14000){
      if(mval<5000){
        gaincont=gaincont+1;
      }
      gaincont=gaincont+0.1;
    }
    if(mval>15000){
      gaincont=gaincont-0.1;
      if(gaincont<1){
        gaincont=1;
      }
    }


    while(itterator<size){
      if(wait_for_sync(frame,&itterator,size,100)!=-1){

        int output=(int)demod(frame,&itterator,size,100);
        int outputcpy=output;
        int position;
        if(output!=-1){
          position=(output>>16);
          output=output&65535;
          if(outputcpy%273==0){
              outputcpy=outputcpy/273;
              if(outputcpy!=0){
                if(outputcpy<=(255*bsize)){
                  chrsrx=outputcpy;
                  printf("%d\n",chrsrx);
                }
              }
            
          }else if(output%257==0){
            output=output/257;
            if(output!=0){
              if(position-1<=bsize&&position>1){

                if(strlen(tbuff)==bsize){
                    int bef=calculate_message_chrsum(tbuff,bsize);
                    char prev=tbuff[position-2];
                    tbuff[position-2]=output;
                    int cur=calculate_message_chrsum(tbuff,bsize);
                    if(bef==chrsrx&&cur!=chrsrx)
                      tbuff[position-2]=prev;
                }else{
                  if(rxcount<bsize && tbuff[position-2] ==0 )
                  rxcount++;
                  tbuff[position-2]=output;
                }

                tbuff[bsize]=0;
                if(strlen(tbuff)==bsize && calculate_message_chrsum(tbuff,bsize)==chrsrx){
                 printf("%s\n",tbuff);
                }else{
                  printf("received: %d/%d %s\n",rxcount,bsize,tbuff);
                }
              }else if(position==1){
                if(output<29)
                bsize=output;
              }
            }
          }
        }

      }
    }
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
