/* -----------------------------------------------------------------------------
/* * * * * * * * * * *          DISPLAY DE LCD            * * * * * * * * * * *
   -----------------------------------------------------------------------------

	 Exemplos de comandos do display de LCD.

// Programa associado à placa de simulação McLab2.

// Cursos de Engenharia e Tecnologia - UNIP - 2º sem. de 2016
// Disciplina: Microprocessadores e Microcontroladores

// Autor: prof. Giovanni Rizzo

*/

// ----------------------------------------------------------------------------------
// Inclusão dos arquivos de cabeçalho ( header files ).

#include <P18F452.h>		// Define o microcontrolador utilizado: PIC18F452

#include <delays.h>			// Funções de delay.

#include <timers.h>			// Rotinas de configuração/controle dos timers.

#include <stdio.h>			// Arquivo header que contém a rotina sprintf().

#include "lcd.h"			// Rotinas para comunicação com o display de LCD.

// ------------------------------------------------------------------------------
// Configuração dos fuses ( configuration bits ) do microcontrolador.

#pragma config OSC=	XT		// Oscilador do clock no modo XT.
#pragma config WDT=	ON		// Watchdog-Timer habilitado.
#pragma config PWRT=ON		// Power-Up Timer habilitado.
#pragma config BOR=	ON		// Brown-Out Reset habilitado.
#pragma config BORV= 45		// Reset para condição de Brown-Out.
#pragma config LVP=	OFF		// Low-Voltage Programming desabilitado.

// ------------------------------------------------------------------------------
// Definições.

#define	delayT0	( 65536 - 25000 )	// Interrupção do timer a cada 25 milissegundos.

void isr( void );					// Protótipo da rotina de tratamento de 
									// interrupção.

void main( void )					// Função principal do programa.
{
	int contador= 0;
	char strtemp[ 32 ];

	ADCON1=	0b00000111;					// Pinos em modo digital.

	TRISB=	0b11111100;					// Habilita LEDs em RB0 e RB1.
	LATB=	0x00;

	TRISD=	0b00000000;					// Pinos do PORTD são saídas ( para enviar dados ao display ).
	LATD=	0b00000000;

	TRISE=	0b00000100;					// Pinos RE0 e RE1 são saídas. ( RS e EN ).
	LATE=	0b00000000;

	LATBbits.LATB0= 1;					// Acende LED em RB0.	

	while ( RCONbits.NOT_TO );			// Aguarda um reset por time-out do watchdog-timer.

	LATBbits.LATB1= 1;					// Acende LED em RB1.

	WriteTimer0( delayT0 );						// Inicializa o Timer0.
	OpenTimer0( T0_SOURCE_INT & T0_16BIT );		// Fonte de interrupção interna, 16 bits.

	
	INTCONbits.TMR0IE=	1;				// Habilita interrupção do timer0.
	INTCONbits.TMR0IF=	0;				// Resseta o flag de interrupçao do timer0.

	RCONbits.IPEN=		0;				// Sem níveis de prioridade de interrupção.

	INTCONbits.GIE=		1;				// Liga chave geral de interrupções.

	lcd_init();							// Invoca a rotina de inicialização do display.

	while ( 1 )							// Loop principal.
	{
		lcd_clear();					// Limpa o display.

		lcd_cmd( CURSOR_OFF );			// Desliga o cursor.


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
	
		Delay10KTCYx( 50 );									// Delay de 0.5 segundo.
	
		lcd_clear();										// Limpa o display.
	
		lcd_cmd( CUR_LINE1 + 3 );							// Posiciona cursor na 5º coluna da linha 1.
		lcd_rom_puts( "ENGENHARIA" );
	
		lcd_cmd( CUR_LINE2 + 4 );							// Posiciona cursor na 5º coluna da linha 2.
		lcd_rom_puts( "UNIP 2016" );
		
		// Desloca a mensagem para a direita e para a esquerda 10 vezes.
		for ( contador= 0; contador < 10; contador++ )
		{
			Delay10KTCYx( 30 );								// Delay de 0.3 segundo.
			lcd_msg_shl();									// Desloca todo o conteúdo para a esquerda.
			Delay10KTCYx( 30 );								// Delay de 0.3 segundo.
			lcd_msg_shr();									// Desloca todo o conteúdo para a direita.
		}


		for ( contador= 0; contador < 7; contador++ )		// Pisca o display 7 vezes.
		{	
			lcd_clear();									// Limpa o display.
	
			Delay10KTCYx( 25 );								// Aguarda 0.25 segundo.

			lcd_cmd( CUR_LINE1 + 2 );						// Posiciona cursor na linha 1, coluna 2.
			lcd_rom_puts( "Professor" );					// Escreve.
	
			lcd_cmd( CUR_LINE2 + 1 );						// Posiciona cursor na linha 2, coluna 1.
			lcd_rom_puts( "GIOVANNI RIZZO" );				// Escreve.
	
			Delay10KTCYx( 25 );								// Aguarda 0.25 segundo.
		}

		lcd_cmd( CURSOR_ONP );		// Habilita cursor piscante.
		
		Delay10KTCYx( 250 );		// Atraso de 5 segundos.
		Delay10KTCYx( 250 );
	}
}

#pragma interrupt isr					// Rotina de tratamento de interrupção.
#pragma code isr= 0x00008
void isr( void )
{
	if ( INTCONbits.TMR0IF )			// Se interrupção provocada por estouro do timer0.
	{
		ClrWdt();						// Limpa o wathdog timer.

		WriteTimer0( delayT0 );				// Reinicia o contador do timer0.

		LATBbits.LATB0= ! LATBbits.LATB0;	// Inverte o estado do LED em RB0.

		INTCONbits.TMR0IF=	0;			// Zera o flag de interrupção do timer0.
	}
}