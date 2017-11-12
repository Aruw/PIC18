/* -----------------------------------------------------------------------------
/* * * * * * * * * * *          DISPLAY DE LCD            * * * * * * * * * * *
   -----------------------------------------------------------------------------

	 Exemplos de comandos do display de LCD.

// Programa associado à placa de simulação McLab2.

// PIM - CST em Automação Industrial - UNIP - 2º sem. de 2014
// Disciplina: Microprocessadores e Microcontroladores

// Autor: prof. Giovanni Rizzo

*/

// ----------------------------------------------------------------------------------
// Inclusão dos arquivos de cabeçalho ( header files ).

#include <P18F452.h>		// Define o microcontrolador utilizado: PIC18F452

#include <delays.h>			// Funções de delay.

#include <stdio.h>			// Arquivo header que contém a rotina sprintf().

#include "lcd.h"			// Rotinas para comunicação com o display de LCD.

// ------------------------------------------------------------------------------
// Configuração dos fuses ( configuration bits ) do microcontrolador.

#pragma config OSC=	XT		// Oscilador do clock no modo XT.
#pragma config WDT=	OFF		// Watchdog-Timer desabilitado.
#pragma config PWRT=ON		// Power-Up Timer habilitado.
#pragma config BORV=	45  // Reset para condição de Brown-Out.
#pragma config LVP=	OFF		// Low-Voltage Programming desabilitado.

// ------------------------------------------------------------------------------
// Definições.
void main( void )
{
	int contador= 0;
	char strtemp[ 32 ];

	ADCON1=	0b00000111;					// Pinos em modo digital.

	TRISD=	0b00000000;					// Pinos do PORTD são saídas ( para enviar dados ao display ).
	LATD=	0b00000000;

	TRISE=	0b00000100;					// Pinos RE0 e RE1 são saídas. ( RS e EN ).
	LATE=	0b00000000;

	lcd_init();							// Inicializa o display.


	lcd_cmd( CUR_LINE1 );								// Posiciona o cursor no canto superior esquerdo.
	lcd_rom_puts( "EXEMPLO USANDO O DISPLAY DE LCD." );	// Imprime a mensagem.

	Delay10KTCYx( 100 );								// Delay de 1 segundo.

	for ( contador= 0; contador < 15; contador++ )		// Desloca 13 vezes para a esquerda.
	{
		Delay10KTCYx( 20 );								// Com um delay de 0.2 s a cada ...
		lcd_msg_shl();									// ... deslocamento à esquerda.
	}

	Delay10KTCYx( 100 );								// Atraso de 1 segundo.
	
	lcd_clear();										// Limpa o display.

	contador= 0;									// Zera o contador.


	lcd_cmd( CUR_LINE1 );							// Cursor no início da linha 1.
	lcd_rom_puts( "Ex. display LCD" );				// Envia texto para o display.


	do													// Mostra como converter número para o display
	{													// usando a função sprintf().
		lcd_cmd( CUR_LINE2 );							// Cursor no início da linha 2.
		sprintf( strtemp, "Contador= %d", contador++ );	// Insere o valor do número na string strtemp.
		lcd_ram_puts( strtemp );						// Envia texto para o display.

		Delay1KTCYx( 150 );								// Delay de 0.15 segundo.

	} while ( contador <= 100 );							// Conta até 100.

	Delay10KTCYx( 50 );

	lcd_clear();										// Limpa o display.

	lcd_cmd( CUR_LINE1 + 3 );							// Posiciona cursor na 5º coluna da linha 1.
	lcd_rom_puts( "AUTOMACAO" );

	lcd_cmd( CUR_LINE2 + 4 );							// Posiciona cursor na 5º coluna da linha 2.
	lcd_rom_puts( "INDUSTRIAL" );
	
	// Desloca a mensagem para a direita e para a esquerda 20 vezes.
	for ( contador= 0; contador < 20; contador++ )
	{
		Delay10KTCYx( 30 );								// Delay de 0.3 segundo.
		lcd_msg_shl();									// Desloca todo o conteúdo para a esquerda.
		Delay10KTCYx( 30 );								// Delay de 0.3 segundo.
		lcd_msg_shr();									// Desloca todo o conteúdo para a direita.
	}

	lcd_cmd( CURSOR_ONP );		// Habilita cursor piscante.

	Delay10KTCYx( 250 );		// Atraso de 5 segundos.
	Delay10KTCYx( 250 );


	Reset();					// Resseta o microcontrolador.
}