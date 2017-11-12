/*
		Programa bimanual inteligente para a placa McLab2 com PIC18F452.
		Compilador utilizado: C18.

		O objetivo é acionar o motor apenas quando os botões S1 e S2 forem 
		pressionados simultaneamente, com um delay de no máximo 250 ms entre
		um botão e o outro.

		Sinaliza tentativa de burlar o sistema com um alarme sonoro.

		Realiza o acionamento do motor por 5 segundos, podendo ser interrom-
		pido por um botão de emergência.

		Disciplinas de Microprocessadores e Microcontroladores para o curso de
		Tecnologia em Automação Industrial. E Sistemas Microcontrolados para os
		cursos de Engenharia de Automação Mecatrônica e Engenharia Eletrônica.

		Autor:  prof. Giovanni Rizzo Junior.   São Paulo, 14/09/2016.
*/

#include <P18F452.h>				// Define o microcontrolador utilizado.

#include <delays.h>					// Funções de delay.

#pragma config	OSC=	XT			// Oscilador do clock em modo XT.
#pragma	config	WDT=	OFF			// Watchdog-Timer desabilitado.

#define	START1	PORTBbits.RB0		// Botão Start1 conectado ao pino RB0.
#define	START2	PORTBbits.RB1		// Botão Start2 conectado ao pino RB1.

#define	LED		LATBbits.LATB2		// LED indicador conectado ao pino RB2.
#define	MOTOR	LATCbits.LATC1		// Motor DC acionado pelo pino RC1.

#define	EMERGENCIA	PORTBbits.RB3	// Botão emergência no pino RB3.

#define	BUZZER	PORTAbits.RA5		// Buzzer no pino RA5.

#define	TEMPO_BOTAO	250u			// Tempo de 250 milissegundos.

unsigned int contador;				// Contador de tempo.

void main( void )					// Função principal do programa.
{
	ADCON1=	0b00000111;				// Pinos em modo digital.

	LED=	0;						// LED inicialmente desligado.
	MOTOR=	0;						// MOTOR inicialmente desligado.
	BUZZER=	0;						// BUZZER inicialmente desligado.

	TRISA=	0b11011111;				// No PORTA, apenas o pino RA5 é saída ( BUZZER ).
	TRISB=	0b11111011;				// No PORTB, apenas o pino RB2 é saída ( LED ).
	TRISC=	0b11111101;				// No PORTC, apenas o pino RC1 é saída ( MOTOR ).

	PIE1=	0x00;					// Configura as interrupções do Timer2.

	PR2=	200;					// Período do Timer2.

	INTCONbits.PEIE= 1;				// Habilita as interrupções periféricas.

	INTCONbits.GIE= 1;				// Liga a chave geral de interrupções.

	T2CON=	0b00000100;				// Habilita o Timer2.


	while ( 1 )
	{
		while ( !START1 || !START2 );	// Aguarda ambos os botões estarem soltos.

		while ( START1 && START2 );	// Aguarda algum botão ser pressionado.
		
		// Enquanto o intervalo de tempo for inferior a 250 milissegundos.
		for ( contador= 0; contador < TEMPO_BOTAO; contador++ ) 
		{
			if ( !START1 && !START2 )	// Se ambos os botões forem pressionados.
			{
				LED=	1;				// Liga o LED.
				MOTOR=	1;				// Liga o motor.

				// Aguarda 5 segundos, contando 5000 milissegundos.
				for ( contador= 0; contador < 5000u; contador++ )
				{
					if ( !EMERGENCIA )	// Se o botão emergência foi pressionado.
						break;			// Sai do loop for, para desligar tudo.

					Delay1KTCYx( 1 );	// Atraso de 1 ms.
				}

				// Fim dos 5 segundos.

				LED=	0;				// Desliga o LED.
				MOTOR=	0;				// Desliga o motor.

				break;					// Abandona o loop for.
			}

			Delay1KTCYx( 1 );			// Atraso de 1 ms.
		}

		// Se o loop for contou até o limite do tempo do botão, um alarme 
		// intermitente acionado.

		contador= 0;					// Zera o contador de tempo.


		if ( ! START1 || ! START2 )	// Enquanto pressionar qualquer botão:
		{
			char tempo= 0;

			PIE1bits.TMR2IE= 1;		// Habilita a interrupção do Timer2.

			do {

				Delay100TCYx( 1 );	// Atraso de 100 microssegundos.

				if ( ! START1 || ! START2 )
					tempo= 0;
				else
					++tempo;

			} while ( tempo < 120 );	
		}

		PIE1bits.TMR2IE= 0;				// Desabilita a interrupção do Timer2.

		BUZZER= 0;						// Garante o desligamento do buzzer.	
	}

}

#pragma interrupt isr
#pragma code isr= 0x00008
void isr( void )
{
	if ( PIR1bits.TMR2IF )				// Interrupção gerada pelo Timer2?
	{
			if ( contador < 1000u )		// Se o contador for menor que XXXXX
				BUZZER= ! BUZZER;		// Inverte o estado do buzzer.
			else
				BUZZER= 0;				// senão, desliga o buzzer.

			if ( contador == 2000u )	// Se contador chegar a XXXX,
				contador= 0;			// zera o contador.

			contador++;					// Incrementa o contador.
	
		PIR1bits.TMR2IF= 0;				// Resseta o flag de interrupção.
	}
	
}
