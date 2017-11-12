/*
	 Exemplo de convers�o anal�gico-digital utilizando um microcontrolador PIC18.
	 O objetivo aqui � transformar o microcontrolador em um term�metro digital e 
	 regulador de temperatura configurado para a placa do kit did�tico McLab2.

	Programa associado � placa de simula��o McLab2. A placa McLab2 mede a tempera-
	tura pr�xima ao resistor por meio de um diodo de sinal 1N4148.

	Conforme a temperatura aumenta, a curva Corrente x Tens�o caracter�stica do
	diodo � alterada. E a queda de tens�o do diodo varia com a temperatura.
	
	Essa varia��o � amplificada por um circuito com amplificador operacional e
	comparada com uma tens�o de refer�ncia ajust�vel em um potenci�metro da placa.

	Uma estimativa da temperatura aparece nos displays de 7 segmentos, enquanto no
	display de LCD aparece a temperatura real, a temperatura de refer�ncia ( set-
	point ), e a porcentagem do ciclo-ativo ( duty cycle ) do PWM.

	A ideia � fazer um regulador de temperatura com elo de realimenta��o.

	Se a temperatura medida for inferior ao valor de refer�ncia ( set-point ), o
	ciclo ativo ( duty cycle ) do PWM � incrementado para o resistor dissipar mais
	energia na forma de calor.

	Se, por outro lado, a temperatura medida for superior ao valor de refer�ncia,
	o duty cycle do sinal PWM aplicado � resist�ncia diminui, gerando uma tend�n-
	cia de queda na temperatura.


	Autor: prof. Giovanni Rizzo
*/

#include <P18F452.h>		// Define o microcontrolador utilizado: PIC18F4550

#include <adc.h>			// Arquivo cabe�alho ( header file ) com rotinas de
							// convers�o anal�gico-digital escrita em C.
#include <math.h>			// Fun��es matem�ticas ( aritm�ticas ).
#include <delays.h>			// Fun��es de delay.
#include <timers.h>			// Rotinas dos timers. ( Timer 0 utilizado na varre-
							// dura dos displays.
#include <pwm.h>			// Rotinas de PWM.

#include <usart.h>

#include <stdio.h>			// Cont�m o prot�tipo da rotina sprintf().

#pragma config OSC=	XT	// Oscilador do clock no modo XT.
#pragma config WDT=	OFF		// Watchdog-Timer desabilitado.
#pragma config PWRT=ON		// Power-Up Timer habilitado.
#pragma config LVP=	OFF		// Low-Voltage Programming desabilitado.

// Macro que realiza a fun��o de arredondamento de n�mero em ponto-flutuante.
#define round( arg ) ( ( ( arg ) > floor( arg ) + 0.5f ) ? ceil( arg ) : floor( arg ) )

// Macro que calcula o valor absoluto ( m�dulo ) de um n�mero escalar.
#define abs( arg ) ( ( arg  )< 0 ? -( arg ) : ( arg ) )

#define	BLANK	0b00000000			// Valor do display em branco.
#define PONTO	0b10000000			// M�scara do display com ponto.
#define NEG		0b01000000			// Sinal negativo.
#define CELSIUS	0b00111001			// Letra C para graus Celsius.


#define	RS	LATEbits.LATE0			// Pino comando/caractere do disp. LCD em RE0.
#define	EN	LATEbits.LATE1			// Pino de habilita��o do disp. LCD em RE1.
#define RW	LATEbits.LATE2			// Pino Read/Write do disp. LCD em RE2.

#define BTN_UP 		PORTBbits.RB3	// Bot�o para elevar o valor do Set-Point da temperatura.
#define BTN_DOWN	PORTBbits.RB2	// Bot�o para reduzir o valor do Set-Point da temperatura.

// Defini��es de mnem�nicos para os comandos do display de LCD.
#define CURSOR_OFF	0x0C
#define CURSOR_SHR	0x06
#define CLEAR		0x01
#define	MSG_SHR		0x1C
#define	MSG_SHL		0x18
#define CUR_SHR		0x14
#define CUR_SHL		0x10
#define	CUR_INIT	0x02
#define CUR_LINE1	0x80
#define	CUR_LINE2	0xC0

#define delayT0	( 65536 - 250 )	// Delay do Timer0. => 500 us

// N�mero de amostras do filtro de m�dia m�vel.
#define NUM_AMOSTRAS	20

#define	VENTILADOR	LATCbits.LATC1

// Declara��o de prot�tipo do c�digo do vetor de interrup��o de alta prioridade.
void VETOR_INT_ALTA( void );

