/* Programa bimanual inteligente para a placa McLab2 com PIC18F452. Compilador utilizado: C18.

O objetivo � acionar o motor apenas quando os bot�es S1 e S2 forem pressionados simultaneamente, 
com um delay de no m�ximo 250ms entre um bot�o e o outro. Sinaliza tentativa de burlar o sistema
com um alarme sonoro. Realiza o acionamento do motor por 5 segundos, podendo ser interrompido por 
um bot�o de emerg�ncia. Disciplina de Microprocessadores e Microcontroladores para o curso de 
Tecnologia em Automa��o Industrial. E Sistemas Microcontrolados para os cursos de Engenharia de 
Automa��o Mecatronica e Engenharia Eletr�nica.

Autor: Prof. Giovanni Rizzo Junior. S�o Paulo, 13/09/2017 

*/

#include <P18F452.h>			//Define o microcontrolador utilizado
#include <delays.h>				//Fun��es de delay 

#pragma config OSC = XT			//Oscilador do clock em modo XT
#pragma config WDT = OFF		//Watcdog-Timer desabilitado

#define START1 PORTBbits.RB0 	//Bot�o Start1 conectado ao pino RB0
#define START2 PORTBbits.RB1 	//Bot�o Start2 conectado ao pino RB1
#define LED LATBbits.LATB2		//LED indicador conectado ao pino RB2
#define MOTOR LATCbits.LATC1 	//Motor DC acionado pelo pino RC1
#define EMERGENCIA PORTBbits.RB3//Bot�o emerg�ncia no pino RB3
#define BUZZER LATAbits.LATA5	//Buzzer no Pino RA5
#define TEMPO_BOTAO 250u		//Intervalo de 250 milissegundos entre os bot�es

void main (void){				//Fun��o Principal do programa

	unsigned int contador = 0;	//Contador de tempo
	ADCON1 = 0b00000111;		//Pinos em modo digital
	LED = 0;					//Led inicialmente desligado
	MOTOR = 0;					//Motor inicialmente desligado
	BUZZER = 0;					//Buzzer inicialmente desligado 
	TRISA = 0b11011111;			//No PORTA, apenas o pino RA5 � sa�da (Buzzer)
	TRISB = 0b11111011;			//No PORTB, apenas o pino RB2 � sa�da (LED)
	TRISC = 0b11111101;			//No PORTC, apenas o pino RC1 � sa�da (MOTOR)

	while(1){
		while(START1 && START2)	//Aguarda algum bot�o ser precionado 
								//Enquanto o intervalo de tempo for inferior a 250 milissegundos 
			for(contador = 0; contador < TEMPO_BOTAO; contador++){
				if(!START1 && !START2){	//Se ambos os bot�es forem pressionados 
					LED = 1;			//Liga o LED
					MOTOR = 1;			//Liga o Motor 
					//Aguarda 5 segundos, contando 5000 milissegundos 
					for(contador = 0; contador < 5000u; contador++){
						if(!EMERGENCIA){		//Se o bot�o emergencia foi pressionado 
							break;				//Sai do loop for, para desligar tudo 
						}	
						Delay1KTCYx(1);			//Atraso de 1ms.
					}
						//Fim dos 5 segundos 
						LED = 0; //Desliga o LED
						MOTOR = 0;//Desliga o MOTOR
						break;		//Abandona o loop for
					}
						Delay1KTCYx(1); //Atraso de 1ms
			}		
						//Se o loop for contou at� o limite do tempo do bot�o, um alarme intermitente acionado
						contador = 0;		//Zera o contador de tempo
					while(!START1||!START2){ 	//Enquanto pressionar qualquer bot�o:
						if(contador < 2500u){	//Se o contador for menor de 2500.
							BUZZER= ! BUZZER;	//Inverte o estado do buzzer
						}else{
							BUZZER = 0;			//sen�o, desliga o buzzer
						}
						if(contador == 5000u){	//Se o contador chegar a 5000,
							contador = 0;		//zera o contador 
						}
						contador ++;			//Incrementa o contador 
						Delay100TCYx(1);		//Atraso de 100 microssegundos (0,1ms).
												//Som com periodo de 200 microssegundos
												//Frequ�ncia de 5kHz
				}
				BUZZER = 0;						//Garante o desligamento do Buzzer
			}
	} 