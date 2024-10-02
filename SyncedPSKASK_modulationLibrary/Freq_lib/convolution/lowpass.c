#include<stdlib.h>
#include<math.h>
#include "lowpass.h"
#include<stdio.h>

//Evan Nikitin Wed Oct  2 09:22:05 PM -00 2024 - self learning radio player

#define PI 3.141592653589793

double* convolution_buffer=NULL;
int convolution_size=0;
int* ringbuffer1=NULL;

void destroy_filter(){
	if(convolution_buffer!=NULL){
		free(convolution_buffer);
		convolution_buffer=NULL;
	}
	
	if(ringbuffer1!=NULL){
		free(ringbuffer1);
		ringbuffer1=NULL;
	}
}
double strength=30;

int shift=0;
void setup_low_pass(int freqhz,int sample_rate){
	destroy_filter();
	if(freqhz>sample_rate){
		return;
	}
	convolution_size=((sample_rate)/(freqhz))/2;
	if(convolution_size<3){
		return;//dont allocate memory, exit
	}
	convolution_buffer=malloc(sizeof(double)*(convolution_size));
	ringbuffer1=malloc(sizeof(int)*convolution_size);
	int i;
	double indexing=PI/((double)(convolution_size-1));
	double position=0;
	for(i=0;i<convolution_size;i++){
		ringbuffer1[i]=0;
		double value=(sin(position))*strength;

		
		convolution_buffer[i]=value;
		position=position+indexing;
	}
}
int bufferize(int input){
	//printf("%d\n",input);
	register int* curr_buff_ptr=ringbuffer1;
		register int i;
	register long sum;
	register long divresult;
	sum=0;
	if(convolution_buffer==NULL){
		return input;	
	}
	register int bound=convolution_size-1;
	register int imsum=0;
	register int b2=bound;
	for(i=0;i<bound;i++){
		divresult=(convolution_buffer[b2]*(curr_buff_ptr[i]));
		imsum=i+1;
		b2--;
		curr_buff_ptr[i]=curr_buff_ptr[imsum];
		sum=sum+divresult;
	}
	divresult=(convolution_buffer[0]*(curr_buff_ptr[i]));
	curr_buff_ptr[bound]=input;
	sum=sum+divresult;
	return (sum/(convolution_size<<4));
}

void filter_buffer(short* buffer, int buffer_size){
	register short perm=0;
	register int result;
  unsigned int i2;
	for(i2=0;i2<buffer_size;i2++){
		perm=buffer[i2];
		result=bufferize(perm);
		//result=perm;
		buffer[i2]=result;
		//printf("%d %d \n",r2,second);
	}
}