// Declara��o de prot�tipo do c�digo da rotina de interrup��o.
void ROTINA_INTERRUPCAO( void );

// Declara��o dos prot�tipos das fun��es do display de LCD.
void lcd_init( void );	
void lcd_cmd( unsigned char cmd );
void lcd_putc( unsigned char chr );
void lcd_ram_puts( ram char *string );
void lcd_rom_puts( const rom char *string );
 
	
// Declara��o do prot�tipo da rotina de exibi��o do display de LCD.
void showRefs( void );
			
// Prot�tipo da rotina de  leitura dos bot�es.
void btns( void );

// Vetor de convers�o de temperatura.
const rom unsigned char tabela_temp[ 256 ]=
{
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		// 15
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	1,		// 31
	2,	2,	3,	3,	4,	4,	5,	5,	6,	6,	7,	7,	8,	8,	9,	9,		// 47
	10,	10,	11,	11,	12,	12,	13,	13,	14,	14,	15,	15,	16,	16,	17,	17,		// 63
	18,	18,	19,	19,	20,	20,	21,	21,	22,	22,	23,	23,	23,	24,	24,	25,		// 79
	25,	26,	26,	27,	27,	28,	28,	29,	29,	30,	30,	31,	31,	32,	32,	33,		// 95
	33,	34,	34,	35,	35,	36,	36,	37,	37,	38,	38,	39,	39,	40,	40,	41,		// 111
	41,	42,	42,	43,	43,	44,	44,	45,	45,	46,	46,	47,	47,	48,	48,	49,		// 127
	49,	50,	50,	51,	51,	52,	52,	53,	53,	54,	54,	55,	55,	56,	56,	57,		// 143
	57,	58,	58,	59,	59,	60,	60,	61,	61,	62,	62,	63,	63,	64,	64,	65,		// 159
	65,	66,	66,	67,	67,	68,	68,	69,	69,	70,	70,	71,	71,	72,	72,	73,		// 175
	73,	74,	74,	75,	75,	76,	76,	77,	77,	78,	78,	79,	79,	80,	80,	81,		// 191
	81,	82,	82,	83,	83,	84,	84,	85,	85,	86,	86,	87,	87,	88,	88,	89,		// 207
	89,	90,	90,	91,	91,	92,	92,	93,	93,	94,	94,	95,	95,	96,	96,	97,		// 223
	97,	98,	98,	99,	99,	100,100,101,101,102,102,103,103,104,104,104,	// 239
	105,105,106,106,107,107,108,108,109,109,110,110,111,111,112,112		// 255
};

// Vetor de convers�o de BCD para c�digo do display de 7 segmentos.
const rom char bcd2disp[]=
{
	0b00111111,		// d�gito 0
	0b00000110,		// d�gito 1 
	0b01011011,		// d�gito 2
	0b01001111,		// d�gito 3
	0b01100110,		// d�gito 4
	0b01101101,		// d�gito 5
	0b01111101,		// d�gito 6
	0b00000111,		// d�gito 7
	0b01111111,		// d�gito 8
	0b01101111		// d�gito 9
};


// Vari�veis globais que cont�m os d�gitos decimais dos displays ( formato BCD ).
char digito1, digito2, digito3;

// Resultado binario da convers�o anal�gico-digital.
volatile unsigned int conversao;

// Resultado da conversao ja em ponto flutuante.
volatile float temp;

// Valor correspondente � taxa do duty cycle do PWM.
unsigned int taxa_pwm= 0;

// Valor do duty cycle ( ciclo ativo) do sinal PWM.
float duty_cycle= 0.0;

// Valor do Set-Point da temperatura.
unsigned int setpoint= 40;

