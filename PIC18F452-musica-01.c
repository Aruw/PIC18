/*
	Experiência de toques de tons musicais utilizando o PIC18F452.
	Este projeto utiliza duas fontes de interrupções com prioridades.
	A interrupção do Timer2 gera o tom musical, cujo período é deter-
	minado pelo valor do registrador PR2.
	A interrupção do Timer0 determina a duração do toque. Rotinas de
	delay não seriam confiáveis nesse caso, porque o toque consome
	um tempo considerável do processador.

	autor: prof. Giovanni Rizzo, São Paulo, 16/09/2017
*/


#include <P18F452.h>		// Define o microcontrolador utilizado.

#include <delays.h>			// Biblioteca com rotinas de delay.
#include <timers.h>			// Biblioteca referente aos timers.
#include <math.h>			// Biblioteca de funções matemáticas.


#pragma config	OSC=	XT		// Oscilador do clock em modo XT.
#pragma config	WDT=	OFF		// Watchdog-Timer desabilitado.

#define	BUZZER	LATAbits.LATA5	// Buzzer acionado pelo pino RA5.

#define	LED	LATBbits.LATB0		// LED acionado pelo pino RB0.

#define	NUM_NOTAS	36			// Número máximo de notas musicais.

#define	delayT0	( 65536 - 50000 ) // Intervalo de tempo entre dois 
								// estouros do Timer0.
								//Com TCY= 1 us, e usando um pres-
								// caler de 1:2, temos 
								// delayT0= 50 ms x 2 = 0.1 segundo.  

// Relações entre a cifra de cada nota musical e a posição no vetor.
#define	A1	0		// Nota Lá ( primeira oitava ).
#define	B1B	1		// Nota Si Bemol ( ou Lá Sustenido ).
#define B1	2		// Nota Si.
#define	C1	3		// Nota Dó.
#define	D1B	4		// Nota Ré Bemol ( ou Dó Sustenido ).
#define D1	5		// Nota Ré.
#define	E1B	6		// Noda Mi Bemol ( ou Ré Sustenido ).
#define	E1	7		// Nota Mi.
#define	F1	8		// Nota Fá.
#define	G1B	9		// Nota Sol Bemol ( ou Fá Sustenido ).
#define	G1	10		// Nota Sol.
#define	A2B	11		// Nota Lá Bemol ( ou Sol Sustenido ).
#define	A2	12		// Nota Lá ( segunda oitava ).
#define	B2B	13		// Nota Sí Bemol ( ou Lá Sustenido ).
#define	B2	14		// Nota Sí.
#define	C2	15		// Nota Dó.
#define	D2B	16		// Nota Ré Bemol ( ou Dó Sustenido ).
#define	D2	17		// Nota Ré.
#define	E2B	18		// Nota Mi Bemol ( dou Ré Sustenido ).
#define	E2	19		// Nota Mi.
#define	F2	20		// Nota Fá.
#define	G2B	21		// Nota Sol Bemol ( ou Fá Sustenido ).
#define	G2	22		// Nota Sol.
#define	A3B	23		// Nota Lá Bemol ( ou Sol Sustenido ).
#define	A3	24		// Nota Lá.
#define B3B	25		// Nota Si Bemol ( ou Lá Sustenido ).
#define B3	26		// Nota Si.
#define C3	27		// Nota Dó

// Protótipos das sub-rotinas.

// Rotina para tocar uma nota, com duração definida
// pelo parâmetro tempo.
void tocar( int nota, int tempo );

// Vetores de interrupção:
void isr_high( void ); // Vetor de alta-prioridade.
void isr_low( void );	// Vetor de baixa-prioridade.

// Rotinas de tratamento de interrupção.
void isr_timer2( void );	// Rotina do Timer2.
void isr_timer0( void );	// Rotina do Timer0.

// Nota fundamental ( mais baixa ). Possui a frequência
// da nota Lá em Hertz.
const float	NOTA_INICIAL=	220.0f;

float vetor_notas[ NUM_NOTAS ];	// Frequências das notas.
int vetor_tempo[ NUM_NOTAS ];	// Períodos das notas.

// Define se o toque está habilitado.
volatile char habilitado;

// Define a duração do toque.
volatile int duracao;

