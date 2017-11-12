/*
	LEITURA E CONTROLE DA VELOCIDADE DE ROTAÇÃO DO VENTILADOR DA PLACA MCLAB2 USANDO
	MALHA ABERTA.

	VARIÁVEL CONTROLADA = 	VELOCIDADE ANGULAR DO VENTILADOR.
	VARIÁVEL MANIPULADA=	DUTY-CICLE ( CICLO ATIVO ) DO SINAL PWM APLICADO AO MOTOR DC.

	AS 7 PÁS DO VENTILADOR POSSIBILITAM A LEITURA DA VELOCIDADE COMO UM DECODER INCREMENTAL.

	A LEITURA DA VELOCIDADE FAZ USO DO MÓDULO CCP AO CALCULAR O TEMPO ENTRE A PASSAGEM DE 
	DUAS PÁS CONSECUTIVAS DO VENTILADOR PELO SENSOR ÓPTICO.

	JÁ O CONTROLE É REALIZADO POR UM SINAL PWM APLICADO AO MOTOR.
	
	Disciplina:  Microprocessadores e Microcontroladores, Sistemas Microcontrolados.

	prof.:		 Giovanni Rizzo Junior


*/

#include <P18F452.h>						// Define o microcontrolador utilizado.

#include <pwm.h>							// Funções do módulo PWM.
#include <timers.h>							// Funções e estruturas referentes aos timers.
#include <delays.h>							// Funções de atraso de tempo ( delay ).
#include <usart.h>							// Funções da USART ( Comunicação serial ).
#include <stdlib.h>							// Função rand().

#include "lcd.h"							// Funções e definições para o módulo LCD.

#pragma config	OSC=	XT					//	Modo de funcionamento do oscilador ( clock )= XT.
#pragma	config	WDT=	OFF					//	Wathdog-Timer desabilitado.
#pragma	config	LVP=	OFF					//	Low Voltage Programming desabilitado.
#pragma config	PWRT=	ON					//	Power-Up Timer habilitado.


#define abs(x) (x<0?-(x):(x)) 


#define BT1	PORTBbits.RB0					//	Botão para decrementar o setpoint.
#define	BT2	PORTBbits.RB1					//	Botão para incrementar o setpoint.

#define	BT7	PORTBbits.RB2					//	Botão para exibir o argumento do PWM no display de LEDs.


#define	TAXA_ATUALIZA_PWM	4				//	Delay para atualização do PWM.
#define	TAXA_BOTAO			4


// Dígitos a serem mostrados após a conversão de decimal em BCD.
unsigned char	unidade, dezena, centena;

unsigned char	tempo=	100;

unsigned int	cont_vent=			0;
unsigned int	cont_vent_round=	0;
float			intensidade_contr=	0;
char			SHOW_LCD=	0;
unsigned char	tempo_lcd=	0;

char usart_rx;

void converte_bcd( unsigned char aux );
void TRATA_INT_TIMER2( void );
void TRATA_HIGH_INT( void );
void display( char digito, char display );


