/*
	Experi�ncia com varredura dos displays de 7
	segmentos na placa McLab2 com o PIC18F452.
	Compilador utilizado: C18.
	Autor: Prof. Giovanni Rizzo Junior.
*/


#include <P18F452.h> 				//Define o microcontrolador utilizado.
#include <delays.h>					//Rotinas de delays.

#pragma config OSC = XT 			//Oscilador em modo XT
#pragma config WDT = OFF			//Watchdog-Time desabilitado.
#pragma config LVP = OFF			//Modo LVP desligado, para utilizar o pino RB5.

//VETOR de correspondencia entre algarismos decimais e c�digo para display de 7 segmentos.

const rom char vetor[]={
	0b00111111,						//D�gito 0
	0b00000110,						//D�gito 1
	0b01011011,						//D�gito 2
	0b01001111,						//D�gito 3
	0b01100110,						//D�gito 4
	0b01101101,						//D�gito 5
	0b01111101,						//D�gito 6
	0b00000111,						//D�gito 7
	0b01111111,						//D�gito 8
	0b01100111,						//D�gito 9
};

void main (void){ 					//Rotina principal do programa 
	
	//variavel para sele��o do display
	char display = 0b00010000;
	
	//Variaveis com os digitos decimais 
	char unidade, dezena, centena, milhar;

	//N�mero de 4 algarismos a ser exibido 
	int numero = 2017;

	//Pinos em modo digital 
	ADCON1 = 0b00000111;

	LATD = 0X00;	//DISPLAYS INICIALMENTE APAGADOS	
	LATB = 0x00; 	//Display inicialmente desabilitados

	//Pinos de habilita��o configurados como saida
	TRISB = 0b00001111;	
	
	TRISD = 0x00;	//PORTD configurado como sa�da

	//Obt�m os 4 algarismos a serem exibidos.

	unidade = numero % 10;	//Resto da divis�o por 10.
	numero /= 10;
	dezena = numero % 10;
	numero /= 10;
	centena = numero %10;
	milhar = numero /10;

	while{			//La�o principal
		LATD = 0x00;	//Apaga o display selecionado.
		LATB = 0x00; 	//Divis�o inteira por 10.

		//Define o valor enviado para o display
		//Conforme o display selecionado.

		switch(display){
			case 0b00010000:
				LATD = vetor[unidade];
				break;
			case 0b00100000:
				LATD = vetor[dezena];
				break;
			case 0b01000000:
				LATD = vetor[centena];
				break;
			case 0b10000000:
				LATD = vetor[milhar];
				break;
		}

	//Envia o c�digo de sele��o para o PORTB.
	LATB = display;

	display <<= 1;		//Seleciona o pr�ximo display
	
	//Se j� selecionou o �ltimo, seleciona o primeiro 
	if(display == 0){
		display = 0b00010000;
	}

	//Atraso de 1ms.
	Delay1KTCYx(1);
	}
}
