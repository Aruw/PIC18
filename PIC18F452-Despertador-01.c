/*
		Relógio Despertador usando a placa McLab2 com o microcontrolador PIC18F452.

		Disciplina: Microprocessadores e Microcontroladores.
		prof.:		Giovanni Rizzo.
*/
#include <P18F452.h>				// 	Define o microcontrolador utilizado.

#include <timers.h>					//	Inclui as rotinas e definições dos timers.
#include <delays.h>					//	Inclui as rotinas de delays.

#pragma config OSC=		XT			//	Oscilador em modo XT.
#pragma config WDT=		OFF			//	Wathdog-Timer desabilitado.
#pragma config LVP=		OFF			//	Low Voltage Programming desabilitado.

#define	BUZZER		LATAbits.LATA5	//	Buzzer no pino RA5.

// Botões

#define	BTHOUR		PORTBbits.RB0	//	Botão de ajuste ( incremento ) das horas.
#define	BTMIN		PORTBbits.RB1	//	Botão de ajuste ( incremento ) dos minutos.
#define	BTTIME		PORTBbits.RB2	//	Botão de seleção do horário atual.
#define	BTALARM		PORTBbits.RB3	//	Botão de seleção do horário do alarme.

// Habilitação dos catodos dos displays.

#define	dsMinUnit	LATBbits.LATB4	//	Display do dígito unidade dos minutos no pino RB4.
#define	dsMinDec	LATBbits.LATB5	//	Display do dígito dezena dos minutos no pino RB5. 
#define	dsHourUnit	LATBbits.LATB6	//	Display do dígito unidade das horas no pino RB6.
#define	dsHourDec	LATBbits.LATB7	//	Display do dígito dezena das horas no pino RB7.

#define	ENABLED		1				//	ENABLED = HABILITADO ( nível lógico alto ).	
#define	DISABLED	0				//	DISABLED = DESABILITADO ( nível lógico baixo ).

#define ALARM	1					//	Quando disp == 1 ( ALARM ), exibe o horário do alarme.
#define	TIME	0					//	Quando disp == 0 ( TIME ), exibe o horário atual.

#define	deltaT0		(65536 - 62500)	//	Número de pulsos até o estouro do Timer0.
									// 	Com fclock = 4 MHz, temos TCY = 1 us ...
									//	... e prescaler de 1:16 , temos:
									//	Tempo de Estouro = 62500 * 16 * TCY 
									//					 = 62500 * 16 * 1 us 
									//					 = 1000000 us.
									//					 = 1 segundo.

#define DURACAO_MAX_ALARME ( 600 )	// Duração máxima do alarme igual a 10 minutos ( 600 segundos ).

// Protótipo das sub-rotinas.

void display( char );				//	Rotina de exibição dos dígitos no display.
void readkeys( void );				//	Rotina de leitura dos botões.

void HIGH_INT_VECTOR( void );		//	Vetor de interrupção de alta prioridade.
void HIGH_INT_ROUTINE( void );		//	Vetor de interrupção de baixa prioridade.
void LOW_INT_VECTOR( void );		//	Rotina de interrupção de alta prioridade.
void LOW_INT_ROUTINE( void );		//	Rotina de interrupção de baixa prioridade.

const rom char num2disp[]=			//	Vetor de codificação de dígito decimal em código de display.
 {									//	de 7 segmentos de catodo comum.
	0b00111111,					//	Dígito 0.
	0b00000110,					//	Dígito 1.
	0b01011011,					//	Dígito 2.
	0b01001111,					//	Dígito 3.
	0b01100110,					//	Dígito 4.
	0b01101101,					//	Dígito 5.
	0b01111101,					//	Dígito 6.
	0b00000111,					//	Dígito 7.
	0b01111111,					//	Dígito 8.
	0b01101111,					//	Dígito 9.
	0b00000000					//	BLANK.
 };


// Variáveis globais.

// Contadores dos dígitos BCD ( unidade e dezena, dos minutos e horas ) do horário atual.
char HoursCounterUnit= 0, HoursCounterDecade= 0;
char MinutesCounterUnit= 0,	MinutesCounterDecade= 0;

char SecondsCounter= 0;		// Contador de segundos.

//	Contadores dos dígitos BCD ( unidade e dezena, dos minutos e horas ) do horário do disparo
// 	do alarme.
char HoursAlarmUnit= 0, HoursAlarmDecade= 0;
char MinutesAlarmUnit= 0, MinutesAlarmDecade= 0;