// Rotina principal do programa.
void main( void )
{
	float	media_temp;					// Media movel das amostras.
	float	vetor_temp[ NUM_AMOSTRAS ]; // Vetor de amostras.
	char i;								// Variavel auxiliar ( indice ).

	ADCON1= 0b00000111;			// Pinos de I/O inicialmente no modo digital.

	OpenTimer0( TIMER_INT_OFF &	// Interrup��o do Timer0 inicialmente desabilitada.
				T0_SOURCE_INT &	// Timer0 utiliza a fonte de clock interna.
				T0_16BIT &		// Timer0 configurado como contador de 16 bits.
				T0_PS_1_2 );	// Timer0 utiliza prescaler de 1:2.

	WriteTimer0( delayT0 );		// Inicializa o Timer0 com o valor do delay.


	// Configura��o do conversor anal�gico-digital ( AD ).
	OpenADC( ADC_FOSC_8 & 		// Clock do conversor com frequ�ncia Fosc / 8.
			 ADC_LEFT_JUST &	// Resultado da convers�o justificado � esquerda.
			 ADC_3ANA_0REF,		// Refer�ncias Vref+ = Vcc, e  Vref- = 0 V ( GND )
			 ADC_CH0 &			// Seleciona o canal AN0 ( pino RA0 ).
			 ADC_INT_OFF );		// Interrup��o associada ao conversor AD desabilitada.

	// Configura a inicializacao do Timer2 ( que gera o periodo do sinal PWM ).
	OpenTimer2( TIMER_INT_OFF &	// Interrupcao do Timer2 desabilitada.
				T2_PS_1_16 );	// Prescaler de 1:16.

	OpenPWM1( 199 );			// PR2= 199.

	SetDCPWM1( 0 );				// PWM inicialmente com duty-cycle de 0%.

	// O periodo do sinal PWM eh obtivo pela expressao: 
	// 
	//					  	( PR2 + 1 ) * 4 * VALOR_PRESCALER_TMR2
	// PERIODO			=	____________________________________
	//		 pwm				 			Fosc
	//
	//
	// No caso presente, usamos temos PR2= 199, Fosc = 4 MHz e o valor
	// do prescaler do PWM eh 16. Portanto:
	//
	// 						( 199 + 1 ) * 4 * 16
	// PERIODO			=  _____________________  = 3200 us = 3.2 ms.
	//		   pwm						  6
	//								4 * 10
	// 
	// Ja o tempo de sinal alto ( do duty cicle ) eh dado por: 
	//
	// TEMPO				taxa_pwm * VALOR_PRESCALER_TMR2
	//		 sinal alto =	_______________________________ 
	//									Fosc
	//
	//					= taxa_pwm * 4 us.
	//
	// Sendo assim, o valor maximo de taxa_pwm ( para duty cycle de 100% )
	// eh igual a 3200 / 4 = 800. 

	TRISA=	0b11111111;			// Pinos do PORTA funcionam como entradas.

	LATB=	0b00000000;			// Inicializa todos os pinos em n�vel baixo.
	TRISB=	0b00001100;			// Configura os pinos do PORTB como sa�da.
								// Com a excecao de RB2 e RB3.

	TRISC=	0b11111001;			// RC1 aciona o ventilador. RC2 aciona o resistor aquecedor.
	LATC=	0b00000000;

	LATD=	0b00000000;			// PORTD conectado aos display de 7 segmentos e de LCD. Inicialmente em n�vel baixo.
	TRISD=	0b00000000;			// Todos os pinos do PORTD configurados como sa�da.

	TRISE=	0b000;				// Pinos RE0 e RE1 s�o sa�das. RE2 � entrada.
	LATE=	0b100;


	Delay1KTCYx( 15 );			//	Atraso de 15 ms para a correta inicializa��o do display.

	lcd_init();					//	Inicializa��o do display de LCD.


	INTCONbits.TMR0IE=	0; 		// Interrup��o do Timer0 inicialmente desabilitada.
	INTCONbits.TMR0IF=	0;		// Flag de ocorr�ncia de interrup��o desativado.
	RCONbits.IPEN=		0;		// Sem n�veis de prioridade de interrup��o.
	INTCONbits.GIE=		1;		// Chave geral de interrup��o habilitada.

	while ( 1 )					// Loop infinito.
	{
		INTCONbits.TMR0IE=	0;		// Desabilita a interrup��o do Timer0 antes da
									// convers�o iniciar.

		btns();						// Chama a rotina de leitura dos bot�es.

		while ( BusyADC() );		// Aguarda o t�rmino da convers�o, caso n�o tenha terminado.
				
		conversao= ADRESH;			// Obt�m o resultado da convers�o.

		ConvertADC();				// Inicia uma nova convers�o AD.

		// O valor correspondente a uma unidade do conversor pode ser
		// obtido atrav�s da equa��o: 
		// 
		//					  ( Vref+ ) - ( Vref- )
		// V			=	_________________________
		//	unidade				 resolu��o
		//						2			-	1
		// No caso presente, usamos s� os 8 bits de ADRESH, portanto a 
		// resolu��o � de 8 bits.
		// As tens�es de refer�ncia s�o: Vref+ = 3V  e Vref- = 0V.
		// Al�m disso, como o resultado da convers�o � a tens�o el�trica
		// com precis�o de duas casas decimais, o resultado � multiplicado
		// por 100 e o d�gito decimal mais alto recebe o ponto.
		// Na placa McLab2 � usado um diodo como sensor de temperatura.
		// O vetor vetor_temp se encarrega de converter a leitura do con-
		// versor AD em uma estimativa da temperatura.

		temp= ( float ) conversao;

		// Substituir esse c�digo por um mais eficiente. 
		// Armazenar os NUM_AMOSTRAS com um for apenas uma vez.
		// Nas leituras seguintes substituir um �nico valor do vetor.
		// ( round robin )
		// Somar todos os valores para o c�lculo da m�dia apenas uma vez.
		// Nas seguintes, subtrair da m�dia o valor que sai do vetor e somar
		// o valor que entra. Desse modo, temos um menor n�mero de opera��es.
		// Diminuindo significativamente o custo computacional do filtro.
			
		for ( i= 0, media_temp= 0; i < NUM_AMOSTRAS-1; i++ )
		{
			vetor_temp[ i ]= vetor_temp[ i+1 ];
			media_temp+= vetor_temp[ i ];
		}

		vetor_temp[ NUM_AMOSTRAS - 1 ]= temp;
		media_temp+= temp;
		media_temp/= NUM_AMOSTRAS;
		temp= media_temp;

		
		INTCONbits.TMR0IE=  1;		// Interrup��o do Timer0 ( que seleciona os displays )
									// habilitada somente ap�s a convers�o AD.
		
		Delay1KTCYx( 1 );			// Atraso de 10000 �s = 10 ms.
	}

}

