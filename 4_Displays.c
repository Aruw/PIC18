#include <P18F452.h>
#include <delays.h>

#pragma config OSC = XT
#pragma config +WDT = OFF
#pragma config LVP = OFF

void main(void){

	ADCON1 = 0b00000111;
	LATD = 0x00;
	LATB = 0b01000000;			//Habilita 1 display
	TRISD = 0x00;
	TRISB = 0b00001111;
	
	while(1){
		LATD = 0b01011011;	//Digito 2
		Delay1KTCYx(80);		// 80% do tempo ligado (8ms)
		LATD = 0x00;			  //Apaga
		Delay1KTCYx(2);			//20% do tempo desligado (2ms)
	}
}
