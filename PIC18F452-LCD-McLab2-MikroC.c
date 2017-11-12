
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

void main( void )
{
 ADCON1=   0b00000111;
 
 LATB=     0b00001001;
 TRISB=    0b11110000;

 Lcd_Init();
 
 Delay_ms( 100 );

 Lcd_Cmd( _LCD_CURSOR_OFF );

 Lcd_Out( 1, 3, "Teste LCD" );
 Lcd_Out( 2, 3, "MikroC PRO" );

 while ( 1 )
 {
  Lcd_Cmd( _LCD_SHIFT_LEFT );
  Delay_ms( 250 );
  Lcd_Cmd( _LCD_SHIFT_RIGHT );
  Delay_ms( 250 );
 }

}