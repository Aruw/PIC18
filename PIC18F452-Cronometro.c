
/*
		Experiência utilizando timer, interrupção e display de 7 segmentos.
	
		Cronômetro com precisão de centésimos de segundos.
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

// Período do timer 0 = PRESCALER * 25000 * TCY 
// = 4 * 2500 * 1 us = 10 ms = 0.01 s
#define delayT0	( 65536 - 2500 ) 

#define START	PORTBbits.RB0		// Botão de START/PAUSE/CONTINUE.
#define RESET	PORTBbits.RB2		// Botão de RESET. ( Zera a contagem ).

#define	FILTRO_LIMITE	200			// Limite para o filtro do botão.

void isr( void );		// Protótipo da rotina de tratamento de interrupção.

// Vetor de conversão de BCD para display de 7 seg.
const rom near char vetor[]=
{
	0b00111111,						// Dígito 0.
	0b00000110,						// Dígito 1.
	0b01011011,						// Dígito 2.
	0b01001111,						// Dígito 3.
	0b01100110,						// Dígito 4.
	0b01101101,						// Dígito 5.
	0b01111101,						// Dígito 6.
	0b00000111,						// Dígito 7.
	0b01111111,						// Dígito 8.
	0b01101111						// Dígito 9.
};

// Variáveis globais correspondentes aos dígitos BCD.
volatile char centesimo= 0, decimo= 0, unidade= 0, dezena= 0;

void main( void )			// Rotina principal do programa.
{
	char select= 0b00010000;	// Byte de seleção do display de 7 segmentos.

	// Filtro de debouncing do botão //START/PAUSE.
	// Porque esse botão funciona como toggle.
	unsigned char filtro= FILTRO_LIMITE;

	ADCON1=	0b00000111;		// Pinos em modo de I/O digital.

	TRISB=	0b00001111;		// RB0 a RB3 são entradas ( botões ). 
							// RB4 a RB7 SÃO saídas ( displays ).

	LATB=	0x00;		// Saídas do PORTB inicialmente em nível lógico baixo.

	TRISD=	0x00;		// PORTD é saída ( sinal enviado para cada display ).
	LATD=	0x00;		// Pinos do PORTD inicialmente em nível lógico baixo.


	OpenTimer0( T0_SOURCE_INT & // Timer0 é incrementado pelos pulsos do clock.
				T0_16BIT & 		// Modo 16 bits. Permite um range de 0 a 65535.
				T0_PS_1_4 );	// Prescaler de 1:4 => O timer é 
								// incrementado a cada 4 pulsos de clock.

	T0CONbits.TMR0ON= 0;		// Timer inicialmente desligado.

	WriteTimer0( delayT0 );		// Inicializa o contador do timer 0.

	INTCONbits.TMR0IE= 0;		// Habilita a interrupção do timer 0.
	INTCONbits.TMR0IF= 0;		// Resseta o flag de interrupção do timer 0.

	RCONbits.IPEN= 0;		// Microcontrolador operando sem prioridade de 
							// interrupções.

	INTCONbits.TMR0IE=	1;	//	Habilita a interrupção do Timer0.

	INTCONbits.GIE= 1;	// Chave geral de interrupções habilitada.

	while ( 1 )			// Laço principal do programa.
	{
		LATD= 0x00;		// Resseta a saída do PORTD para apagar o display 
						// habilitado ( reduzindo ruídos ).
		LATB&= 0x0F;	// Resseta os 4 bits mais significativos do PORTB. 
						// Desabilitando todos os displays.

		Delay10TCYx( 1 );	// Atraso de 10 us.

		switch ( select )	// Seleção do display durante a varredura.
		{
			case 0b00010000:	// Dígito centésimo de segundo.
				LATD= vetor[ centesimo ]; 
				break;

			case 0b00100000:	// Dígito décimo de segundo.
				LATD= vetor[ decimo ];	break;

			case 0b01000000:	// Dígito unidade dos segundos.
								// Com ponto decimal.
				LATD= vetor[ unidade ] | PONTO;	break;

			case 0b10000000:	// Dígito da dezena.
				LATD= vetor[ dezena ]; break;
		}

		LATB|= select;	// Habilita o display selecionado.
		select<<= 1;	// Deslocamento à esquerda da seleção do 
						// display ( passa para o próximo ).

		// Depois que o bit foi deslocado para a última posição, 
		// reinicializa.
		if ( ! select ) select= 0b00010000;	

		// O botão START/PAUSE é do tipo Toggle ( liga/desliga ).		
		if ( ! START )		// Se o botão START foi pressionado.
		{
			if ( filtro == 0 )	// Se o filtro é igual a zero.
				T0CONbits.TMR0ON= ! T0CONbits.TMR0ON;   // Inverte a 
					// habilitação do Timer0. Se estava ligado,
					// desliga. E vice-versa.
			// Reinicia o filtro quando o botão for pressinado.		
			filtro= FILTRO_LIMITE;	
		}
		else				// Se o botão estiver liberado:
		{
			// Enquanto filtro > 0, o filtro é decrementado.
			if ( filtro )
				--filtro;
		}

		// Se RESET foi pressionado durante a contagem de tempo...
		if ( ! RESET && ! T0CONbits.TMR0ON )  
		{					  
			WriteTimer0( delayT0 );    // ... reinicia o timer0.
			// Zera todos os dígitos.
			centesimo= decimo= unidade= dezena= 0;
		}
		// Atraso de 100 us para visualização do display.
		Delay100TCYx( 1 );	
	}
}

// isr é rotina de interrupção ( termina com a instrução RETFIE ).
#pragma interrupt isr

// O endereço inicial de isr é 0x00008, o vetor de interrupção.
#pragma code isr= 0x00008	

void isr( void )			// Rotina isr, de tratamento de interrupção.
{
	if ( INTCONbits.TMR0IF )	// Se a interrupção foi disparada pelo 				
	{							// estouro do timer0...
		WriteTimer0( delayT0 );			// Reinicia o timer0.

		if ( ++centesimo == 10 ) // Incrementa centésimo. Se atingir valor 10...
		{
			centesimo= 0;					// ...zera centésimo.
			if ( ++decimo == 10 )		// Incrementa décimo, e, se atingir	
			{					// valor 10...
				decimo= 0;		// ...zera o dígito décimo.
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
								// habilitando uma nova interrupção.
	}
}
