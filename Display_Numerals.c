#include <P18F452.h>
#include <delays.h>

#pragma config OSC = XT;
#pragma config WDT = OFF;
#pragma config LVP = OFF;

const rom char vetor [] = {
	0b00111111,					//0
	0b00000110,					//1
	0b01011011,					//2
	0b01001111,					//3
	0b01100110,					//4
	0b01101101,					//5
	0b01111101,					//6
	0b00000111,					//7
	0b01111111,					//8
	0b01101111					//9
};

void main (void){

	char milhar = 1;
	char centena = 3;
	char dezenas = 3;
	char unidade = 7;
	char mascara = 0b00010000;
	
	ADCON1 = 0b00000111;
	TRISB = 0b00001111;
	TRISD = 0x00;

	while(1){

		LATD = 0x00;		//Apaga o display
		LATB = 0x00;		//Transistor em corte 
		Delay10TCYx(1);		//Atraso de 10 microsegundos
		
		switch(mascara){	//Qual display está habilitado?
			case 0b00100000://Se é RB5 (Dezena)
				LATD = vetor[dezena];
				break;			
			case 0b01000000://Se é RB6 (Centena)
				LATD = vetor[centena];
				break;
			case 0b10000000://Se é RB7 (Milhar)
				LATD = vetor[milhar];
				break;
		}
 
		LATB = mascara;		//Habilita o display correspondente.
		mascara <<= 1;		//Desloca um bit à direita.
		
		if(mascara == 0){	//Não está mais selecionando nenhum display
			mascara = 0b00010000;	//Nesse caso volta a habilitar a unidade
			Deley1KTCYx(1);			//Atraso de 1 milisegundo.
		}		
	
	}	
}
