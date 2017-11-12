/*
	Experi�ncia de toques de tons musicais utilizando o PIC18F452.
	Este projeto utiliza duas fontes de interrup��es com prioridades.
	A interrup��o do Timer2 gera o tom musical, cujo per�odo � deter-
	minado pelo valor do registrador PR2.
	A interrup��o do Timer0 determina a dura��o do toque. Rotinas de
	delay n�o seriam confi�veis nesse caso, porque o toque consome
	um tempo consider�vel do processador.

	autor: prof. Giovanni Rizzo, S�o Paulo, 16/09/2017
*/


#include <P18F452.h>		// Define o microcontrolador utilizado.

#include <delays.h>			// Biblioteca com rotinas de delay.
#include <timers.h>			// Biblioteca referente aos timers.
#include <math.h>			// Biblioteca de fun��es matem�ticas.


#pragma config	OSC=	XT		// Oscilador do clock em modo XT.
#pragma config	WDT=	OFF		// Watchdog-Timer desabilitado.

#define	BUZZER	LATAbits.LATA5	// Buzzer acionado pelo pino RA5.

#define	LED	LATBbits.LATB0		// LED acionado pelo pino RB0.

#define	NUM_NOTAS	36			// N�mero m�ximo de notas musicais.

#define	delayT0	( 65536 - 50000 ) // Intervalo de tempo entre dois 
								// estouros do Timer0.
								//Com TCY= 1 us, e usando um pres-
								// caler de 1:2, temos 
								// delayT0= 50 ms x 2 = 0.1 segundo.  

// Rela��es entre a cifra de cada nota musical e a posi��o no vetor.
#define	A1	0		// Nota L� ( primeira oitava ).
#define	B1B	1		// Nota Si Bemol ( ou L� Sustenido ).
#define B1	2		// Nota Si.
#define	C1	3		// Nota D�.
#define	D1B	4		// Nota R� Bemol ( ou D� Sustenido ).
#define D1	5		// Nota R�.
#define	E1B	6		// Noda Mi Bemol ( ou R� Sustenido ).
#define	E1	7		// Nota Mi.
#define	F1	8		// Nota F�.
#define	G1B	9		// Nota Sol Bemol ( ou F� Sustenido ).
#define	G1	10		// Nota Sol.
#define	A2B	11		// Nota L� Bemol ( ou Sol Sustenido ).
#define	A2	12		// Nota L� ( segunda oitava ).
#define	B2B	13		// Nota S� Bemol ( ou L� Sustenido ).
#define	B2	14		// Nota S�.
#define	C2	15		// Nota D�.
#define	D2B	16		// Nota R� Bemol ( ou D� Sustenido ).
#define	D2	17		// Nota R�.
#define	E2B	18		// Nota Mi Bemol ( dou R� Sustenido ).
#define	E2	19		// Nota Mi.
#define	F2	20		// Nota F�.
#define	G2B	21		// Nota Sol Bemol ( ou F� Sustenido ).
#define	G2	22		// Nota Sol.
#define	A3B	23		// Nota L� Bemol ( ou Sol Sustenido ).
#define	A3	24		// Nota L�.
#define B3B	25		// Nota Si Bemol ( ou L� Sustenido ).
#define B3	26		// Nota Si.
#define C3	27		// Nota D�

// Prot�tipos das sub-rotinas.

// Rotina para tocar uma nota, com dura��o definida
// pelo par�metro tempo.
void tocar( int nota, int tempo );

// Vetores de interrup��o:
void isr_high( void ); // Vetor de alta-prioridade.
void isr_low( void );	// Vetor de baixa-prioridade.

// Rotinas de tratamento de interrup��o.
void isr_timer2( void );	// Rotina do Timer2.
void isr_timer0( void );	// Rotina do Timer0.

// Nota fundamental ( mais baixa ). Possui a frequ�ncia
// da nota L� em Hertz.
const float	NOTA_INICIAL=	220.0f;

float vetor_notas[ NUM_NOTAS ];	// Frequ�ncias das notas.
int vetor_tempo[ NUM_NOTAS ];	// Per�odos das notas.

// Define se o toque est� habilitado.
volatile char habilitado;

// Define a dura��o do toque.
volatile int duracao;

