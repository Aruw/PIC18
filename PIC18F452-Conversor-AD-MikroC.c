
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
 unsigned long valor;                       // Valor convertido.
 char string[ 32 ];                         // String para texto.
 char centena, dezena, unidade, decimo;     // Dígitos a serem exibidos.
 unsigned long temp;

 ADCON1=   0b00001111;          // Canal analógico AN0 habilitado.
                                // Vref+ no pino RA3 ( entrada AN3 ).
                                // Vref- no pino RA2 ( entrada AN2 ).

                                // A tensão de referência é de 2 Volts.

 ADC_Init();                    // Inicializa o conversor AD interno.

 Lcd_Init();                    // Inicializa o display de LCD.

 Delay_ms( 100 );               // Atraso de 100 ms para estabilização.

 Lcd_Cmd( _LCD_CURSOR_OFF );    // Cursor do display oculto.

 Lcd_Out( 1, 1, "Celsius=" );   // Escreve "Celsius" na linha 1, coluna 1.
 Lcd_Out( 2, 1, "Fahr.= " );    // Escreve "Fahr." na linha 2, coluna 1.

 while ( 1 )                    // Loop infinito.
 {
       valor= ADC_Read( 0 );              // Realiza a conversão AD.

       valor*= 200000;                    // 2 Volts equivale ao valor 1023.
       valor/= 102300;

       temp= valor;                  // Copia o valor para conversão posterior.

       centena= valor / 1000;         // Obtém os dígitos.
       valor%= 1000;
       dezena= valor / 100;
       valor%= 100;
       unidade= valor / 10;           // unidade é quociente da divisão inteira.
       decimo= valor % 10;            // decimo é o módulo ( resto ) da divisão.

                // Os dígitos da centena e dezena apenas são escritos se não
                // forem zeros à esquerda, caso contrário escreve "espaço".
       string[ 0 ]= centena > 0 ? centena + '0' : ' ';
       string[ 1 ]= centena > 0 || dezena > 0 ? dezena + '0' : ' ';
       string[ 2 ]= unidade + '0';     // Obtém o caractere da unidade.
       string[ 3 ]= '.';               // Armazena o caractere ponto decimal.
       string[ 4 ]= decimo + '0';      // Obtém o caractere do valor decimal.
       string[ 5 ]= 0xDF;          // Armazena o caractere 'º' da unidade Volts.
       string[ 6 ]= 'C';
       string[ 7 ]= '\0';         // Finaliza com caractere de final de string.

       Lcd_Out( 1, 10, string );   // Escreve a string na coluna 10 da linha 1.

       valor= temp;                // Recupera o valor convertido.
       valor= valor * 180 / 100 + 320;   // Converte em graus Fahrenheit.


       centena= valor / 1000;        // Extrai os dígitos decimais.
       valor%= 1000;
       dezena= valor / 100;
       valor%= 100;
       unidade= valor / 10;          // unidade é quociente da divisão inteira.
       decimo= valor % 10;           // decimo é o módulo ( resto ) da divisão.

       string[ 0 ]= centena > 0 ? centena + '0' : ' ';
       string[ 1 ]= centena > 0 || dezena > 0 ? dezena + '0' : ' ';
       string[ 2 ]= unidade + '0';     // Obtém o caractere da unidade.
       string[ 3 ]= '.';               // Armazena o caractere ponto decimal.
       string[ 4 ]= decimo + '0';      // Obtém o caractere do valor decimal.
       string[ 5 ]= 0xDF;          // Armazena o caractere 'º' da unidade Volts.
       string[ 6 ]= 'F';
       string[ 7 ]= '\0';          // Finaliza com caractere de final de string.

       Lcd_Out( 2, 10, string );       // Escreve na linha 2, coluna 10.

       Delay_ms( 10 );                 // Atraso de 10 milissegundos.
 }

}