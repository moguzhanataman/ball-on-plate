#include "serial.h"

bool init_serial(){
	
	char mode[]={'8','N','1',0};
	
	if(RS232_OpenComport(COM_PORT, BD_RATE, mode))
			
		return false;
	
	RS232_flushRX(COM_PORT);

	return true;	
}

bool readBuf(char *buf, int size){
	
	int n=0;
	
	while(n != size){
		
		n += RS232_PollComport(COM_PORT, ((unsigned char*)buf)+n, 1);
		
		if(n == 0)
			return false;
	}
	
	return true;
}

bool sendBuf(char *buf, int size){
		
	if(RS232_SendBuf(COM_PORT, (unsigned char*)buf, size) == -1)
	
		return false;
		
	RS232_flushTX(COM_PORT);
	
	return true;
}

bool sendPID(float* pid){
	//TODO ayrı ayrı yollanabilir?
	int buf_size = sizeof(float)*6 + 1;
	char buf[buf_size] = {1};
	
	memcpy(buf+1, pid, buf_size-1);
	
	return sendBuf(buf, buf_size);
}

bool sendSetpoints(float x, float y){
	
	int buf_size = 9;
	char buf[buf_size]={0};
		
	memcpy(buf+1, &x, 4);
	memcpy(buf+5, &y, 4);
	
	return sendBuf(buf, buf_size);
}

bool getCoordinates(int16_t* x, int16_t* y, float* servo_x, float* servo_y){

	int buf_size = 12;
	char buf[buf_size]; 
	
	if(readBuf(buf, buf_size) == false)
		return false;
	
	memcpy(x, buf,2);
	memcpy(y, buf+2,2);
	memcpy(servo_x, buf+4,4);
	memcpy(servo_y, buf+8,4);
		
	return true;	
}