void main( void )		// Rotina principal do programa.
{
	char i;				// Vari�vel para contagem.
	float passo; 		// Raz�o geom�trica entre dois semitons consecutivos.
	
	ADCON1=	0b00000111;		// Pinos em modo digital.
	BUZZER=	0;				// Buzzer inicialmente desligado.
	TRISA=	0b11011111;		// Pino RA5 configurado como sa�da ( Buzzer ).
	LED=	0;				// LED inicialmente desligado.
	TRISB=	0b11111110;		// Pino do LED configurado como sa�da.

	INTCONbits.TMR0IF=	0;	// Flag de interrup��o do Timer0 inicialmente em 0.
	INTCONbits.TMR0IE=	1;	// Habilita interrup��o do Timer0.

	RCONbits.IPEN=	1;		// Habilita prioridade de interrup��es.

	INTCONbits.PEIE= 1;		// Habilita interrup��es de perif�ricos.
	INTCONbits.GIE=	1;		// Liga a chave geral de interrup��es.
	
	WriteTimer0( delayT0 );	// Inicializa o contador do Timer0.

	INTCON2bits.TMR0IP= 0;	// Timer0 gera interrup��o de baixa prioridade.

	// Liga o Timer0 com interrup��o habilitada, modo 16 bits, incremento
	// fornecido pelo clock interno, e prescaler de 1:2.
	OpenTimer0( TIMER_INT_ON & T0_16BIT & T0_SOURCE_INT & T0_PS_1_2 );

	// Raz�o entre semitons= 2 ^ ( 1/12 )
	passo= pow( 2.0f, 1.0f / 12.0f );	
	vetor_notas[ 0 ]= NOTA_INICIAL;		// Primeira nota = NOTA_INICIAL.

	// Para as notas seguintes � primeira, o valor da frequ�ncia � o valor
	// do passo multiplicado pela frequ�ncia do semitom.
	for ( i= 1; i < NUM_NOTAS; i++ )
	{
		vetor_notas[ i ]= passo * vetor_notas[ i - 1 ];
	}	

	// Para cada nota cadastrada, calcula o meio-per�odo, que deve ser o 
	// tempo em que o buzzer fica ligado ou desligado. Esse valor atribu�-
	// do ao registrador PR2 faz o buzzer emitir a frequ�ncia espec�fica.
	// T = 1 / f ( em segundos ).  T = 1000000 / f ( em microssegundos ).
	// T / 2 = 500000 / f ( em microssegundos ).
	for ( i= 0; i < NUM_NOTAS; i++ )
	{
		vetor_tempo[ i ]= ( int ) ( 50000.0f / vetor_notas[ i ] );
	}

	habilitado= 0;		// Inicialmente desabilita qualquer toque.

	while ( 1 )			// Loop infinito ( Loop principal do programa ).
	{
		tocar( C2, 6 );
		tocar( E2, 3 );
		tocar( G2B, 3 );
		tocar( A3,	3 );
		tocar( G2, 6 );

		tocar( E2, 3 );

		Delay1KTCYx( 30 );
		
		tocar( C2, 6 );
		tocar( A2, 3 );
		
		tocar( G1B, 2 );
		tocar( G1B, 2 );
		tocar( G1B, 2 );
		tocar( G1, 12 );

		Delay1KTCYx( 30 );
	
		tocar( G1B, 2 );
		tocar( G1B, 2 );
		tocar( G1B, 2 );
		tocar( G1B, 2 );
		tocar( B2B, 12 );

		tocar( C1, 2 );
		tocar( C1, 2 );
		tocar( C1, 2 );
		tocar( C1, 6 );

		Delay10KTCYx( 60 );
	}		
}

// Rotina que inicia um toque musical.
void tocar( int nota, int tempo )
{
	habilitado= 1;		// Habilita o toque.

	duracao= tempo;		// Define a dura��o.

	PIE1bits.TMR2IE= 0;		// Desliga a interrup��o do Timer2.
	T2CONbits.TMR2ON= 0;	// Desliga o Timer2.
	
	// Define o novo per�odo em fun��o da nota.
	PR2= vetor_tempo[ nota ];

	// Interrup��o do Timer2 com alta prioridade.
	IPR1bits.TMR2IP= 1;

	// Liga interrup��o do Timer2, com prescaler de 1:1 e
	// Postscaler de 1:10.
	OpenTimer2( TIMER_INT_ON & T2_PS_1_1 & T2_POST_1_10 );
	
	T2CONbits.TMR2ON= 1;	// Garante a liga��o do Timer2.

	// Aguarda o tempo do toque encerrar.
	while ( habilitado );

	BUZZER= 0;	// Desliga o buzzer.
}

// Vetor de interrup��o de alta prioridade.
#pragma code isr_high= 0x00008
void isr_high( void )
{
	_asm 
		GOTO	isr_timer2	// Desvia para rotina do Timer2.
	_endasm
}
#pragma code

// Rotina de tratamento de interrup��o do Timer2, com alta
// prioridade.
#pragma interrupt isr_timer2
void isr_timer2( void )
{
	if ( PIR1bits.TMR2IF )	// Se Timer2 gerou interrup��o:
	{
		if ( habilitado )	// Se o toque est� habilitado:
		{
			BUZZER= ! BUZZER;	// Inverte o buzzer.
		}
		else			// Se o toque estiver desabilitado:
		{
			BUZZER= 0;		// Desliga o buzzer.
		}
		PIR1bits.TMR2IF= 0;	// Zera o flag de interrup��o.
	}
}

// Vetor de interrup��o de baixa prioridade.
#pragma code isr_low= 0x00018

void isr_low( void )
{
	_asm
		GOTO	isr_timer0	// Desvia para rotina do Timer0.
	_endasm
}
#pragma code

// Rotina de tratamento de interrup��o do Timer0, com baixa
// prioridade.
#pragma interruptlow isr_timer0
void isr_timer0( void )
{
	static int contador= 0;	// Contador inicializado em zero.

	// Se a interrup��o foi gerada pelo Timer0:
	if ( INTCONbits.TMR0IF )
	{
		WriteTimer0( delayT0 );		// Inicializa o Timer0.

		if ( habilitado )	// Se o toque est� habilitado:
		{
			// Incrementa o contador e compara com a dura��o.
			if ( ++contador == duracao )
			{
				contador= 0;	// Zera o contador.
				habilitado= 0;	// Desabilita o toque.
			}
		}		
		
		INTCONbits.TMR0IF= 0;	// Zera o flag da interrup��o.
	}
}
