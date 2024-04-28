#include <stdint.h>
#include "PLL.h"
#include "SysTick.h"
#include "uart.h"
#include "onboardLEDs.h"
#include "tm4c1294ncpdt.h"
#include "VL53L1X_api.h"
#include "initialise.h"

#define CW 1
#define CCW 0
#define SAMPLERATE 32

int start = 0;
int numberOfScans = 3;
int data[SAMPLERATE];
int delay = 133333;
int validData = 0;


uint16_t wordData;
uint8_t ToFSensor = 1; // 0=Left, 1=Center(default), 2=Right
uint16_t Distance;
uint16_t SignalRate;
uint16_t AmbientRate;
uint16_t SpadNum; 
uint8_t RangeStatus;


char motorSteps[] = {0b00000011, 0b00000110, 0b00001100, 0b00001001};

void spinMotor(int direction){

	if (direction == CW){

		// 360/11.25 = 32
		
		for (int m = 0 ; m < 8; m++){

			// used to trigger motor poles
			for (int i = 0; i < 4; i++){

				GPIO_PORTH_DATA_R = motorSteps[i];
				SysTick_Wait(delay);

			}
		}
		FlashLED2(1);
		SysTick_Wait10ms(350);


	} else {
			
		for (int j = 0; j < 512; j++) {
			for (int k = 4; k >= 0; k--) {
				GPIO_PORTH_DATA_R = motorSteps[k];
				SysTick_Wait(delay);
			}
				
		}
	}

}



//*********************************************************************************************************
//*********************************************************************************************************
//***********					MAIN Function				*****************************************************************
//*********************************************************************************************************
//*********************************************************************************************************
uint16_t	dev = 0x29;			//address of the ToF sensor as an I2C slave peripheral
int status=0;
uint16_t ToFState = 0;

void getData(int i){

	while (validData == 0)
	{
		status = VL53L1X_CheckForDataReady(dev, &validData);
		VL53L1_WaitMs(dev, 2);
	}

	validData = 0;
	status = VL53L1X_GetRangeStatus(dev, &RangeStatus);	
	status = VL53L1X_GetDistance(dev, &Distance);
	data[i] = Distance;
	sprintf(printf_buffer,"%u\r\n", data[i]);
	UART_printf(printf_buffer);


}

void moveForward(void){
	SysTick_Wait10ms(750);
	FlashLED3(1);
	SysTick_Wait10ms(750);
	FlashLED3(1);
	SysTick_Wait10ms(750);
	FlashLED3(1);
	SysTick_Wait10ms(750);
	FlashLED3(1);
}


void GPIOJ_IRQHandler(void){
	FlashLED2(1);
	GPIO_PORTJ_ICR_R = 0x02;


    for (int i = 0; i < numberOfScans; i++) {
        for (int j = 0; j < 64; j++) {
            if (start) { // Only execute if Python script is running
				
				if (j%2 == 0){

					getData(j);
				}
                spinMotor(CW);
            }
        }
        if (start) { // Only execute if Python script is running
            spinMotor(CCW);
			moveForward();
        }
    }

}

// char string[24] = "\nSharvin Soosaipillai\r\n";

int main(void) {

	//initialize
	PLL_Init();	
	PortH_Init();
	PortN_Init();
	SysTick_Init();
	onboardLEDs_Init();
	I2C_Init();
	UART_Init();
	PortJ_Init();
	PortJ_Interrupt_Init();
	
	char j;
	int input = 0;


	while (ToFState == 0){
		status = VL53L1X_BootState(dev, &ToFState);
		SysTick_Wait10ms(300);
	}

	FlashLED2(1);


	status = VL53L1X_ClearInterrupt(dev); /* clear interrupt has to be called to enable next interrupt*/
  	status = VL53L1X_SensorInit(dev);
	Status_Check("SensorInit", status);
	
	status = VL53L1X_StartRanging(dev);   /* This function has to be called to enable the ranging */
	Status_Check("StartRanging", status);
	while(1){		
		//wait for the right transmition initiation code
		while(1){
			input = UART_InChar();
			if (input == 's')
				start = 1;
				break;
		}

		// check the bus speed
		// Bus_speed();


		// UART STUFF
		// UART_OutChar('A');
		// UART_OutString(string);
		// SysTick_Wait10ms(400);

	}

	
}