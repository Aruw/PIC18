


#define HIGH(param)    ( (param) >> 8 )
#define LOW( param )    ( (param) & 0x0F )

#define PERIODO ( 256 - 2500 )               // Per�odo de 1 milissegundo.

sbit LCD_D4     at    RD4_bit;              // Display de LCD configurado com
sbit LCD_D5     at    RD5_bit;              // comunica��o por 4 vias no nibble
sbit LCD_D6     at    RD6_bit;              // alto do PORTD.
sbit LCD_D7     at    RD7_bit;

sbit LCD_RS     at    RB4_bit;              // Pino RS conectado ao pino RB4.
sbit LCD_EN     at    RB5_bit;              // Pino EN conectado ao pino RB5.

sbit LCD_D4_Direction at TRISD4_bit;        // LCD configurado para escrita.
sbit LCD_D5_Direction at TRISD5_bit;
sbit LCD_D6_Direction at TRISD6_bit;
sbit LCD_D7_Direction at TRISD7_bit;

sbit LCD_RS_Direction at TRISB4_bit;
sbit LCD_EN_Direction at TRISB5_bit;

char canal= 0;                         // Cont�m o canal anal�gico selecionado.
char string[ 6 ];                      // Armazena uma string de 6 caracteres.

unsigned valor;                        // Armazena o valor da convers�o AD.
char unidade, decimo;                  // Armazena os dois d�gitos.
volatile char exibir= 0;               // Permite a atualiza��o.

void interrupt( void ){                // Rotina de tratamento de interrup��o.
 if ( INTCON.TMR0IF )                  // Se interrup��o provocada pelo Timer0.
 {
   exibir= 1;                          // Permite uma nova atualiza��o.
   TMR0L=   PERIODO;
   INTCON.TMR0IF= 0;                   // Resseta o flag da interrup��o.
 }
}

void main( void )               // Rotina principal do programa.
{

 INTCON.TMR0IF= 0;              // Resseta o flag da interrup��o.
 INTCON.TMR0IE= 1;              // Habilita a interrup��o do Timer0.

 TMR0L=  LOW( PERIODO );        // Inicializa o contador do Timer0.
 TMR0H=  HIGH( PERIODO );

 INTCON.GIE=  1;                // Habilita interrup��es via chave geral.

 ADCON1=   0b00000111;

 LATD=    0x00;                // PORTD com todos os pinos ressetados.

 TRISD=    0x00;

 TRISB= 0xFF;

 Lcd_Init();                    // Inicializa o display de LCD.

 Lcd_Cmd( _LCD_CURSOR_OFF );    // Cursor desabilitado.
 Lcd_Out( 1, 1, "P0=     P1=" );   // Envia texto para a linha 1 do display.
 Lcd_Out( 2, 1, "P2=     P3=" );   // Envia texto para a linha 2 do display.

 while ( 1 )                       // La�o infinito principal do programa.
 {
  if ( exibir )                    // Se ocorreu uma interrup��o do Timer0.
  {
   TMR0L= LOW(PERIODO);                  // Reinicia o contador do Timer0.
   TMR0H= HIGH( PERIODO );

   valor= ADC_Read( 1 );       // Realiza a convers�o anal�gico-digital
                                   // para o canal especificado.
   valor*= 50;                     // Mapeia no intervalo 0 a 50.
   valor/= 1023;

   unidade= valor / 10;            // unidade � quociente da divis�o inteira.
   decimo= valor % 10;             // decimo � o m�dulo ( resto ) da divis�o.

   string[ 0 ]= unidade + '0';     // Obt�m o caractere da unidade.
   string[ 1 ]= '.';               // Armazena o caractere ponto decimal.
   string[ 2 ]= decimo + '0';      // Obt�m o caractere do valor decimal.
   string[ 3 ]= 'V';               // Armazena o caractere 'V' da unidade Volts.
   string[ 4 ]= '\0';              // Finaliza com caractere de final de string.

   Lcd_Out( 1, 4, string );


    exibir= 0;                     // Desabilita a permiss�o para exibir.
   }
 }
}