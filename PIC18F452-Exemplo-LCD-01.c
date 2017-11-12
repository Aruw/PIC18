/*

			ESSE PROGRAMA USA AS ROTINAS DE 2010.

*/


#include <P18F452.h>


#include <delays.h>

#pragma config OSC=	XT
#pragma config WDT=	OFF

#define	RS	LATEbits.LATE0
#define	EN	LATEbits.LATE1
#define RW	LATEbits.LATE2

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

void lcd_cmd( unsigned char cmd );
void lcd_putc( unsigned char chr );
void lcd_8x5init( void );
void lcd_rom_puts( const rom char *string );
void lcd_ram_puts( ram char *string );
void lcd_msg_shr( void );
void lcd_msg_shl( void );
void lcd_cursor_shr( void );
void lcd_cursor_shl( void );
void lcd_cursor_init( void );
void lcd_cursor_pos( char pos );


void main( void )
{

	ADCON1=	0b00000111;			//	Pinos de I/O em modo digital.

	TRISD=	0x00;
	TRISE=	0b00000100;

	lcd_8x5init();
	lcd_cmd( CUR_LINE1 );
	lcd_rom_puts( "FUNCIONA FILHA DA PUTA" );

	while ( 1 );
	while ( 1 );
}

void lcd_cmd( unsigned char cmd )
 {
	RS=	0;
	PORTD=	cmd;
	EN=	1;
	Delay10TCYx( 1 );
	EN=	0;
	Delay10TCYx( 4 );
 }

//void escreve_lcd( unsigned char caractere )
// {
//  RS= 1;
//  PORTD= caractere;
//  EN= 1;
//  Delay10TCYx( 1 );
//  EN= 0;
//  Delay10TCYx( 4 );	
// }
//

void lcd_putc( unsigned char chr )
 {
	RS=	1;
	PORTD=	chr;
	EN=	1;
	Delay10TCYx( 1 );
	EN=	0;
	Delay10TCYx( 5 );
 }

//void inicializa_lcd( void )
// { 
//  comando_lcd( 0x30 );
//  Delay1KTCYx( 4 );
//
//  comando_lcd( 0x30 );
//  Delay10TCYx( 10 );
//
//  comando_lcd( 0x30 );
//
//  comando_lcd( 0x38 );
//
//  comando_lcd( 0x01 );
//  Delay1KTCYx( 2 );
//
//  comando_lcd( 0x0C );
//
//  comando_lcd( 0x06 );
// }
//

void lcd_8x5init( void )
 {
	lcd_cmd( 0x30 );
	Delay10TCYx( 4 );

	lcd_cmd( 0x38 );
	Delay10TCYx( 4 );

	lcd_cmd( CLEAR );
	Delay1KTCYx( 2 );

	lcd_cmd( CURSOR_OFF );

	lcd_cmd( CURSOR_SHR );
 }


//void escreve_frase( const rom char *frase )
// {
//  do
//    escreve_lcd( *frase );
//    while ( *++frase );
// }

void lcd_rom_puts( const rom char *string )
 {
	do
	 {
		lcd_putc( *string );
	 } while ( *++string );
 }

void lcd_ram_puts( ram char *string )
 {
	do
	 {
		lcd_putc( *string );
	 } while ( *++string );
 }

void lcd_msg_shr( void )
 {
	lcd_cmd( MSG_SHR );
	Delay10TCYx( 4 );
 }

void lcd_msg_shl( void )
 {
	lcd_cmd( MSG_SHL );
	Delay10TCYx( 4 );
 }

void lcd_cursor_shr( void )
 {
	lcd_cmd( 0x14 );//lcd_cmd( CURSOR_RIGHT );
	Delay10TCYx( 4 );
 }

void lcd_cursor_shl( void )
 {
	lcd_cmd( 0x10 );//lcd_cmd( CURSOR_LEFT );
	Delay10TCYx( 4 );
 }

void lcd_cursor_init( void )
 {
	lcd_cmd( 0x02 );//lcd_cmd( INIT_CURSOR );
	Delay10TCYx( 4 );
 }

void lcd_cursor_pos( char pos )
 {
	lcd_cmd( pos );
 }
/*
#include "lcd.h"

void lcd_cmd( unsigned char cmd )
 {
	RS=	0;
	PORTD=	cmd;
	EN=	1;
	Delay10TCYx( 10 );
	EN=	0;
	Delay10TCYx( 40 );
 }


void lcd_putc( unsigned char chr )
 {
	RS=	1;
	PORTD=	chr;
	EN=	1;
	Delay10TCYx( 10 );
	EN=	0;
	Delay10TCYx( 50 );
 }

void lcd_init( void )					//	Rotina de inicialização do display.
 {
	lcd_cmd( 0x30 );
	Delay1KTCYx( 4 );

	lcd_cmd( 0x30 );
	Delay100TCYx( 1 );

	lcd_cmd( 0x30 );
	Delay10TCYx( 1 );

	lcd_cmd( 0x38 );					//	Comunicação em 8 vias. Matriz 7x5.
	Delay10TCYx( 4 );

	lcd_cmd( CLEAR );					// Comando para limpar o display e posicionar o cursor na primeira
	Delay1KTCYx( 18 );					// linha e primeira coluna ( da esquerda ).

	lcd_cmd( CURSOR_OFF );
 }

void lcd_rom_puts( const rom char *string )
 {
	do
	 {
		lcd_putc( *string );
	 } while ( *++string );
 }

void lcd_ram_puts( ram char *string )
 {
	do
	 {
		lcd_putc( *string );
	 } while ( *++string );
 }

void lcd_msg_shr( void )
 {
	lcd_cmd( MSG_SHR );
	Delay10TCYx( 40 );
 }

void lcd_msg_shl( void )
 {
	lcd_cmd( MSG_SHL );
	Delay10TCYx( 40 );
 }

void lcd_cursor_shr( void )
 {
	lcd_cmd( CUR_SHR );
	Delay10TCYx( 40 );
 }

void lcd_cursor_shl( void )
 {
	lcd_cmd( CUR_SHL );
	Delay10TCYx( 40 );
 }

void lcd_cursor_init( void )
 {
	lcd_cmd( CUR_INIT );
	Delay10TCYx( 40 );
 }

void lcd_cursor_pos( char pos )
 {
	lcd_cmd( pos );
 }

*/