// Vetor de interrup��o de alta prioridade ( �nica interrup��o usada nesse programa ).
#pragma code VETOR_INT_ALTA= 0x00008
void VETOR_INT_ALTA( void )
{
	_asm GOTO ROTINA_INTERRUPCAO _endasm	// Desvia para a rotina de tratamento.
}
#pragma code


// Rotina de tratamento de interrup��o.
#pragma interrupt ROTINA_INTERRUPCAO
void ROTINA_INTERRUPCAO( void )
{
	// M�scara de sele��o de display.
	static unsigned char dscounter= 0b10000000;
	static unsigned char digito;	// Armazena cada d�gito BCD.

	static unsigned char delaypwm=  10;

	static unsigned char modulo_lcd= 0;
												
	if ( INTCONbits.TMR0IF )		// Testa se a interrup��o foi gerada pelo Timer0.
	{
		INTCONbits.TMR0IF=	0;		// Zera o flag de ocorr�ncia de interrup��o do Timer0.

		WriteTimer0( delayT0 );		// Reinicia a contagem de tempo.
	
		LATD=	0b00000000;			// Limpa o conte�do do LATD. Display apagado.

		LATB&=  0b00001111;			// Desabilita todos os displays.
		Delay10TCYx( 1 );			// Atraso de 10 ciclos de barramento.

		if ( ! modulo_lcd-- )		// Atualiza o display de LCD periodicamente.
		{
			showRefs();

			LATD= 0b00000000;

			modulo_lcd= 251;
		}

		switch ( dscounter>>= 1 )	// Testa a m�scara de sele��o de display.
		{
			default:				// Se nenhum display esta selecionado...
					dscounter= 0b10000000; // Reinicia a mascara.

					conversao= ( unsigned int ) abs( round( temp ) ); // Converte.

					conversao= tabela_temp[ conversao ];

					if ( ! --delaypwm )
					{
						// Se a temperatura eh inferior ao setpoint o duty-cycle aumenta.
						if ( conversao < setpoint  && taxa_pwm < 800 )
						{
							SetDCPWM1( taxa_pwm++ );
						}

						// Se a temperatura eh superior ao setpoint, o duty cycle diminui.
						if ( conversao > setpoint && taxa_pwm > 0 )
						{
							SetDCPWM1( taxa_pwm-- );
						}

						delaypwm= 65000;//abs( 65000 - 5000 * abs( ( int ) conversao - ( int ) setpoint ) );
					}

					if ( conversao >= setpoint + 2 )
					{
						VENTILADOR=	1;
					}

					if ( conversao <= setpoint + 1 )
					{
						VENTILADOR=	0;
					}


			case 0b10000000:		
		

					digito1= conversao / 100; // Obt�m o d�gito mais significativo.

					// Move o d�gito para o LATD.
					LATD= digito1 == 0 ? BLANK : bcd2disp[ digito1 ];
				
					break;

			case 0b01000000:

					// Extrai o peso do primeiro d�gito.
					digito= conversao - digito1 * 100; 

					// Obt�m o d�gito seguinte.
					digito2= digito / 10;

					// Move o d�gito para o LATD.
					LATD= conversao < 990 ? bcd2disp[ digito2 ] /*| PONTO */ : NEG;
				
					break;
	
			case 0b00100000:

					// Subtrai o peso do segundo d�gito.
					digito-= digito2 * 10;
					digito3= digito;
				
					// Move o d�gito para o LATD.
					LATD= conversao < 990 ? bcd2disp[ digito3 ] : NEG;

					break;

			case 0b00010000:

					LATD= CELSIUS;		// Exibe a letra C de Celsius.
					break;
		}
		
		LATB|= dscounter;				// Habilita o display selecionado.
	}
}


	// Fun��o de exibi��o dos valores de refer�ncia no display de LCD.

	void showRefs( void )
	{
		static char strtemp[ 63 ];
	
	
		lcd_cmd( CUR_LINE1 );
		sprintf( strtemp, "T=%d%d%d", digito1, digito2, digito3 );
		lcd_ram_puts( strtemp );
		lcd_putc( 0b11011111 );  // �
		lcd_putc( 'C' );

		sprintf( strtemp, " Tr=%d", setpoint );
		lcd_ram_puts( strtemp );
		lcd_putc( 0b11011111 );  // �
		lcd_putc( 'C' );

		lcd_cmd( CUR_LINE2 );
	
		duty_cycle= ( 100.0f * ( float ) taxa_pwm ) / ( ( PR2 + 1 ) * 4.0f );

		sprintf( strtemp, " PWM=%d%%  ", ( char ) round( duty_cycle ) );
		lcd_ram_puts( strtemp );

		Delay10TCYx( 5 );
	
		EN= 0;	
	}

	// Rotina de inicializa��o do display de LCD.	
	void lcd_init( void )
	 {
		lcd_cmd( 0x30 );
		Delay1KTCYx( 4 );

		lcd_cmd( 0x30 );
		Delay100TCYx( 1 );

		lcd_cmd( 0x38 );
		Delay10TCYx( 4 );

		lcd_cmd( CLEAR );
		Delay1KTCYx( 2 );

		lcd_cmd( CURSOR_OFF );

		lcd_cmd( CURSOR_SHR );
	 }
	
	// Rotina para enviar um comando para o display de LCD.
	void lcd_cmd( unsigned char cmd )
	 {
		RS=	0;
		LATD=	cmd;
		EN=	1;
		Delay10TCYx( 1 );
		EN=	0;
		Delay10TCYx( 4 );
	 }
	
	// Rotina para enviar um caractere para o display de LCD.
	void lcd_putc( unsigned char chr )
	 {
		RS=	1;
		LATD=	chr;
		EN=	1;
		Delay10TCYx( 1 );
		EN=	0;
		Delay10TCYx( 4 );
	 }
	
	// Rotina para escrever um texto da RAM no display de LCD.
	void lcd_ram_puts( ram char *string )
	 {
		do
		 {
			lcd_putc( *string );
		 } while ( *++string );
	 }
	
	// Rotina para escrever um texto de uma mem. n�o-vol�til no display de LCD.
	void lcd_rom_puts( const rom char *string )
 	{
		do
	 	{
			lcd_putc( *string );
	 	} while ( *++string );
 	}
	
