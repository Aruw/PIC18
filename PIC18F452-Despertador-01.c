/*
		Rel�gio Despertador usando a placa McLab2 com o microcontrolador PIC18F452.

		Disciplina: Microprocessadores e Microcontroladores.
		prof.:		Giovanni Rizzo.
*/
#include <P18F452.h>				// 	Define o microcontrolador utilizado.

#include <timers.h>					//	Inclui as rotinas e defini��es dos timers.
#include <delays.h>					//	Inclui as rotinas de delays.

#pragma config OSC=		XT			//	Oscilador em modo XT.
#pragma config WDT=		OFF			//	Wathdog-Timer desabilitado.
#pragma config LVP=		OFF			//	Low Voltage Programming desabilitado.

#define	BUZZER		LATAbits.LATA5	//	Buzzer no pino RA5.

// Bot�es

#define	BTHOUR		PORTBbits.RB0	//	Bot�o de ajuste ( incremento ) das horas.
#define	BTMIN		PORTBbits.RB1	//	Bot�o de ajuste ( incremento ) dos minutos.
#define	BTTIME		PORTBbits.RB2	//	Bot�o de sele��o do hor�rio atual.
#define	BTALARM		PORTBbits.RB3	//	Bot�o de sele��o do hor�rio do alarme.

// Habilita��o dos catodos dos displays.

#define	dsMinUnit	LATBbits.LATB4	//	Display do d�gito unidade dos minutos no pino RB4.
#define	dsMinDec	LATBbits.LATB5	//	Display do d�gito dezena dos minutos no pino RB5. 
#define	dsHourUnit	LATBbits.LATB6	//	Display do d�gito unidade das horas no pino RB6.
#define	dsHourDec	LATBbits.LATB7	//	Display do d�gito dezena das horas no pino RB7.

#define	ENABLED		1				//	ENABLED = HABILITADO ( n�vel l�gico alto ).	
#define	DISABLED	0				//	DISABLED = DESABILITADO ( n�vel l�gico baixo ).

#define ALARM	1					//	Quando disp == 1 ( ALARM ), exibe o hor�rio do alarme.
#define	TIME	0					//	Quando disp == 0 ( TIME ), exibe o hor�rio atual.

#define	deltaT0		(65536 - 62500)	//	N�mero de pulsos at� o estouro do Timer0.
									// 	Com fclock = 4 MHz, temos TCY = 1 us ...
									//	... e prescaler de 1:16 , temos:
									//	Tempo de Estouro = 62500 * 16 * TCY 
									//					 = 62500 * 16 * 1 us 
									//					 = 1000000 us.
									//					 = 1 segundo.

#define DURACAO_MAX_ALARME ( 600 )	// Dura��o m�xima do alarme igual a 10 minutos ( 600 segundos ).

// Prot�tipo das sub-rotinas.

void display( char );				//	Rotina de exibi��o dos d�gitos no display.
void readkeys( void );				//	Rotina de leitura dos bot�es.

void HIGH_INT_VECTOR( void );		//	Vetor de interrup��o de alta prioridade.
void HIGH_INT_ROUTINE( void );		//	Vetor de interrup��o de baixa prioridade.
void LOW_INT_VECTOR( void );		//	Rotina de interrup��o de alta prioridade.
void LOW_INT_ROUTINE( void );		//	Rotina de interrup��o de baixa prioridade.

const rom char num2disp[]=			//	Vetor de codifica��o de d�gito decimal em c�digo de display.
 {									//	de 7 segmentos de catodo comum.
	0b00111111,					//	D�gito 0.
	0b00000110,					//	D�gito 1.
	0b01011011,					//	D�gito 2.
	0b01001111,					//	D�gito 3.
	0b01100110,					//	D�gito 4.
	0b01101101,					//	D�gito 5.
	0b01111101,					//	D�gito 6.
	0b00000111,					//	D�gito 7.
	0b01111111,					//	D�gito 8.
	0b01101111,					//	D�gito 9.
	0b00000000					//	BLANK.
 };