char WakeUp= 0;				// 	Se WakeUp == 1, o alarme dispara.
char ShowAlarm= 0;			//	Flag para disparar o alarme.

char Blink=	DISABLED;		//	Blink = Flag para piscar o ponto.

unsigned int AlarmCounter= 0;	// Conta por quanto tempo o alarme está disparado.

char aguardo=	0;

void main( void )			//	Rotina principal do programa.
 {
	ADCON1=	0b00000111;		//	Pinos de I/O em modo digital.

	TRISA=	0b11011111;		//	Pino RA5 é saída ( Buzzer ).
	BUZZER=	0;				//	Buzzer inicialmente desligado.

	TRISB=	0b00001111;		//	Pinos RB0 a RB3 são entradas ( botões ).
							//	Pinos RB4 a RB7 são saídas ( habilitação dos displays ).

	LATB=	0b00001111;		//	Displays inicialmente desabilitados.

	TRISD=	0b00000000;		//	Todos os pinos associados aos segmentos são saídas.
	LATD=	0b00000000;		//	Todos os segmentos inicialmente apagados.

	WriteTimer0( deltaT0 );	//	Inicializa o Timer0.

	// Inicialização do Timer0:
	OpenTimer0( T0_SOURCE_INT 	&	//	Timer0 operando com fonte de clock interna.
				T0_PS_1_16		&	//	Timer0 com prescaler de 1:16.
				T0_16BIT );			//	Timer0 com 16 bits. ( Valor máximo de 65535 ).

	INTCONbits.TMR0IF=	0;	//	Resseta o flag de ocorrência de interrupções.
	INTCONbits.TMR0IE=	1;	//	Habilita interrupções do Timer0.
	INTCON2bits.TMR0IP=	1;	//	Interrupção do Timer0 com alta prioridade.

	WriteTimer2( 0 );		//	Inicializa o Timer2.

	PR2=	100;			//	Registrador PR2 com valor 100.
	// Inicialização do Timer2:
	OpenTimer2( T2_PS_1_1		&	//	Timer2 com prescaler em 1:1.
				T2_POST_1_10 );		//	Timer2 com postscaler em 1:10.


	PIR1bits.TMR2IF=	0;	//	Resseta o flag de interrupção do Timer2.
	PIE1bits.TMR2IE=	1;	//	Habilita o flag de interrupção do Timer2.
	IPR1bits.TMR2IP=	0;	//	Interrupção do Timer2 com baixa prioridade.

	INTCONbits.GIEH=	1;	//	Habilita chave geral de interrupções de alta prioridade.
	INTCONbits.GIEL=	1;	//	Habilita chave geral de interrupções de baixa prioridade.
	RCONbits.IPEN=		1;	//	Interrupções com dois níveis de prioridade.

	while ( 1 )				//	Loop infinito.
	 {
		display( ShowAlarm );	//	Invoca a rotina de exibição do display.

		readkeys();				//	Invoca a rotina de leitura dos botões.

		while ( ! aguardo );	// Aguarda a execução da rotina do Timer3.

		aguardo= 0;
	 }
 }