#define BTNFILTRO		30		// Filtro ( leituras consecutivas com bot�o pressionado )
#define OFFSETMINIMO	10		// Range m�nimo = 1�C.
		
	// Rotina de leitura dos bot�es.
	void btns( void )
	{
		static unsigned int tmaxdown=	BTNFILTRO;	// Contador de leituras de BTEMP_MAX_DOWN.
		static unsigned int tmaxup=		BTNFILTRO;	// Contador de leituas de BTEMP_MAX_UP.			
		
		// Se BTEMP_MAX_DOWN estiver pressionado por BTNFILTRO leituras consecutivas, ent�o
		// temp_max � decrementado.
		if ( ! BTN_DOWN )
		{
			if ( ! --tmaxdown )
			{
				tmaxdown= BTNFILTRO;
		
				if ( setpoint > 30 )
					setpoint--;
			}
		}
		else
		{
			tmaxdown= BTNFILTRO;
		}

		// Se BTEMP_MAX_UP estiver pressionado por BTNFILTRO leituras consecutivas, ent�o
		// temp_max � incrementado.
		if ( ! BTN_UP )
		{
			if ( ! --tmaxup )
			{
				tmaxup= BTNFILTRO;
				
				if ( setpoint < 80 )
					setpoint++;
			}
		}
		else
		{
			tmaxup= BTNFILTRO;
		}
	}