// Vari�veis globais.

// Contadores dos d�gitos BCD ( unidade e dezena, dos minutos e horas ) do hor�rio atual.
char HoursCounterUnit= 0, HoursCounterDecade= 0;
char MinutesCounterUnit= 0,	MinutesCounterDecade= 0;

char SecondsCounter= 0;		// Contador de segundos.

//	Contadores dos d�gitos BCD ( unidade e dezena, dos minutos e horas ) do hor�rio do disparo
// 	do alarme.
char HoursAlarmUnit= 0, HoursAlarmDecade= 0;
char MinutesAlarmUnit= 0, MinutesAlarmDecade= 0;

char WakeUp= 0;				// 	Se WakeUp == 1, o alarme dispara.
char ShowAlarm= 0;			//	Flag para disparar o alarme.

char Blink=	DISABLED;		//	Blink = Flag para piscar o ponto.

unsigned int AlarmCounter= 0;	// Conta por quanto tempo o alarme est� disparado.

char aguardo=	0;

void main( void )			//	Rotina principal do programa.
 {
	ADCON1=	0b00000111;		//	Pinos de I/O em modo digital.

	TRISA=	0b11011111;		//	Pino RA5 � sa�da ( Buzzer ).
	BUZZER=	0;				//	Buzzer inicialmente desligado.

	TRISB=	0b00001111;		//	Pinos RB0 a RB3 s�o entradas ( bot�es ).
							//	Pinos RB4 a RB7 s�o sa�das ( habilita��o dos displays ).

	LATB=	0b00001111;		//	Displays inicialmente desabilitados.

	TRISD=	0b00000000;		//	Todos os pinos associados aos segmentos s�o sa�das.
	LATD=	0b00000000;		//	Todos os segmentos inicialmente apagados.

	WriteTimer0( deltaT0 );	//	Inicializa o Timer0.

	// Inicializa��o do Timer0:
	OpenTimer0( T0_SOURCE_INT 	&	//	Timer0 operando com fonte de clock interna.
				T0_PS_1_16		&	//	Timer0 com prescaler de 1:16.
				T0_16BIT );			//	Timer0 com 16 bits. ( Valor m�ximo de 65535 ).

	INTCONbits.TMR0IF=	0;	//	Resseta o flag de ocorr�ncia de interrup��es.
	INTCONbits.TMR0IE=	1;	//	Habilita interrup��es do Timer0.
	INTCON2bits.TMR0IP=	1;	//	Interrup��o do Timer0 com alta prioridade.

	WriteTimer2( 0 );		//	Inicializa o Timer2.

	PR2=	100;			//	Registrador PR2 com valor 100.
	// Inicializa��o do Timer2:
	OpenTimer2( T2_PS_1_1		&	//	Timer2 com prescaler em 1:1.
				T2_POST_1_10 );		//	Timer2 com postscaler em 1:10.


	PIR1bits.TMR2IF=	0;	//	Resseta o flag de interrup��o do Timer2.
	PIE1bits.TMR2IE=	1;	//	Habilita o flag de interrup��o do Timer2.
	IPR1bits.TMR2IP=	0;	//	Interrup��o do Timer2 com baixa prioridade.

	INTCONbits.GIEH=	1;	//	Habilita chave geral de interrup��es de alta prioridade.
	INTCONbits.GIEL=	1;	//	Habilita chave geral de interrup��es de baixa prioridade.
	RCONbits.IPEN=		1;	//	Interrup��es com dois n�veis de prioridade.

	while ( 1 )				//	Loop infinito.
	 {
		display( ShowAlarm );	//	Invoca a rotina de exibi��o do display.

		readkeys();				//	Invoca a rotina de leitura dos bot�es.

		while ( ! aguardo );	// Aguarda a execu��o da rotina do Timer3.

		aguardo= 0;
	 }
 }


