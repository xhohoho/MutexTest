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
osThreadDef(x_Thread1, osPriorityNormal, 1, 0);
osThreadDef(x_Thread2, osPriorityNormal, 1, 0);
osThreadDef(x_Thread3, osPriorityNormal, 1, 0);

osThreadId T_x1;
osThreadId T_x2;
osThreadId T_x3;

osMessageQId bufferdata_que;
osMessageQDef (bufferdata_que,0xff,unsigned char);
osEvent  result;

osMutexId x_mutex;
osMutexDef(x_mutex);
osSemaphoreId item_semaphore;                         // Semaphore ID
osSemaphoreDef(item_semaphore);                       // Semaphore definition
osSemaphoreId slot_semaphore;                         // Semaphore ID
osSemaphoreDef(slot_semaphore);                       // Semaphore definition

long int x=0;
long int i=0;
long int j=0;

const unsigned int N = 4;
unsigned char buffer[N];
unsigned int insertPtr = 0;
unsigned int removePtr = 0;
unsigned char bufferdata;
unsigned char buffer0;
unsigned char buffer1;
unsigned char buffer2;
unsigned char buffer3;

void put(unsigned char a_burger){
	osSemaphoreWait(slot_semaphore, osWaitForever);
	osMutexWait(x_mutex, osWaitForever);
	buffer[insertPtr] = a_burger;
	buffer0=buffer[0];
	buffer1=buffer[1];
	buffer2=buffer[2];
	buffer3=buffer[3];
	insertPtr = (insertPtr + 1) % N;
	osMutexRelease(x_mutex);
	osSemaphoreRelease(item_semaphore);
}

unsigned char get(){
	unsigned int removed = 0;
	osSemaphoreWait(item_semaphore, osWaitForever);
	osMutexWait(x_mutex, osWaitForever);
	bufferdata = buffer[removePtr];
	buffer[removePtr]=removed;
	buffer0=buffer[0];
	buffer1=buffer[1];
	buffer2=buffer[2];
	buffer3=buffer[3];
	removePtr = (removePtr + 1) % N;
	osMutexRelease(x_mutex);
	osSemaphoreRelease(slot_semaphore);
	return bufferdata;
}

void x_Thread1 (void const *argument) 
{
	//cheff
	unsigned char burger = 0x41;	//char 'A'
	for(;;){
		put(burger++);
	}
}

void x_Thread2 (void const *argument) 
{
	//consumer (waiter #1)
	unsigned int data;
	for(;;){
		data = get();
		//SendChar(data);
		osMessagePut(bufferdata_que,data,osWaitForever);             //Place a value in the message queue
	}
}

void x_Thread3(void const *argument)
{
	//cashier
	//osMessagePut(bufferdata_que,0x30,osWaitForever);
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
	slot_semaphore = osSemaphoreCreate(osSemaphore(slot_semaphore), N);
	x_mutex = osMutexCreate(osMutex(x_mutex));	
	
	bufferdata_que = osMessageCreate(osMessageQ(bufferdata_que),NULL);					//create the message queue
	
	T_x1 = osThreadCreate(osThread(x_Thread1), NULL);//chef
	T_x2 = osThreadCreate(osThread(x_Thread2), NULL);//waiter1
	T_x3 = osThreadCreate(osThread(x_Thread3), NULL);//casher
 
	osKernelStart ();                         // start thread execution 
}
