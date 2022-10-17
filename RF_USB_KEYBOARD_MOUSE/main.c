
/**************************************************************
작성자 : KKS
참고   : TEENSY 2.0 KEYBOARD PROJECT

MCU :   ATmega32u4
BOARD : ARDUINO LEONARDO 
H/W   : 5V 
CLOCK : 16Mhz
 
주요 기능
1. USB 통신
2. HID2.0 KEYBOARD
3. HID2.0 MOUSE
4. NRF24L01 무선 데이터 송수신    
**************************************************************/
#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "usb.h"
#include "spi.h"
#include "uart.h"
#include "nrf24l01.h"

#define RF_IRQ 1
#define LINK_PHASE 2
#define MOUSE_PHASE 3
#define KEYBOARD_PHASE 4
#define DO_NOTHING 0

//#define DEBUGMODE

volatile uint8_t phase = DO_NOTHING;
uint8_t key_rf_buffer[6] = {0,0,0,0,0,0};
int8_t  mouse_rf_buffer[6] = {0,0,0,0,0,0}; 
uint8_t address_rf_buffer[6] = {0,0,0,0,0,0};
	


/*
uint8_t number_keys[10]=
{KEY_0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9};
	
*/
int8_t const PROGMEM circle[] = {
	16, -1,
	15, -4,
	14, -7,
	13, -9,
	11, -11,
	9, -13,
	7, -14,
	4, -15,
	1, -16,
	-1, -16,
	-4, -15,
	-7, -14,
	-9, -13,
	-11, -11,
	-13, -9,
	-14, -7,
	-15, -4,
	-16, -1,
	-16, 1,
	-15, 4,
	-14, 7,
	-13, 9,
	-11, 11,
	-9, 13,
	-7, 14,
	-4, 15,
	-1, 16,
	1, 16,
	4, 15,
	7, 14,
	9, 13,
	11, 11,
	13, 9,
	14, 7,
	15, 4,
	16, 1
};

static uint8_t rx_buf[PAYLOAD_WIDTH];


int main(void)
{
    /* Replace with your application code */
	
	int8_t x, y, *p;
	uint8_t i;

	_delay_ms(100);
	FILE* fpStdio = fdevopen(usartTxChar,NULL);
	uart_init();
	SPI_Master_Init();
	printf("ATmega32u4_UART_INIT_COMPLETE\n");
	printf("ATmega32u4_SPI_INIT_COMPLETE\n");
	
	nrf24_pin_init();
	
	nrf24_init();
	nrf24_enable_irq();

	nrf24_dump_registers();
	ce_high();
		
	usb_init();
	while (!usb_configured()) /* wait */ ;
	_delay_ms(1000);

    while(1) 
    {
		/*
		 usb_keyboard_press(KEY_D,KEY_SHIFT);
		_delay_ms(2000);
		*/
		switch(phase)
		{
			case(DO_NOTHING):

			/*if(nrf24_receive(rx_buf, PAYLOAD_WIDTH)>0)
			{
				printf("RF_IRQ_STATUS");
				
				uint8_t* data[PAYLOAD_WIDTH] ; //POINTER ARRAY
				uint8_t i;
				
				for(i=0;i<8;i++)
				{
					data[i] = &(rx_buf[i]);
					printf("data[%d]: %d",i,*(data[i]));
				}
			}*/
			break;
			
			/**********************************************************************
										 RF IRQ SECTION 
			**********************************************************************/				   
			case(RF_IRQ):
			
				//nrf24_disable_irq();
				printf("RF_IRQ_STATUS!\n");
				if(nrf24_receive(rx_buf, PAYLOAD_WIDTH)>0)
				{
					printf("RF_IRQ_STATUS");
							
					uint8_t* data[PAYLOAD_WIDTH] ; //POINTER ARRAY
					uint8_t i;
							
					for(i=0;i<8;i++)
					{
						data[i] = &(rx_buf[i]);
						printf("data[%d]: %d",i,*(data[i]));
					}
					#ifdef DEBUGMODE
					printf("Buffer full!\n");
					#endif
							
					if(*(data[0])==0x01)
					{
						switch(*(data[1])) //KEY_DATA
						{
							case 0x01:  
										
								for(i=0;i<6;i++)
								{
									key_rf_buffer[i] = *(data[i+2]);
								}
										
								usb_keyboard_rf_send(key_rf_buffer,0);
						
							break;
									
							case 0x02:  
										
								for(i=0;i<6;i++)
								{
									mouse_rf_buffer[i] = *(data[i+2]);
								}
										
								usb_keyboard_rf_send(&mouse_rf_buffer[0],0);
									
							break;
									
							case 0x03:  
										
								for(i=0;i<6;i++)
								{
									address_rf_buffer[i] = *(data[i+2]);
								}
										
								nrf24_change_add();
							break;
									
							default:
							break;
									
								}
							}
							
							
							
				//nrf24_enable_irq();
				phase = DO_NOTHING;			
			break;
			
			
			default:
			break;
		}//switch
		/*
		_delay_ms(1000);
		
		p = circle;
		for (i=0; i<36; i++) {
			x = pgm_read_byte(p++);
			y = pgm_read_byte(p++);
			usb_mouse_move(x, y, 0);
			_delay_ms(20);
		}
		_delay_ms(9000);
		*/
				
			
			 
				}
    }//while
}//main



void loop(void)
{
	if (nrf24_receive(rx_buf, PAYLOAD_WIDTH) > 0)
	{
		uint16_t     *data = (uint16_t *)rx_buf;
		printf("\x1B[25DData = %5d", *data);
	}
}

ISR(INT0_vect) //WHEN IRQ LOW EDGE
{
	printf("IRQ_OCCUR!\n");
	phase = RF_IRQ;
}

