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

  init_modulation_scheme(48000,21,500,6,1);
  create_receiver(48000,19000);

  char tbuff[29];
  bzero(tbuff,sizeof(char)*29);

  int size = calculate_frame_size(15,15);
  printf("initialized\n");
  short frame[size];
  short frame2[size];
  bzero(frame,sizeof(short)*size);
  unsigned int itterator = 0;
  //create_header(frame, &itterator);
  //
  //
  if(setup_alsa("default",NULL,size,48000)<0)
    return 0;

  printf("alsa ready\n");
  printf("\n");

  int bsize=-1;
  int rxcount=0;
  double gaincont = 1;
  short mval;
  int chrsrx=-1;

  int msgrx=0;

  int framegain=25000;
  int sqg=framegain/4;
  int error=0;
  int peakavg=0;
  int success=0;
  int success_past=0;
  double cgain;
  double cgain_cgain;



  int success_rate_max=0;
  double success_rate_gain=0;


  //PID gain control
  int previous_error=0;
  int error_over=0;


  while(msgrx==0){
    aread(frame);
    demod_carrier(frame,size);
    prepare_array(frame,size,gaincont);
    cgain=gaincont;
    
    mval=getmaxval(frame,size);
    error=(framegain/2)-mval;
    //500 hz bandwidth
    gaincont=0.00059*(error) + 0*(error-previous_error) + 0.00025*(error_over);
    //300 hz band width
    //gaincont=0.001*(error) + 0*(error-previous_error) + 0.001*(error_over);
    error_over=(error_over+error)/2;
    previous_error=error;

    while(itterator<size){
      if(wait_for_sync(frame,&itterator,size,sqg)!=-1){

        int output=(int)demod(frame,&itterator,size,sqg);
        int outputcpy=output;
        int position;
        if(output!=-1){
          position=(output>>16);
          output=output&65535;
          if(outputcpy%273==0){
              outputcpy=outputcpy/273;
              if(outputcpy!=0){
                if(outputcpy<=(255*bsize)){
                  success++;
                  chrsrx=outputcpy;
                }
              }
            
          }else if(output%257==0){
            output=output/257;
            if(output!=0){
              if(position-1<=bsize&&position>1){

                success++;
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
                int chrsm=calculate_message_chrsum(tbuff,bsize);
                if(strlen(tbuff)==bsize && chrsm==chrsrx){
                 printf("\n%s\n",tbuff);
                 bzero(tbuff,sizeof(char)*29);
                 bsize=-1;
                 rxcount=0;
                  chrsrx=-1;
                 //msgrx=1;
                 break;
                }else{
                  printf("received: %d/%d %d vs %d          \r",rxcount,bsize,chrsm,chrsrx);
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
    //printf("%g %d %d %d \n",cgain, success,success_past,mval);
    
    cgain_cgain=cgain;
    success_past=success;
    success=0;
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
