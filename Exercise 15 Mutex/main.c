/*----------------------------------------------------------------------------
	
	Designers Guide to the Cortex-M Family
	CMSIS RTOS Mutex Example

*----------------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "cmsis_os.h"
#include "uart.h"

void x_Thread1 (void const *argument);
void x_Thread2 (void const *argument);
void x_Thread3 (void const *argument);
void x_Thread4 (void const *argument);
osThreadDef(x_Thread1, osPriorityNormal, 1, 0);
osThreadDef(x_Thread2, osPriorityNormal, 1, 0);
osThreadDef(x_Thread3, osPriorityNormal, 1, 0);
osThreadDef(x_Thread4, osPriorityNormal, 1, 0);

osThreadId T_x1;
osThreadId T_x2;
osThreadId T_x3;
osThreadId T_x4;

osMessageQId bufferdata_que;
osMessageQDef (bufferdata_que,0x16,unsigned char);
osEvent  result;

osMutexId x_mutex;
osMutexDef(x_mutex);
osSemaphoreId item_semaphore;                         // Semaphore ID
osSemaphoreDef(item_semaphore);                       // Semaphore definition
osSemaphoreId space_semaphore;                         // Semaphore ID
osSemaphoreDef(space_semaphore);                       // Semaphore definition

long int x=0;
long int i=0;
long int j=0;
long int k=0;

const unsigned int N = 4;
unsigned char buffer[N];
unsigned int insertPtr = 0;
unsigned int removePtr = 0;
unsigned char bufferdata;
unsigned char buffer0;
unsigned char buffer1;
unsigned char buffer2;
unsigned char buffer3;

void put(unsigned char an_item){
	osSemaphoreWait(space_semaphore, osWaitForever);
	osMutexWait(x_mutex, osWaitForever);
	buffer[insertPtr] = an_item;
	buffer0=buffer[0];
	buffer1=buffer[1];
	buffer2=buffer[2];
	buffer3=buffer[3];
	insertPtr = (insertPtr + 1) % N;
	osMutexRelease(x_mutex);
	osSemaphoreRelease(item_semaphore);
}

unsigned char get(){
	unsigned int rr = 0;
	osSemaphoreWait(item_semaphore, osWaitForever);
	osMutexWait(x_mutex, osWaitForever);
	bufferdata = buffer[removePtr];
	buffer[removePtr]=rr;
	buffer0=buffer[0];
	buffer1=buffer[1];
	buffer2=buffer[2];
	buffer3=buffer[3];
	removePtr = (removePtr + 1) % N;
	osMutexRelease(x_mutex);
	osSemaphoreRelease(space_semaphore);
	return bufferdata;
}

int loopcount = 20;

void x_Thread1 (void const *argument) 
{
	//producer
	unsigned char item = 0x41;
	for(; i<loopcount; i++){
		put(item++);
	}
}

void x_Thread2 (void const *argument) 
{
	//consumer (waiter #1)
	unsigned int data = 0x00;
	for(; j<loopcount; j++){
		data = get();
		//SendChar(data);
		osMessagePut(bufferdata_que,data,osWaitForever);             //Place a value in the message queue
	}
}

void x_Thread3 (void const *argument) 
{
	//consumer (waiter #2)
	unsigned int data2 = 0x00;
	for(; k<loopcount; k++){
		data2 = get();
		//SendChar(data2);
		osMessagePut(bufferdata_que,data2,osWaitForever);             //Place a value in the message queue
	}
}

void x_Thread4(void const *argument)
{
	//cashier
	for(;;){
		result = 	osMessageGet(bufferdata_que,osWaitForever);				//wait for a message to arrive
		SendChar(result.value.v);
	}
}

int main (void) 
{
	osKernelInitialize ();                    // initialize CMSIS-RTOS
	USART1_Init();
	item_semaphore = osSemaphoreCreate(osSemaphore(item_semaphore), 0);
	space_semaphore = osSemaphoreCreate(osSemaphore(space_semaphore), N);
	x_mutex = osMutexCreate(osMutex(x_mutex));	
	
	bufferdata_que = osMessageCreate(osMessageQ(bufferdata_que),NULL);					//create the message queue
	
	T_x1 = osThreadCreate(osThread(x_Thread1), NULL);//producer
	T_x2 = osThreadCreate(osThread(x_Thread2), NULL);//consumer
	T_x3 = osThreadCreate(osThread(x_Thread3), NULL);//another consumer
	T_x4 = osThreadCreate(osThread(x_Thread4), NULL);//casher
 
	osKernelStart ();                         // start thread execution 
}