void display( char disp )					//	Rotina de exibi��o do display de 7 segmentos.
 {
	static char	dsCounter=	0;				//	Sele��o do display.
	static char	blkCounter=	0;				//	Contador do ponto piscante.
	char		BLINKMASK;					//	M�scara do ponto piscante.

	char HoursUnit, HoursDecade;			//	D�gitos da unidade e dezena das horas.
	char MinutesUnit, MinutesDecade;		//	D�gitos da unidade e dezena dos minutos.

	if ( Blink )							//	Se o ponto piscante deve ser acionado.
	 {		
		BLINKMASK=	0b10000000;				//	Habilita a m�scara do ponto piscante.
		if ( blkCounter++ > 15 )			//	Se passou o tempo do ponto piscante:
		 {
			blkCounter=	0;					//	Reinicia o contador.
			Blink=		DISABLED;			//	Desabilita o ponto.

			BLINKMASK=	0b00000000;			//	Desabilita a m�scara do ponto piscante.
		 }
	 }

	if ( disp == TIME )						//	Se disp == TIME, exibe o hor�rio atual.
 	 {
		HoursUnit= 	 HoursCounterUnit;
		HoursDecade= HoursCounterDecade;
		MinutesUnit= MinutesCounterUnit;
		MinutesDecade= MinutesCounterDecade;
	 }
	 else									//	Do contr�rio, exibe o hor�rio do alarme.
	 {
		HoursUnit= 	HoursAlarmUnit;
		HoursDecade=HoursAlarmDecade;
		MinutesUnit=MinutesAlarmUnit;
		MinutesDecade= MinutesAlarmDecade;
	 }

	//	O c�digo abaixo realiza a varredura dos displays de LEDs de 7 segmentos.
	//	A cada passagem por esse c�digo, um �nico display � habilitado.
	//	A sele��o do display habilitado � realizada pela vari�vel dsCounter.

	switch ( dsCounter++ )					//	Seleciona um d�gito para ser exibido.
	 {
		case	0:	dsHourDec=	DISABLED;					//	Desabilita o display anterior.
					PORTD=		num2disp[ MinutesUnit ];	//	Move o c�digo do d�gito para o display.
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

					dsCounter=	0;					// No �ltimo display, reinicia a varredura.
	 }

 }

