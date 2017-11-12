/*
	* * * * * * * * * * * * * * * EXEMPLO DE CONTADOR DE PULSOS * * * * * * * * *

	Programa desenvolvido para a placa McLab2.
	Disciplina: Microprocessadores e Microcontroladores.
	Curso:		CST em Automa��o Industrial.
	Professor:  Giovanni Rizzo
*/

#include <P18F452.h>						//	Microcontrolador utilizado.

#include <delays.h>							//	Rotinas de delay.

#include <timers.h>							//	Defini��es e rotinas dos timers.

#include <stdio.h>							//	Rotina sprintf().

#include "lcd.h"							//	Defini��es e rotinas do display de LCD.

#define	SENSOR	PORTBbits.RB0				//	"Sensor" representado pelo bot�o em RB0.
#define	LED		LATBbits.LATB3				//	LED.

#define	BTRESET	PORTBbits.RB1				//	Bot�o de Reset da contagem no pino RB1.

#define	delayT0	( 65536 - 500 )				//	Per�odo de 500 us para o Timer0.

char testa_sensor( void );					//	Rotina de teste do sensor.

volatile unsigned int contador= 0;			//	Contador de pe�as.

void main( void )							//	Rotina principal do programa.
{
	char strtemp[ 100 ];					//	String para armazenar mensagens enviadas ao display.

	ADCON1=	0b00000111;						//	Pinos de I/O no modo digital.

	LED=	0;								//	LED incialmente apagado.
	TRISB=	0b11110111;						//	RB3 � sa�da ( LED ).

	LATE=	0x00;							//	Pinos EN e RS s�o sa�das ( controle do display ).
	TRISE=	0b00000100;

	LATD=	0x00;							//	PORTD � sa�da ( para enviar dados ao display ).
	TRISD=	0x00;

	// Inicializa o Timer0 com fonte de incremento interna ( clock do sistema ), prescaler de 1:1, com 16 bits.
	OpenTimer0( T0_SOURCE_INT & T0_PS_1_1 & T0_16BIT );

	// Valor inicial da contagem de tempo.
	WriteTimer0( delayT0 );

	INTCONbits.TMR0IF=	0;					//	Flag de interrup��o do Timer0 inicialmente desabilitada.
	INTCONbits.TMR0IE=	1;					//	Interrup��o do Timer0 habilitada.
	RCONbits.IPEN=		0;					//	Interrup��es sem n�vel de prioridade.

	INTCONbits.GIE=		1;					//	Habilita a chave geral de interrup��es.

	lcd_init();								//	Inicializa o display de LCD.

	lcd_cmd( CUR_LINE1 );					//	Posiciona o cursor no in�cio da linha 1.
	lcd_rom_puts( "CONTAGEM PECAS: " );		//	Escreve a mensagem na tela.

	while ( 1 )								//	Loop infinito principal ( Mainloop ).
	{		
		// Escreve na string strtemp o valor da contagem.
		sprintf( strtemp, "Contador = %d     ", contador );

		lcd_cmd( CUR_LINE2 );				//	Posiciona o cursor no in�cio da linha 2.
		lcd_ram_puts( strtemp );			//	Escreve a mensagem na tela ( proveniente da string ).

		if ( BTRESET == 0 )					//	Se o bot�o Reset foi pressionado:
		{
			contador= 0;					//	zera o contador.
		}

		Delay100TCYx( 1 );					// Delay de 0.1 milissegundo.
		
	}
	
}

// --------------------------------------------------------------------------------------------------
//	Rotina do teste do sensor.
#define	FILTRO	30							//	N�mero leituras consecutivas para habilitar o bot�o.
char testa_sensor( void )
{
	static unsigned char contador_filtro=	FILTRO;	// Contador de leituras.

	if ( SENSOR == 0 )						//	Se o sensor detectou pe�a ( bot�o pressionado )...
	{
		if ( contador_filtro == 0 )			//	Se o contador de leituras estiver zerado:
		{
			contador_filtro= FILTRO;		//	Reinicia o contador.
			return 1;						//	Retorna verdadeiro.
		}

		contador_filtro= FILTRO;			//	Reinicia o contador o bot�o est� pressionado.
	}
	else									//	Se o sensor n�o detectou pe�a:
	{
		if ( contador_filtro )				//	Se o contador de leituras for superior a zero:
		{
			contador_filtro--;				//	� decrementado.
		}
	}

	LED=	contador_filtro;				//	LED aceso se e somente se contador diferente de zero.
											//  Acende quando uma pe�a sai do alcance do sensor, mos-
											//	trando prontid�o para detec��o de uma nova pe�a.

	return 0;								//	Retorna falso.
}

// ---------------------------------------------------------------------------------------------------
// Rotina de tratamento de interrup��o.

#pragma code INTERRUPCAO=	0x00008
#pragma interrupt	INTERRUPCAO
void INTERRUPCAO( void )
{
	if ( INTCONbits.TMR0IF )				//	Se a interrup��o foi gerada pelo Timer0:
	{
		WriteTimer0( delayT0 );				//	Reinicia a contagem de tempo.

		if ( testa_sensor() )				//	Se h� uma nova pe�a:
		{
			contador++;						//	incrementa o contador.
		}

		INTCONbits.TMR0IF=	0;				//	Resseta o flag de ocorr�ncia de interrup��o.
	}
}
#pragma code

// ---------------------------------------------------------------------------------------------------

