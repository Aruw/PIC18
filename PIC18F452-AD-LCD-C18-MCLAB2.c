/* -----------------------------------------------------------------------------
/* * * * * * * * * * *     CONVERS�O ANAL�GICO-DIGITAL     * * * * * * * * * * *
   -----------------------------------------------------------------------------

	 Exemplos de convers�o Anal�gico-Digital usando o display de LCD.

// Programa associado � placa de simula��o McLab2.

// Cursos de Engenharia e Tecnologia - UNIP - 2� sem. de 2016
// Disciplina: Microprocessadores e Microcontroladores

// Autor: prof. Giovanni Rizzo

*/
// ------------------------------------------------------------------------------
// Inclus�o dos arquivos de cabe�alho ( header files ).

#include <P18F452.h>		// Define o microcontrolador utilizado: PIC18F452

#include <delays.h>			// Fun��es de delay.

#include <adc.h>			// Fun��es do conversor anal�gico-digital.

#include <stdio.h>			// Arquivo header que cont�m a rotina sprintf().

#include "lcd.h"			// Rotinas para comunica��o com o display de LCD.

// ------------------------------------------------------------------------------
// Configura��o dos fuses ( configuration bits ) do microcontrolador.

#pragma config OSC=	XT		// Oscilador do clock no modo XT.
#pragma config WDT=	ON		// Watchdog-Timer habilitado.
#pragma config PWRT=ON		// Power-Up Timer habilitado.
#pragma config BOR=	ON		// Brown-Out Reset habilitado.
#pragma config BORV= 45		// Reset para condi��o de Brown-Out.
#pragma config LVP=	OFF		// Low-Voltage Programming desabilitado.

// ------------------------------------------------------------------------------
// 

void main( void )					// Fun��o principal do programa.
{
	char string[ 32 ];					// String com texto a ser exibido.
	long valor;						// Vari�vel que cont�m o valor convertido.
	long temp;
	char unidade, decimo;


	ADCON1=	0b00000100;					// Canais anal�gicos AN0, AN1 e AN3 habilitados.

	TRISAbits.TRISA1=	1;				// Pino RA1 � entrada ( canal AN1 do conv. AD ).

	TRISB=	0b11111111;					// Habilita LEDs em RB0 e RB1.
	LATB=	0x00;

	TRISD=	0b00000000;					// Pinos do PORTD s�o sa�das ( para enviar dados ao display ).
	LATD=	0b00000000;

	EN=	1;								// Certifica-se de que nenhum sinal � enviado ao display.
	RS=	1;

	Delay100TCYx( 1 );					// Atraso de 100 microssegundos.		

	TRISE=	0b00000100;					// Pinos RE0 e RE1 s�o sa�das. ( RS e EN ).

	lcd_init();						// Invoca a rotina de inicializa��o do display.

	while ( RCONbits.NOT_TO );		// Aguarda o reset por Time-Out do Watchdog-Timer.

	Delay10KTCYx( 100 );				// Atraso de 1 segundo para estabilizar as tens�es no display.

	lcd_init();							// Invoca a rotina de inicializa��o do display.

	lcd_cmd( CURSOR_OFF );				// Desabilita o cursor do display.

	// Configura��o do conversor anal�gico-digital ( AD ).
	OpenADC( ADC_FOSC_8 &		// Clock do conversor com freq. = Fosc / 8.
			 ADC_RIGHT_JUST &	// Resultado justificado � direita.
			 ADC_6ANA_0REF,		// Vref+= Vdd ( 5V ) e Vref-= Vss ( 0V ).
			 ADC_CH1 &			// Seleciona/habilita o canal anal�gico 1 ( pino AN1 ).
			 ADC_INT_OFF );		// Interrup��o do conversor AD desabilitada.
	

	while ( 1 )							// Loop principal.
	{		
		lcd_cmd( CUR_LINE1 );							// Posiciona o cursor no canto superior esquerdo.
		lcd_rom_puts( "Conv. AD. C18" );				// Imprime a mensagem.
	
		lcd_cmd( CUR_LINE2 );			// Posiciona o cursor no canto inferior esquerdo.
		lcd_rom_puts( "Tensao = " );	// Imprime a mensagem.

	
		ConvertADC();				// Inicia a convers�o AD.

		while ( BusyADC() );		// Aguarda o t�rmino da convers�o.
				
		// Obt�m o valor bin�rio da convers�o com todos os bits.
		// Esse c�digo serve para valor justificado � direita.
		valor= ADRESH;
		valor<<= 8;
		valor+= ADRESL;

		// O valor correspondente a uma unidade do conversor pode ser
		// obtido atrav�s da equa��o: 
		// 
		//					  ( Vref+ ) - ( Vref- )
		// V			=	_________________________
		//	unidade				 resolu��o
		//						2			-	1
		// No caso presente, usamos s� os 8 bits de ADRESH, portanto a 
		// resolu��o � de 8 bits.
		// As tens�es de refer�ncia s�o: Vref+ = 5V  e Vref- = 0V.
		// 
		// Resolu��o = 10 bits. 2^10 = 1024.
		// O resultado deve ser dividido por 1023.
		
		valor*= 5000;					// Mapeia no intervalo 0 a 5000.
		valor/= 1023;
	
		temp= valor;                   // Armazena o valor convertido em temp.
       
       	valor/= 100;                    // Remapeia no invertalo 0 a 50.
       
       	if ( temp % 100 >= 50 )         // Deve arredondar para cima?
          valor++;
           
       	unidade= valor / 10;            // unidade � quociente da divis�o inteira.
       	decimo= valor % 10;             // decimo � o m�dulo ( resto ) da divis�o.

       	string[ 0 ]= unidade + '0';     // Obt�m o caractere da unidade.
       	string[ 1 ]= '.';               // Armazena o caractere ponto decimal.
       	string[ 2 ]= decimo + '0';      // Obt�m o caractere do valor decimal.
       	string[ 3 ]= 'V';               // Armazena o caractere 'V' da unidade Volts.
       	string[ 4 ]= '\0';              // Finaliza com caractere de final de string.

		lcd_ram_puts( string );			// Escreve o valor convertido no display.

		ClrWdt();

		Delay1KTCYx( 10 );				// Atraso de 10 milissegundos.
	}
}
