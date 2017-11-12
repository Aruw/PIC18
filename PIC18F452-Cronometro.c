
/*
		Experi�ncia utilizando timer, interrup��o e display de 7 segmentos.
	
		Cron�metro com precis�o de cent�simos de segundos.
		Cursos Superiores em Tecnologia e Engenharia - UNIP - 2017.

		prof. Giovanni Rizzo Junior
*/

#include <P18F452.h>	// Microcontrolador utilizado.

#include <timers.h>		// Rotinas dos timers.
#include <delays.h>		// Rotinas de delay.

#pragma config OSC= XT	// Oscilador do clock em modo XT.
#pragma config WDT=	OFF	// Watchdog Timer desabilitado.
#pragma config LVP=	OFF	// Low Voltage Programming desabilitado, permite 
                        // uso do pino RB5

#define	PONTO	0b10000000	// Ponto decimal no display de 7 segmentos.

// Per�odo do timer 0 = PRESCALER * 25000 * TCY 
// = 4 * 2500 * 1 us = 10 ms = 0.01 s
#define delayT0	( 65536 - 2500 ) 

#define START	PORTBbits.RB0		// Bot�o de START/PAUSE/CONTINUE.
#define RESET	PORTBbits.RB2		// Bot�o de RESET. ( Zera a contagem ).

#define	FILTRO_LIMITE	200			// Limite para o filtro do bot�o.

void isr( void );		// Prot�tipo da rotina de tratamento de interrup��o.

// Vetor de convers�o de BCD para display de 7 seg.
const rom near char vetor[]=
{
	0b00111111,						// D�gito 0.
	0b00000110,						// D�gito 1.
	0b01011011,						// D�gito 2.
	0b01001111,						// D�gito 3.
	0b01100110,						// D�gito 4.
	0b01101101,						// D�gito 5.
	0b01111101,						// D�gito 6.
	0b00000111,						// D�gito 7.
	0b01111111,						// D�gito 8.
	0b01101111						// D�gito 9.
};

// Vari�veis globais correspondentes aos d�gitos BCD.
volatile char centesimo= 0, decimo= 0, unidade= 0, dezena= 0;

void main( void )			// Rotina principal do programa.
{
	char select= 0b00010000;	// Byte de sele��o do display de 7 segmentos.

	// Filtro de debouncing do bot�o //START/PAUSE.
	// Porque esse bot�o funciona como toggle.
	unsigned char filtro= FILTRO_LIMITE;

	ADCON1=	0b00000111;		// Pinos em modo de I/O digital.

	TRISB=	0b00001111;		// RB0 a RB3 s�o entradas ( bot�es ). 
							// RB4 a RB7 S�O sa�das ( displays ).

	LATB=	0x00;		// Sa�das do PORTB inicialmente em n�vel l�gico baixo.

	TRISD=	0x00;		// PORTD � sa�da ( sinal enviado para cada display ).
	LATD=	0x00;		// Pinos do PORTD inicialmente em n�vel l�gico baixo.


	OpenTimer0( T0_SOURCE_INT & // Timer0 � incrementado pelos pulsos do clock.
				T0_16BIT & 		// Modo 16 bits. Permite um range de 0 a 65535.
				T0_PS_1_4 );	// Prescaler de 1:4 => O timer � 
								// incrementado a cada 4 pulsos de clock.

	T0CONbits.TMR0ON= 0;		// Timer inicialmente desligado.

	WriteTimer0( delayT0 );		// Inicializa o contador do timer 0.

	INTCONbits.TMR0IE= 0;		// Habilita a interrup��o do timer 0.
	INTCONbits.TMR0IF= 0;		// Resseta o flag de interrup��o do timer 0.

	RCONbits.IPEN= 0;		// Microcontrolador operando sem prioridade de 
							// interrup��es.

	INTCONbits.TMR0IE=	1;	//	Habilita a interrup��o do Timer0.

	INTCONbits.GIE= 1;	// Chave geral de interrup��es habilitada.

	while ( 1 )			// La�o principal do programa.
	{
		LATD= 0x00;		// Resseta a sa�da do PORTD para apagar o display 
						// habilitado ( reduzindo ru�dos ).
		LATB&= 0x0F;	// Resseta os 4 bits mais significativos do PORTB. 
						// Desabilitando todos os displays.

		Delay10TCYx( 1 );	// Atraso de 10 us.

		switch ( select )	// Sele��o do display durante a varredura.
		{
			case 0b00010000:	// D�gito cent�simo de segundo.
				LATD= vetor[ centesimo ]; 
				break;

			case 0b00100000:	// D�gito d�cimo de segundo.
				LATD= vetor[ decimo ];	break;

			case 0b01000000:	// D�gito unidade dos segundos.
								// Com ponto decimal.
				LATD= vetor[ unidade ] | PONTO;	break;

			case 0b10000000:	// D�gito da dezena.
				LATD= vetor[ dezena ]; break;
		}

		LATB|= select;	// Habilita o display selecionado.
		select<<= 1;	// Deslocamento � esquerda da sele��o do 
						// display ( passa para o pr�ximo ).

		// Depois que o bit foi deslocado para a �ltima posi��o, 
		// reinicializa.
		if ( ! select ) select= 0b00010000;	

		// O bot�o START/PAUSE � do tipo Toggle ( liga/desliga ).		
		if ( ! START )		// Se o bot�o START foi pressionado.
		{
			if ( filtro == 0 )	// Se o filtro � igual a zero.
				T0CONbits.TMR0ON= ! T0CONbits.TMR0ON;   // Inverte a 
					// habilita��o do Timer0. Se estava ligado,
					// desliga. E vice-versa.
			// Reinicia o filtro quando o bot�o for pressinado.		
			filtro= FILTRO_LIMITE;	
		}
		else				// Se o bot�o estiver liberado:
		{
			// Enquanto filtro > 0, o filtro � decrementado.
			if ( filtro )
				--filtro;
		}

		// Se RESET foi pressionado durante a contagem de tempo...
		if ( ! RESET && ! T0CONbits.TMR0ON )  
		{					  
			WriteTimer0( delayT0 );    // ... reinicia o timer0.
			// Zera todos os d�gitos.
			centesimo= decimo= unidade= dezena= 0;
		}
		// Atraso de 100 us para visualiza��o do display.
		Delay100TCYx( 1 );	
	}
}

// isr � rotina de interrup��o ( termina com a instru��o RETFIE ).
#pragma interrupt isr

// O endere�o inicial de isr � 0x00008, o vetor de interrup��o.
#pragma code isr= 0x00008	

void isr( void )			// Rotina isr, de tratamento de interrup��o.
{
	if ( INTCONbits.TMR0IF )	// Se a interrup��o foi disparada pelo 				
	{							// estouro do timer0...
		WriteTimer0( delayT0 );			// Reinicia o timer0.

		if ( ++centesimo == 10 ) // Incrementa cent�simo. Se atingir valor 10...
		{
			centesimo= 0;					// ...zera cent�simo.
			if ( ++decimo == 10 )		// Incrementa d�cimo, e, se atingir	
			{					// valor 10...
				decimo= 0;		// ...zera o d�gito d�cimo.
				if ( ++unidade == 10 )	// Incrementa unidade, e, se atingir 
				{						// valor 10...
					unidade= 0;			// ...zera unidade.
					if ( ++dezena == 10 )	// Incrementa dezena, e, se 		
								 // atingir o valor10...
						dezena= 0;		// ...zera dezena.
				}
			}
		}

		INTCONbits.TMR0IF= 0;	// Resseta o flag de estouro do timer0, 			
								// habilitando uma nova interrup��o.
	}
}