void main( void )		// Rotina principal do programa.
{
	char i;				// Variável para contagem.
	float passo; 		// Razão geométrica entre dois semitons consecutivos.
	
	ADCON1=	0b00000111;		// Pinos em modo digital.
	BUZZER=	0;				// Buzzer inicialmente desligado.
	TRISA=	0b11011111;		// Pino RA5 configurado como saída ( Buzzer ).
	LED=	0;				// LED inicialmente desligado.
	TRISB=	0b11111110;		// Pino do LED configurado como saída.

	INTCONbits.TMR0IF=	0;	// Flag de interrupção do Timer0 inicialmente em 0.
	INTCONbits.TMR0IE=	1;	// Habilita interrupção do Timer0.

	RCONbits.IPEN=	1;		// Habilita prioridade de interrupções.

	INTCONbits.PEIE= 1;		// Habilita interrupções de periféricos.
	INTCONbits.GIE=	1;		// Liga a chave geral de interrupções.
	
	WriteTimer0( delayT0 );	// Inicializa o contador do Timer0.

	INTCON2bits.TMR0IP= 0;	// Timer0 gera interrupção de baixa prioridade.

	// Liga o Timer0 com interrupção habilitada, modo 16 bits, incremento
	// fornecido pelo clock interno, e prescaler de 1:2.
	OpenTimer0( TIMER_INT_ON & T0_16BIT & T0_SOURCE_INT & T0_PS_1_2 );

	// Razão entre semitons= 2 ^ ( 1/12 )
	passo= pow( 2.0f, 1.0f / 12.0f );	
	vetor_notas[ 0 ]= NOTA_INICIAL;		// Primeira nota = NOTA_INICIAL.

	// Para as notas seguintes à primeira, o valor da frequência é o valor
	// do passo multiplicado pela frequência do semitom.
	for ( i= 1; i < NUM_NOTAS; i++ )
	{
		vetor_notas[ i ]= passo * vetor_notas[ i - 1 ];
	}	

	// Para cada nota cadastrada, calcula o meio-período, que deve ser o 
	// tempo em que o buzzer fica ligado ou desligado. Esse valor atribuí-
	// do ao registrador PR2 faz o buzzer emitir a frequência específica.
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

	duracao= tempo;		// Define a duração.

	PIE1bits.TMR2IE= 0;		// Desliga a interrupção do Timer2.
	T2CONbits.TMR2ON= 0;	// Desliga o Timer2.
	
	// Define o novo período em função da nota.
	PR2= vetor_tempo[ nota ];

	// Interrupção do Timer2 com alta prioridade.
	IPR1bits.TMR2IP= 1;

	// Liga interrupção do Timer2, com prescaler de 1:1 e
	// Postscaler de 1:10.
	OpenTimer2( TIMER_INT_ON & T2_PS_1_1 & T2_POST_1_10 );
	
	T2CONbits.TMR2ON= 1;	// Garante a ligação do Timer2.

	// Aguarda o tempo do toque encerrar.
	while ( habilitado );

	BUZZER= 0;	// Desliga o buzzer.
}

// Vetor de interrupção de alta prioridade.
#pragma code isr_high= 0x00008
void isr_high( void )
{
	_asm 
		GOTO	isr_timer2	// Desvia para rotina do Timer2.
	_endasm
}
#pragma code

// Rotina de tratamento de interrupção do Timer2, com alta
// prioridade.
#pragma interrupt isr_timer2
void isr_timer2( void )
{
	if ( PIR1bits.TMR2IF )	// Se Timer2 gerou interrupção:
	{
		if ( habilitado )	// Se o toque está habilitado:
		{
			BUZZER= ! BUZZER;	// Inverte o buzzer.
		}
		else			// Se o toque estiver desabilitado:
		{
			BUZZER= 0;		// Desliga o buzzer.
		}
		PIR1bits.TMR2IF= 0;	// Zera o flag de interrupção.
	}
}

// Vetor de interrupção de baixa prioridade.
#pragma code isr_low= 0x00018

void isr_low( void )
{
	_asm
		GOTO	isr_timer0	// Desvia para rotina do Timer0.
	_endasm
}
#pragma code

// Rotina de tratamento de interrupção do Timer0, com baixa
// prioridade.
#pragma interruptlow isr_timer0
void isr_timer0( void )
{
	static int contador= 0;	// Contador inicializado em zero.

	// Se a interrupção foi gerada pelo Timer0:
	if ( INTCONbits.TMR0IF )
	{
		WriteTimer0( delayT0 );		// Inicializa o Timer0.

		if ( habilitado )	// Se o toque está habilitado:
		{
			// Incrementa o contador e compara com a duração.
			if ( ++contador == duracao )
			{
				contador= 0;	// Zera o contador.
				habilitado= 0;	// Desabilita o toque.
			}
		}		
		
		INTCONbits.TMR0IF= 0;	// Zera o flag da interrupção.
	}
}