void main( void )
 {
	int intensidade_vent=	300;				// Variável manipulada para o sinal PWM. 
												// Essa variável corresponde ao tempo em microssegundos em que o 
												// sinal PWM permanece em nível alto ( duty cycle ).

	int intensidade_vent_anterior=	300;		//	Valor anterior da variável ( para estimativa da derivada ).

	int ref_vent=		85; 					// Número de rotações por segundo do Setpoint ( valor de referência ).
	int referencia=	ref_vent * 7;				// A frequência de referência é o produto de ref_vent pelo número de pás.
	int ref_bt=		ref_vent * 100;

	int	erro= 0;								// Variável erro usada pelo controlador.

	char dscounter=		0;						
	char charcounter=	0;
	char usart_tx;

	char bt_counter=	0;


	unsigned int Periodo_PWM;
	unsigned int Tempo_Sinal_Alto;
	char Duty_Cycle;

	unsigned char atualizar_PWM=	0;


	PORTA=	PORTB=	PORTC=	PORTD=	PORTE=	0x00;
	LATA=	LATB=	LATC=	LATD=	LATE=	0x00;

	TRISA=	0b11111111;
	TRISB=	0b00000111;					// Botões em RB0, RB1 e RB2. LED em RB3. Seleção do display de LEDs de RB5 a RB7.
	TRISC=	0b10111001;
	TRISD=	0b00000000;
	TRISE=	0b00000100;

	ADCON1=	0b00000111;					// Pinos em modo digital



	// Configuração do Timer1:
	OpenTimer1( TIMER_INT_OFF &  	// Timer1 não gera requisição de interrupção. ( interrupção desabilitada ).
				T1_SOURCE_EXT &	 	// Timer1 com fonte de incremento ( clock ) externa ( sensor infravermelho ).
				T1_16BIT_RW   &	 	// Timer1 é contador de 16 bits.
				T1_PS_1_1     &	 	// Prescaler em 1:1.
				T1_OSC1EN_OFF &		// Oscilador do Timer1 desabilitado.
			   	T1_SYNC_EXT_ON  );	// Fonte de sincronismo com sinal externo ativada.


	OpenTimer2( TIMER_INT_ON &		// Interrupção do Timer2 habilitada.
				T2_PS_1_4 	 &		// Prescaler de 1:4.
				T2_POST_1_16 );		// Postscaler igual a 1:16.



	OpenPWM2( 239 );	// PR2= 239 -> Período PWM gerado pelo Timer2 = 960us. 
						// Período = ( PR2 + 1 ) * 4 * PrescalerTMR2 / Fosc.

	
	CCP2CON = 0b00001111;		// Configuração do módulo CCP2 no modo PWM, com bits duty-cycle inicialmente em 0%.
	


	SetDCPWM2( intensidade_vent ); 	// Inicia o sinal PWM com duty-cycle de 300 * 4 / 4 MHz = 300 us.
									// Porcentagem inicial de 300 us / ( 240 * 4 * 4 / 4 MHz ) = 31,25 %.

								  	//	             	intensidade_vent * PrescalerTMR2 / Fosc
								  	// Porcentagem =	____________________________________________
							  	  	//					( PR2 + 1 ) * 4 * PrescalerTMR2 / Fosc



	Periodo_PWM=	( ( ( unsigned int ) PR2 + 1 ) * 4 );	// Armazena o período do PWM = 960 microssegundos.

	PIR1bits.TMR2IF=	0;			//	Flag de interrupção do Timer2 inicialmente ressetado.
	RCONbits.IPEN=		0;			//	Interrupções sem nível de prioridade.
	INTCONbits.PEIE=	1;			//	Interrupções dos periféricos habilitadas.
	INTCONbits.GIE=		1;			//	Liga chave geral de interrupções.

	lcd_init();						//	Inicializa o display de LCD.

	lcd_cmd( CUR_LINE1 );				//	Posiciona o cursor no início da linha 1.
	lcd_rom_puts( "    MOTOR    PWM" );	//	Escreve na linha superior.
	
	Delay10KTCYx( 200 );				//	Atraso de 2 segundos para estabilizar a rotação do ventilador.

	while ( 1 )								// Loop infinito.
	 {
		if ( SHOW_LCD )						//	Se houver permissão para atualizar o display de LCD.
		 {					
	

			if ( bt_counter-- == 0 )							// Periodicamente lê os botões para incrementar ou
			{													// decrementar o setpoint.

				if ( ! BT1 )	intensidade_vent--;				
				if ( ! BT2 )	intensidade_vent++;

				bt_counter= TAXA_BOTAO;
			}
	

			LATB&=	0x0F;					//	Certifica-se da desabilitação dos displays de LEDs de 7 segmentos.

			lcd_cmd( CUR_LINE2 + 3 );			//	Posiciona o cursor no início da linha 2.

		
			converte_bcd( cont_vent_round );//	Obtém os 3 dígitos decimais da velocidade atual do motor.

			lcd_putc( ' ' );				//	Escreve um caractere de espaço.

			lcd_putc( centena ? '0' + centena : ' ' );	// Exibe os 3 dígitos no display.
			lcd_putc( dezena || centena ? '0' + dezena : ' ' );
			lcd_putc( unidade + '0' );


			// Calcula o duty-cycle pela fórmula:
			//
		  	//	             	intensidade_vent * PrescalerTMR2 
		  	// Porcentagem =	___________________________________
	  	  	//					( PR2 + 1 ) * 4 * PrescalerTMR2 


		
			Duty_Cycle=	( char ) ( ( 100.0 * ( float ) intensidade_vent ) / ( float ) Periodo_PWM ); 

	
			while ( Duty_Cycle > 100 )
			{
				intensidade_vent--;
				Duty_Cycle=	( char ) ( ( 100.0 * ( float ) intensidade_vent ) / ( float ) Periodo_PWM ); 
			}
			
			
			while ( Duty_Cycle < 0 || intensidade_vent < 0 )
			{
				intensidade_vent++;
				Duty_Cycle=	( char ) ( ( 100.0 * ( float ) intensidade_vent ) / ( float ) Periodo_PWM ); 
			}


			converte_bcd( Duty_Cycle );			// Obtém os 3 dígitos decimais.

			lcd_putc( ' ' );					// Envia 3 caracteres de espaço.
			lcd_putc( 'R' );
			lcd_putc( 'P' );
			lcd_putc( 'S' );
			lcd_putc( ' ' );

			lcd_putc( centena ? '0' + centena : ' ' );				// Exibe a porcentagem do duty-cycle.
			lcd_putc( dezena || centena ? '0' + dezena : ' ' );
			lcd_putc( unidade	+ '0' );
			lcd_putc( '%' );
			lcd_putc( ' ' );




			// O valor do PWM não é atualizado em todas as varreduras, o que ajuda o controle a ser mais estável
			// ( apesar de mais lento ). Atualizando a cada varredura o controle tende a ser mais instável.

			if ( atualizar_PWM-- == 0 )			
			{
				atualizar_PWM=	TAXA_ATUALIZA_PWM;			//	Reinicia a contagem de varreduras.

			
															// A duração do duty-cycle nunca pode superar o período.		
				if ( intensidade_vent > Periodo_PWM )
					intensidade_vent=	Periodo_PWM;

															// Nem ser inferior a zero.
				if ( intensidade_vent < 0 )
					intensidade_vent=	0;

				SetDCPWM2( intensidade_vent );				// Atualiza o valor do duty-cycle.	

			}
		 }	

	
		LATD=	0x00;
		Delay10TCYx( 1 );									// Atraso de 10 microssegundos.

		
		converte_bcd( intensidade_vent / 10 );
	

		if ( ! BT7 )										// Se o botão RB2 estiver pressionado, exibe 
		{													// intensidade_vent / 10 no display de LEDs.
			switch ( dscounter++ )
			 {
				case 0: display( centena, 4 ); break;
				case 1: display( dezena,  3 ); break;
				case 2: display( unidade, 2 ); break;
				default:display( intensidade_vent % 10, 1 ); dscounter= 0;
		 	}

			Delay10TCYx( 5 );									// Atraso de 50 microssegundos.
	
		
		}
	
		
 	 }

 }