void readkeys( void )								//	Rotina de leitura dos bot�es.
 {
	static unsigned int Counter= 0;					//	Contador para o intervalo de leitura dos bot�es.

	if ( Counter++ < 500 )							//	Retorna enquanto o contador for inferior a 500.
		return;

	Counter= 0;										//	Reinicia o contador.

	if ( WakeUp )									//	Se o alarme foi acionado:
	 {
		if ( ~( PORTB | 0xF0 ) )					//	Qualquer bot�o pode desligar o alarme.
		 {
			WakeUp= 0;
			BUZZER= 0;
		 }

		return;										//	Retorna.
	 }

	if ( ! BTALARM && ! BTTIME )		//	Pressionar os bot�es TIME e ALARM simultaneamente
		return;										//	faz a rotina retornar.

	
	if ( ! BTALARM )							//	Se o bot�o pressionado foi ALARM:
 	 {
		if ( ! BTHOUR )						//	Juntamente com o bot�o de HORAS:
		 {
			if ( ++HoursAlarmUnit == 10 )			//	Incremento BCD dos d�gitos das horas.
			 {
				HoursAlarmUnit= 0;
				HoursAlarmDecade++;
			 }
			if ( HoursAlarmDecade == 2 && HoursAlarmUnit == 4 )	//	Se o resultado for 24 horas:
			 {
				HoursAlarmDecade= 0;				//	Resseta ambos os d�gitos das horas.
				HoursAlarmUnit=	  0;
			 }
		 }

		if ( ! BTMIN )						//	Se o bot�o ALARM for pressionado com o bot�o MINUTOS.
		 {
			if ( ++MinutesAlarmUnit == 10 )			//	Incremento BCD dos d�gitos dos minutos.
			 {
				MinutesAlarmUnit= 0;
				if ( ++MinutesAlarmDecade == 6 )	//	Resseta em 60 minutos.
					MinutesAlarmDecade= 0;
			 }
		 }

		ShowAlarm=	ENABLED;						//	Habilita a exibi��o do hor�rio do alarme.

		return;										//	Retorna.
	 }

	ShowAlarm= 	DISABLED;							//	Desabilita a exibi��o do hor�rio do alarme,
													//	mostrando o hor�rio atual.

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

	if ( ! BTTIME && ! BTMIN )		//	Se os bot�es TIME e MINUTOS forem pressionados simultaneamente:
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

#pragma code	HIGH_INT_VECTOR=	0x000008		//	Vetor de interrup��o de alta prioridade.
void HIGH_INT_VECTOR( void )
 {
	_asm	GOTO	HIGH_INT_ROUTINE	_endasm
 }
#pragma code

#pragma code	LOW_INT_VECTOR=		0x000018		//	Vetor de interrup��o de baixa prioridade.
void LOW_INT_VECTOR( void )
 {
	_asm	GOTO	LOW_INT_ROUTINE		_endasm
 }
#pragma code

#pragma interrupt	HIGH_INT_ROUTINE				//	Rotina de interrup��o de alta prioridade.
void HIGH_INT_ROUTINE( void )
 {	
	if ( INTCONbits.TMR0IF )						//	Se a interrup��o foi gerada pelo Timer0:
	 {				
		WriteTimer0( deltaT0 );						//	Reinicia a contagem do Timer0.
		INTCONbits.TMR0IF=	0;						//	Resseta o flag de interrup��o.
		
		if ( ++SecondsCounter == 60 )				//	Incrementa o contador de segundos, at� 60.
		 {
			SecondsCounter= 0;						//	Resseta o contador de segundos.
			if ( ++MinutesCounterUnit == 10 )		//	Incrementa a unidade dos minutos, at� 10.
			 {
				MinutesCounterUnit= 0;				//	Resseta a unidade dos minutos.
				if ( ++MinutesCounterDecade == 6 )	//	Incrementa a dezena dos minutos, at� 6.
				 {
					MinutesCounterDecade= 0;		//	Resseta a dezena dos minutos.
					if ( ++HoursCounterUnit == 10 )	//	Incrementa a unidade das horas at� 10.
					 {
						HoursCounterUnit= 0;		//	Resseta a unidade das horas.
						HoursCounterDecade++;		//	Incrementa a dezena das horas.
					 }
					if ( HoursCounterDecade == 2 && HoursCounterUnit == 4 )	//	Resseta �s 24 horas.
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
				AlarmCounter= 0;					//	Resseta ap�s o estouro.
				WakeUp=	0;
			 }
		 }

		// O c�digo abaixo aciona o alarme quando o hor�rio atual se iguala ao hor�rio do alarme.
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

#pragma interruptlow	LOW_INT_ROUTINE					//	Rotina de interrup��o de baixa prioridade.
void LOW_INT_ROUTINE( void )
 {
	static unsigned long int intermitance= 0;			//	Per�odo de intermit�ncia do alarme.

	if ( PIR1bits.TMR2IF )								//	Se a interrup��o foi gerada pelo Timer2:
	 {
		if ( WakeUp && ( intermitance > 200 ) )			//	Se alarme acionado, o buzzer toca por 40% do per�odo.
			BUZZER=	~BUZZER;							//	Inverte o estado do buzzer, para haver vibra��o e som.

		if ( ++intermitance == 500 )					//	Ao completar o per�odo de 500, ele � reiniciado.
			intermitance= 0;

		aguardo=	1;

		PIR1bits.TMR2IF=	0;							//	Resseta o flag de ocorr�ncia de interrup��o.
	 }
 }