void display( char disp )					//	Rotina de exibição do display de 7 segmentos.
 {
	static char	dsCounter=	0;				//	Seleção do display.
	static char	blkCounter=	0;				//	Contador do ponto piscante.
	char		BLINKMASK;					//	Máscara do ponto piscante.

	char HoursUnit, HoursDecade;			//	Dígitos da unidade e dezena das horas.
	char MinutesUnit, MinutesDecade;		//	Dígitos da unidade e dezena dos minutos.

	if ( Blink )							//	Se o ponto piscante deve ser acionado.
	 {		
		BLINKMASK=	0b10000000;				//	Habilita a máscara do ponto piscante.
		if ( blkCounter++ > 15 )			//	Se passou o tempo do ponto piscante:
		 {
			blkCounter=	0;					//	Reinicia o contador.
			Blink=		DISABLED;			//	Desabilita o ponto.

			BLINKMASK=	0b00000000;			//	Desabilita a máscara do ponto piscante.
		 }
	 }

	if ( disp == TIME )						//	Se disp == TIME, exibe o horário atual.
 	 {
		HoursUnit= 	 HoursCounterUnit;
		HoursDecade= HoursCounterDecade;
		MinutesUnit= MinutesCounterUnit;
		MinutesDecade= MinutesCounterDecade;
	 }
	 else									//	Do contrário, exibe o horário do alarme.
	 {
		HoursUnit= 	HoursAlarmUnit;
		HoursDecade=HoursAlarmDecade;
		MinutesUnit=MinutesAlarmUnit;
		MinutesDecade= MinutesAlarmDecade;
	 }

	//	O código abaixo realiza a varredura dos displays de LEDs de 7 segmentos.
	//	A cada passagem por esse código, um único display é habilitado.
	//	A seleção do display habilitado é realizada pela variável dsCounter.

	switch ( dsCounter++ )					//	Seleciona um dígito para ser exibido.
	 {
		case	0:	dsHourDec=	DISABLED;					//	Desabilita o display anterior.
					PORTD=		num2disp[ MinutesUnit ];	//	Move o código do dígito para o display.
					dsMinUnit=	ENABLED;					//	Habilita o display.
					break;									//	Desvia para o fim do bloco switch-case.

		case	1:	dsMinUnit=	DISABLED;
					PORTD=		num2disp[ MinutesDecade ];
					dsMinDec=	ENABLED;
					break;

		case	2:	dsMinDec=	DISABLED;
					PORTD=		num2disp[ HoursUnit ] | BLINKMASK;	//	Display unidade das horas com ponto piscante. 
					dsHourUnit=	ENABLED;
					break;

		default:	dsHourUnit=	DISABLED;
					PORTD=		num2disp[ HoursDecade ];
					dsHourDec=	ENABLED;

					dsCounter=	0;					// No último display, reinicia a varredura.
	 }

 }

void readkeys( void )								//	Rotina de leitura dos botões.
 {
	static unsigned int Counter= 0;					//	Contador para o intervalo de leitura dos botões.

	if ( Counter++ < 500 )							//	Retorna enquanto o contador for inferior a 500.
		return;

	Counter= 0;										//	Reinicia o contador.

	if ( WakeUp )									//	Se o alarme foi acionado:
	 {
		if ( ~( PORTB | 0xF0 ) )					//	Qualquer botão pode desligar o alarme.
		 {
			WakeUp= 0;
			BUZZER= 0;
		 }

		return;										//	Retorna.
	 }

	if ( ! BTALARM && ! BTTIME )		//	Pressionar os botões TIME e ALARM simultaneamente
		return;										//	faz a rotina retornar.

	
	if ( ! BTALARM )							//	Se o botão pressionado foi ALARM:
 	 {
		if ( ! BTHOUR )						//	Juntamente com o botão de HORAS:
		 {
			if ( ++HoursAlarmUnit == 10 )			//	Incremento BCD dos dígitos das horas.
			 {
				HoursAlarmUnit= 0;
				HoursAlarmDecade++;
			 }
			if ( HoursAlarmDecade == 2 && HoursAlarmUnit == 4 )	//	Se o resultado for 24 horas:
			 {
				HoursAlarmDecade= 0;				//	Resseta ambos os dígitos das horas.
				HoursAlarmUnit=	  0;
			 }
		 }

		if ( ! BTMIN )						//	Se o botão ALARM for pressionado com o botão MINUTOS.
		 {
			if ( ++MinutesAlarmUnit == 10 )			//	Incremento BCD dos dígitos dos minutos.
			 {
				MinutesAlarmUnit= 0;
				if ( ++MinutesAlarmDecade == 6 )	//	Resseta em 60 minutos.
					MinutesAlarmDecade= 0;
			 }
		 }

		ShowAlarm=	ENABLED;						//	Habilita a exibição do horário do alarme.

		return;										//	Retorna.
	 }

	ShowAlarm= 	DISABLED;							//	Desabilita a exibição do horário do alarme,
													//	mostrando o horário atual.

	if ( ! BTTIME && ! BTHOUR )		//	Se forem pressionados TIME e HORA simultaneamente:
	 {
		if ( ++HoursCounterUnit == 10 )				//	Incremento BCD das horas.
		 {
			HoursCounterUnit= 0;
			HoursCounterDecade++;
		 }
		if ( HoursCounterDecade == 2 && HoursCounterUnit == 4 )	// Resseta ao atingir o valor de 24 hs.
		 {
			HoursCounterDecade= 0;
			HoursCounterUnit= 0;
		 }

		SecondsCounter= 0;							//	Resseta o contador de segundos.

		return;										//	Retorna.
	 }

	if ( ! BTTIME && ! BTMIN )		//	Se os botões TIME e MINUTOS forem pressionados simultaneamente:
	 {
		if ( ++MinutesCounterUnit == 10 )			//	Incremento BCD dos minutos.
		 {
			MinutesCounterUnit= 0;
			if ( ++MinutesCounterDecade == 6 )		//	Resseta quanto a dezena dos minutos atinge o valor 6 ( 60 min. ).
				MinutesCounterDecade= 0;

			SecondsCounter= 0;						//	Resseta o contador de segundos.

			return;									//	Retorna.
		 }
	 }

 }

