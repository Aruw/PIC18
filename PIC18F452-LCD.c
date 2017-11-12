/* -----------------------------------------------------------------------------
/* * * * * * * * * * *          DISPLAY DE LCD            * * * * * * * * * * *
   -----------------------------------------------------------------------------

	 Exemplos de comandos do display de LCD.

// Programa associado � placa de simula��o McLab2.

// PIM - CST em Automa��o Industrial - UNIP - 2� sem. de 2014
// Disciplina: Microprocessadores e Microcontroladores

// Autor: prof. Giovanni Rizzo

*/

// ----------------------------------------------------------------------------------
// Inclus�o dos arquivos de cabe�alho ( header files ).

#include <P18F452.h>		// Define o microcontrolador utilizado: PIC18F452

#include <delays.h>			// Fun��es de delay.

#include <stdio.h>			// Arquivo header que cont�m a rotina sprintf().

#include "lcd.h"			// Rotinas para comunica��o com o display de LCD.

// ------------------------------------------------------------------------------
// Configura��o dos fuses ( configuration bits ) do microcontrolador.

#pragma config OSC=	XT		// Oscilador do clock no modo XT.
#pragma config WDT=	OFF		// Watchdog-Timer desabilitado.
#pragma config PWRT=ON		// Power-Up Timer habilitado.
#pragma config BORV=	45  // Reset para condi��o de Brown-Out.
#pragma config LVP=	OFF		// Low-Voltage Programming desabilitado.

// ------------------------------------------------------------------------------
// Defini��es.
void main( void )
{
	int contador= 0;
	char strtemp[ 32 ];

	ADCON1=	0b00000111;					// Pinos em modo digital.

	TRISD=	0b00000000;					// Pinos do PORTD s�o sa�das ( para enviar dados ao display ).
	LATD=	0b00000000;

	TRISE=	0b00000100;					// Pinos RE0 e RE1 s�o sa�das. ( RS e EN ).
	LATE=	0b00000000;

	lcd_init();							// Inicializa o display.


	lcd_cmd( CUR_LINE1 );								// Posiciona o cursor no canto superior esquerdo.
	lcd_rom_puts( "EXEMPLO USANDO O DISPLAY DE LCD." );	// Imprime a mensagem.

	Delay10KTCYx( 100 );								// Delay de 1 segundo.

	for ( contador= 0; contador < 15; contador++ )		// Desloca 13 vezes para a esquerda.
	{
		Delay10KTCYx( 20 );								// Com um delay de 0.2 s a cada ...
		lcd_msg_shl();									// ... deslocamento � esquerda.
	}

	Delay10KTCYx( 100 );								// Atraso de 1 segundo.
	
	lcd_clear();										// Limpa o display.

	contador= 0;									// Zera o contador.


	lcd_cmd( CUR_LINE1 );							// Cursor no in�cio da linha 1.
	lcd_rom_puts( "Ex. display LCD" );				// Envia texto para o display.


	do													// Mostra como converter n�mero para o display
	{													// usando a fun��o sprintf().
		lcd_cmd( CUR_LINE2 );							// Cursor no in�cio da linha 2.
		sprintf( strtemp, "Contador= %d", contador++ );	// Insere o valor do n�mero na string strtemp.
		lcd_ram_puts( strtemp );						// Envia texto para o display.

		Delay1KTCYx( 150 );								// Delay de 0.15 segundo.

	} while ( contador <= 100 );							// Conta at� 100.

	Delay10KTCYx( 50 );

	lcd_clear();										// Limpa o display.

	lcd_cmd( CUR_LINE1 + 3 );							// Posiciona cursor na 5� coluna da linha 1.
	lcd_rom_puts( "AUTOMACAO" );

	lcd_cmd( CUR_LINE2 + 4 );							// Posiciona cursor na 5� coluna da linha 2.
	lcd_rom_puts( "INDUSTRIAL" );
	
	// Desloca a mensagem para a direita e para a esquerda 20 vezes.
	for ( contador= 0; contador < 20; contador++ )
	{
		Delay10KTCYx( 30 );								// Delay de 0.3 segundo.
		lcd_msg_shl();									// Desloca todo o conte�do para a esquerda.
		Delay10KTCYx( 30 );								// Delay de 0.3 segundo.
		lcd_msg_shr();									// Desloca todo o conte�do para a direita.
	}

	lcd_cmd( CURSOR_ONP );		// Habilita cursor piscante.

	Delay10KTCYx( 250 );		// Atraso de 5 segundos.
	Delay10KTCYx( 250 );


	Reset();					// Resseta o microcontrolador.
}