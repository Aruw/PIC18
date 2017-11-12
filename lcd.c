// Definições das rotinas do display de LCD 16x2 para placa McLab2 ( clock de 4 MHz ).

#include <delays.h>

#include "lcd.h"


#if  LCD_MACROS

#else
// As rotinas abaixo foram substituídas por macros no arquivo lcd.h .

void lcd_cmd( unsigned char cmd )
 {
	RS=	0;
	LATD=	cmd;
	EN=	1;
	Delay10TCYx( 1 );
	EN=	0;
	Delay10TCYx( 4 );
 }

void lcd_putc( unsigned char chr )
 {
	RS=	1;
	LATD=	chr;
	EN=	1;
	Delay10TCYx( 1 );
	EN=	0;
	Delay10TCYx( 4 );
 }

#endif

void lcd_init( void )
 {
	lcd_cmd( CURSOR_8285 );
	Delay10TCYx( 4 );

	lcd_cmd( CLEAR );
	Delay100TCYx( 16 );

	lcd_cmd( CURSOR_OFF );

	lcd_cmd( CURSOR_SHR );

	lcd_cmd( CUR_LINE1 );
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
	Delay10TCYx( 4 );
 }

void lcd_msg_shl( void )
 {
	lcd_cmd( MSG_SHL );
	Delay10TCYx( 4 );
 }

void lcd_cursor_shr( void )
 {
	lcd_cmd( CURSOR_SHR );
	Delay10TCYx( 4 );
 }

void lcd_cursor_shl( void )
 {
	lcd_cmd( CURSOR_SHL );
	Delay10TCYx( 4 );
 }

void lcd_cursor_init( void )
 {
	lcd_cmd( CURSOR_INIT );
	Delay10TCYx( 160 );
 }

void lcd_cursor_pos( char pos )
 {
	lcd_cmd( pos );
 }

void lcd_clear( void )
{
	lcd_cmd( CURSOR_CLR );
	Delay10TCYx( 160 );
}
