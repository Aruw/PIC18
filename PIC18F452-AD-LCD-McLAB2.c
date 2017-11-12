
/*
  EXPERIÊNCIA CONVERSOR ANALÓGICO-DIGITAL DO PIC18F452 - VOLTÍMETRO DIGITAL.
  
  
*/

sbit LCD_D4     at    RD4_bit;              // Display de LCD configurado com
sbit LCD_D5     at    RD5_bit;              // comunicação por 4 vias no nibble
sbit LCD_D6     at    RD6_bit;              // alto do PORTD.
sbit LCD_D7     at    RD7_bit;

sbit LCD_RS     at    RE0_bit;              // Pino RS conectado ao pino RB4.
sbit LCD_EN     at    RE1_bit;              // Pino EN conectado ao pino RB5.

sbit LCD_D4_Direction at TRISD4_bit;        // LCD configurado para escrita.
sbit LCD_D5_Direction at TRISD5_bit;
sbit LCD_D6_Direction at TRISD6_bit;
sbit LCD_D7_Direction at TRISD7_bit;

sbit LCD_RS_Direction at TRISE0_bit;
sbit LCD_EN_Direction at TRISE1_bit;


void main( void )                           // Função principal do programa.
{
 long valor;                                // Valor a ser convertido.
 char string[ 32 ];                         // String com texto a ser exibido.
 char unidade, decimo;                      // Dígitos da conversão.
 unsigned long temp;

 ADCON1=   0b00000100;                      // Canais analógicos 0, 1 e 3
                                            // habilitados.

 TRISA.TRISA1= 1;                           // Pino RA1 é entrada.

 ADC_Init();                                // Inicializa o conversor AD.

 Lcd_Init();                                // Inicializa o dieplsay de LCD.

 Delay_ms( 100 );                           // Atraso de 100 milissegundos.

 Lcd_Cmd( _LCD_CURSOR_OFF );                // Desabilita o cursor do display.

 Lcd_Out( 1, 1, "Conv. AD MikroC" );        // Mensagem da linha 1.
 Lcd_Out( 2, 1, "Tensao= " );               // Mensagem da linha 2.

 while ( 1 )
 {
       valor= ADC_Read( 1 );           // Realiza conversão AD no canal AN1.

       valor*= 5000;                   // Mapeia no invervalo 0 a 5000.
       valor/= 1023;
       
       temp= valor;                    // Armazena o valor convertido em temp.
       
       valor/= 100;                    // Remapeia no invertalo 0 a 50.
       
       if ( temp % 100 >= 50 )         // Deve arredondar para cima?
          valor++;
           
       unidade= valor / 10;            // unidade é quociente da divisão inteira.
       decimo= valor % 10;             // decimo é o módulo ( resto ) da divisão.

       string[ 0 ]= unidade + '0';     // Obtém o caractere da unidade.
       string[ 1 ]= '.';               // Armazena o caractere ponto decimal.
       string[ 2 ]= decimo + '0';      // Obtém o caractere do valor decimal.
       string[ 3 ]= 'V';               // Armazena o caractere 'V' da unidade Volts.
       string[ 4 ]= '\0';              // Finaliza com caractere de final de string.

       Lcd_Out( 2, 9, string );        // Escreve o valor convertido no display.

       Delay_ms( 10 );                 // Atraso de 10 milissegundos.
 }

}