#pragma code	HIGH_INT_VECTOR=	0x000008		//	Vetor de interrupção de alta prioridade.
void HIGH_INT_VECTOR( void )
 {
	_asm	GOTO	HIGH_INT_ROUTINE	_endasm
 }
#pragma code

#pragma code	LOW_INT_VECTOR=		0x000018		//	Vetor de interrupção de baixa prioridade.
void LOW_INT_VECTOR( void )
 {
	_asm	GOTO	LOW_INT_ROUTINE		_endasm
 }
#pragma code

#pragma interrupt	HIGH_INT_ROUTINE				//	Rotina de interrupção de alta prioridade.
void HIGH_INT_ROUTINE( void )
 {	
	if ( INTCONbits.TMR0IF )						//	Se a interrupção foi gerada pelo Timer0:
	 {				
		WriteTimer0( deltaT0 );						//	Reinicia a contagem do Timer0.
		INTCONbits.TMR0IF=	0;						//	Resseta o flag de interrupção.
		
		if ( ++SecondsCounter == 60 )				//	Incrementa o contador de segundos, até 60.
		 {
			SecondsCounter= 0;						//	Resseta o contador de segundos.
			if ( ++MinutesCounterUnit == 10 )		//	Incrementa a unidade dos minutos, até 10.
			 {
				MinutesCounterUnit= 0;				//	Resseta a unidade dos minutos.
				if ( ++MinutesCounterDecade == 6 )	//	Incrementa a dezena dos minutos, até 6.
				 {
					MinutesCounterDecade= 0;		//	Resseta a dezena dos minutos.
					if ( ++HoursCounterUnit == 10 )	//	Incrementa a unidade das horas até 10.
					 {
						HoursCounterUnit= 0;		//	Resseta a unidade das horas.
						HoursCounterDecade++;		//	Incrementa a dezena das horas.
					 }
					if ( HoursCounterDecade == 2 && HoursCounterUnit == 4 )	//	Resseta às 24 horas.
					 {
						HoursCounterDecade= 0;
						HoursCounterUnit= 0;
					 }
				 }
			 }
		 }

		Blink=	ENABLED;							//	Acende o ponto piscante.

		
		if ( WakeUp )								//	Se o alarme acabou de ser acionado:
		 {
			if ( ++AlarmCounter == DURACAO_MAX_ALARME )	//	Inicia a contagem do tempo do alarme.
			 {
				AlarmCounter= 0;					//	Resseta após o estouro.
				WakeUp=	0;
			 }
		 }

		// O código abaixo aciona o alarme quando o horário atual se iguala ao horário do alarme.
		if ( ! SecondsCounter )
			if ( HoursCounterDecade == HoursAlarmDecade &&
				 HoursCounterUnit	== HoursAlarmUnit	&&
				 MinutesCounterDecade == MinutesAlarmDecade &&
				 MinutesCounterUnit	== MinutesAlarmUnit )
			 {
				WakeUp=	1;
			 }	

	 }
 }

#pragma interruptlow	LOW_INT_ROUTINE					//	Rotina de interrupção de baixa prioridade.
void LOW_INT_ROUTINE( void )
 {
	static unsigned long int intermitance= 0;			//	Período de intermitência do alarme.

	if ( PIR1bits.TMR2IF )								//	Se a interrupção foi gerada pelo Timer2:
	 {
		if ( WakeUp && ( intermitance > 200 ) )			//	Se alarme acionado, o buzzer toca por 40% do período.
			BUZZER=	~BUZZER;							//	Inverte o estado do buzzer, para haver vibração e som.

		if ( ++intermitance == 500 )					//	Ao completar o período de 500, ele é reiniciado.
			intermitance= 0;

		aguardo=	1;

		PIR1bits.TMR2IF=	0;							//	Resseta o flag de ocorrência de interrupção.
	 }
 }