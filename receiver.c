#include "SyncedPSKASK_modulationLibrary/modulator.h"
#include "alsa/alsa.h"
#include<string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pwmplus.c"

//Evan Nikitin Wed Oct  2 09:22:05 PM -00 2024


int calculate_message_chrsum(unsigned char* message,int size){
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
  double gaincont = 1;
int avgavg=0;

void receive_signal(Modulator m,short* frame, int size, int framegain){
    aread(frame,size);
    float error,mval;
    demod_carrier(frame,size);
    de_sample(frame,size,8);
    prepare_array(m,frame,size,gaincont);
    //printf("\n gain; %g\n",gaincont);
    
    mval=getavgval(frame,size);
    avgavg=(avgavg+(int)mval)>>1;
   /* if(mval < 1)
      return;*/

    error=framegain-avgavg;
    //printf("%d\n",mval);

  /*
  //PID controller
    //500 hz bandwidth
    gaincont=0.00059*(error) + 0*(error-previous_error) + 0.00025*(error_over);
    //300 hz band width
    //gaincont=0.001*(error) + 0*(error-previous_error) + 0.001*(error_over);
    */
  //schmidt controller
  //courtesy of Sergey Nikitin
  gaincont=gaincont+sin(error/32768.0)*0.1;
  //debug
  //printf("gain: %g, value: %d\n",gaincont,mval);
  //bounds
  if(gaincont>20){
    gaincont=20;
  }
  if(gaincont<-20){
    gaincont=-20;
  }



}

  //checksum and data validation
  int bsize=-1;
  int chrsrx=-1;
  int rxcount=0;
void process_message(unsigned char* tbuff,int output){
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
                }
              }
            
          }else if(output%257==0){
            /*}else if(output>256){
              if(output%257!=0){
                int nonp=output/257;
                double decc=output;
                double dval=decc/257.0;
                double middle=nonp+0.5;
                if(dval>middle){
                  output=nonp+1;
                }else{
                  output=nonp;
                }
                


                
    
              }else{
                 output=output/257;
              }*/
            output=output/257;
            if(output!=0){
              if(position-1<=bsize&&position>1){

                if(strlen(tbuff)==bsize){
                    int bef=calculate_message_chrsum(tbuff,bsize);
                    unsigned char prev=tbuff[position-2];
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
                }else{
            //\r is backline
                  printf("received: %d/%d %d vs %d         \r",rxcount,bsize,chrsm,chrsrx);
                  fflush(stdout);
                }
              }else if(position==1){
                if(output<29)
                bsize=output;
              }
            }
      }
  }
  }

int main(int argn, char* argv[]){

  Modulator m=init_modulation_scheme(48000,21,500,6,1);
  //uplink: 17000, downlink: 24000
  create_receiver(48000,24000);

  char tbuff[29];
  bzero(tbuff,sizeof(char)*29);

  int size = calculate_frame_size(m,1,1);
  printf("initialized buffer size: %d\n",size);
  short frame[size];
  short frame2[size];
  bzero(frame,sizeof(short)*size);
  unsigned int itterator = 0;
  //create_header(frame, &itterator);
  int break_size=get_packet_size_buffer(m);
  printf("%d %d\n",break_size,size);
  //
  //
  if(setup_alsa("default",NULL,size,48000)<0)
    return 0;

  printf("alsa ready\n");
  printf("\n");


  int msgrx=0;

  int framegain=5000;


  while(msgrx==0){
    receive_signal(m,frame,size,framegain);

    while(itterator<=size){

      int mid=avgavg;
      if(wait_for_sync(m,frame,&itterator,size,mid + mid>>1)>-1){

          //printf("synced\n");

        if(size-itterator<break_size){
          int freesize=(size-itterator);//size left uncovered
          int treaded_size=itterator;//size covered


          memcpy(frame2,frame+itterator,sizeof(short)*freesize);
          memcpy(frame,frame2,sizeof(short)*freesize);
          receive_signal(m,frame2,treaded_size,framegain);
          memcpy(frame+freesize,frame2,sizeof(short)*treaded_size);

          //printf("%d\n",size-itterator);
          itterator=0;
        }
        //printf("%d\n",size-itterator);
        //demod2(frame,size,sqg);
        
        //itterator=size;
          int output=(int)demod(m,frame,&itterator,size);

          if(output>=0)
          process_message((unsigned char*)tbuff,output);
          
        }else{
           int freesize=(size-itterator);//size left uncovered
          int treaded_size=itterator;//size covered


          memcpy(frame2,frame+itterator,sizeof(short)*freesize);
          memcpy(frame,frame2,sizeof(short)*freesize);
          receive_signal(m,frame2,treaded_size,framegain);
          memcpy(frame+freesize,frame2,sizeof(short)*treaded_size);

          //printf("%d\n",size-itterator);
          itterator=0;

        }
      }

    
    itterator=0;
  }
    //printf("%g %d %d %d \n",cgain, success,success_past,mval);
  
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
