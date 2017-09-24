/*
	Experiência com varredura dos displays de 7
	segmentos na placa McLab2 com o PIC18F452.
	Compilador utilizado: C18.
	Autor: Prof. Giovanni Rizzo Junior.
*/


#include <P18F452.h> 				//Define o microcontrolador utilizado.
#include <delays.h>					//Rotinas de delays.

#pragma config OSC = XT 			//Oscilador em modo XT
#pragma config WDT = OFF			//Watchdog-Time desabilitado.
#pragma config LVP = OFF			//Modo LVP desligado, para utilizar o pino RB5.

//VETOR de correspondencia entre algarismos decimais e código para display de 7 segmentos.

const rom char vetor[]={
	0b00111111,						//Dígito 0
	0b00000110,						//Dígito 1
	0b01011011,						//Dígito 2
	0b01001111,						//Dígito 3
	0b01100110,						//Dígito 4
	0b01101101,						//Dígito 5
	0b01111101,						//Dígito 6
	0b00000111,						//Dígito 7
	0b01111111,						//Dígito 8
	0b01100111,						//Dígito 9
};

void main (void){ 					//Rotina principal do programa 
	
	//variavel para seleção do display
	char display = 0b00010000;
	
	//Variaveis com os digitos decimais 
	char unidade, dezena, centena, milhar;

	//Número de 4 algarismos a ser exibido 
	int numero = 2017;

	//Pinos em modo digital 
	ADCON1 = 0b00000111;

	LATD = 0X00;	//dISPLAYS INICIALMENTE APAGADOS	
	LATB = 0x00; 	//Display inicialmente desabilitados

	//Pinos de habilitação configurados como saida
	TRISB = 0b00001111;	