// -----------------------------------------------------------------------------
// Rotina de tratamento de interrupção

#pragma code INTERRUPT	=	0x0008						// No PIC18, a interrupção de alta prioridade está no endereço 8.
#pragma interrupt	INTERRUPT
void INTERRUPT( void )
 {
	if ( ! PIR1bits.TMR2IF )	return;		// Se a interrupção não foi causada pelo Timer2, returnar da interrupção.

	PIR1bits.TMR2IF=	0;					//	Resseta o flag de ocorrência de interrupção.

	if ( ! --tempo )						// Após 6400
	{
		tempo=	100;
		cont_vent=	ReadTimer1();
		WriteTimer1( 0 );
	
		cont_vent_round=	cont_vent / 7;
	}

	if ( ! --tempo_lcd )
	{
		tempo_lcd= 5;
		SHOW_LCD=	1;
 	}	
	else
	{
		SHOW_LCD=	0;
	}
 }
#pragma code



void converte_bcd( unsigned char aux )
 {
	unidade=	aux % 10;
	aux/= 10;
	dezena=		aux % 10;
	aux/= 10;
	centena=	aux;
 }


void display( char digito, char display )
 {
	const rom static char num2disp[]=
	 {
		0b00111111,	//	0
		0b00000110,	//	1
		0b01011011,	//	2
		0b01001111,	//	3
		0b01100110,	//	4
		0b01101101,	//	5
		0b01111101,	//	6
		0b00000111,	//	7
		0b01111111,	//	8
		0b01101111,	//	9
		0b00000000	//	BLANK
 	};
	
	PORTD=	0x00;

	PORTB=	( 0x10 << ( display - 1 ) ) | ( PORTB & 0x0F );

	PORTD=	num2disp[ digito ];
 }
