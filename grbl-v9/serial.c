/*
  serial.c - Low level functions for sending and recieving bytes via the serial port
  Part of Grbl

  Copyright (c) 2011-2015 Sungeun K. Jeon
  Copyright (c) 2009-2011 Simen Svale Skogsrud

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "grbl.h"

uint8_t serial_rx_buffer[RX_BUFFER_SIZE];
uint8_t serial_rx_buffer_head = 0;
volatile uint8_t serial_rx_buffer_tail = 0;

uint8_t serial_tx_buffer[TX_BUFFER_SIZE];
uint8_t serial_tx_buffer_head = 0;
volatile uint8_t serial_tx_buffer_tail = 0;
  
// Returns the number of bytes used in the RX serial buffer.
uint8_t serial_get_rx_buffer_count()
{
  uint8_t rtail = serial_rx_buffer_tail; // Copy to limit multiple calls to volatile
  if (serial_rx_buffer_head >= rtail) { return(serial_rx_buffer_head-rtail); }
  return (RX_BUFFER_SIZE - (rtail-serial_rx_buffer_head));
}
// Returns the number of bytes used in the TX serial buffer.
// NOTE: Not used except for debugging and ensuring no TX bottlenecks.
uint8_t serial_get_tx_buffer_count()
{
  uint8_t ttail = serial_tx_buffer_tail; // Copy to limit multiple calls to volatile
  if (serial_tx_buffer_head >= ttail) { return(serial_tx_buffer_head-ttail); }
  return (TX_BUFFER_SIZE - (ttail-serial_tx_buffer_head));
}
//Set up the TX and RX pins for the UART on PIOA
void UART_PIO_Init(void)
{
	//Enable clock for the UART
	PMC->PMC_PCER0 = 1 << ID_UART;
	
	// Disable RX and TX PIO lines
	PIOA->PIO_PDR |= PIO_PA8A_URXD;
	PIOA->PIO_PDR |= PIO_PA9A_UTXD;
	//Set pins to use peripheral A(UART)
	PIOA->PIO_ABSR &= ~PIO_PA8A_URXD;
	PIOA->PIO_ABSR &= ~PIO_PA9A_UTXD;
	// Enable the pull up on the RX and TX pins
	PIOA->PIO_PUER |= PIO_PA8A_URXD;
	PIOA->PIO_PUER |= PIO_PA9A_UTXD;
}
//Set up the UART
void UART_Init(void)
{
	// Reset receiver & transmitter, on reset both are disabled
	UART->UART_CR = UART_CR_RSTRX | UART_CR_RSTTX;
	// Set the baud rate to 115200
	UART->UART_BRGR = F_CPU/(16*BAUD_RATE);	//DIVISOR = F_CPU/(16*BAUD)
	// Disable PMC controller
	UART->UART_PTCR = UART_PTCR_RXTDIS | UART_PTCR_TXTDIS;
	// No Parity
	UART->UART_MR = UART_MR_PAR_NO;
	// Clear all UART interrupts
	UART->UART_IDR = 0xFFFFFFFF;
	//Configure RX interrupt, TX is enabled when data is ready to be sent
	UART->UART_IER = UART_IER_RXRDY;
	NVIC_EnableIRQ((IRQn_Type) ID_UART);
	// Enable receiver and transmitter
	UART->UART_CR = (UART_CR_RXEN | UART_CR_TXEN);
}
//
void UART_Handler(void)
{
	uint8_t data, ht;
	
	// Check if the interrupt source is receive ready
	if((UART->UART_IMR & UART_IMR_RXRDY) && (UART->UART_SR & UART_SR_RXRDY))
	{
		// Save the character to the receive buffer
		data = UART->UART_RHR;
		// Pick off realtime command characters directly from the serial stream. These characters are
		// not passed into the buffer, but these set system state flag bits for realtime execution.
		switch (data) 
		{
			case CMD_STATUS_REPORT:	// Set as true 
				bit_true_atomic(sys_rt_exec_state, EXEC_STATUS_REPORT); 
				break;
			case CMD_CYCLE_START:	// Set as true   
				bit_true_atomic(sys_rt_exec_state, EXEC_CYCLE_START); 
				break;
			case CMD_FEED_HOLD:		// Set as true     
				bit_true_atomic(sys_rt_exec_state, EXEC_FEED_HOLD); 
				break; 
			case CMD_SAFETY_DOOR:	// Set as true   
				bit_true_atomic(sys_rt_exec_state, EXEC_SAFETY_DOOR); 
				break; 
			case CMD_RESET:			// Call motion control reset routine.         
				mc_reset(); 
				break; 
			default:				// Write character to buffer
			{
				ht = serial_rx_buffer_head + 1;
				if (ht == RX_BUFFER_SIZE) 
					ht = 0;
				// Write data to buffer unless it is full.
				if (ht != serial_rx_buffer_tail) 
				{
					serial_rx_buffer[serial_rx_buffer_head] = data;
					serial_rx_buffer_head = ht;
				}
					//TODO: else alarm on overflow?
			}
		}
	}
	// Check if the interrupt source is transmit ready
	if((UART->UART_IMR & UART_IMR_TXRDY) && (UART->UART_SR & UART_SR_TXRDY))
	{
		ht = serial_tx_buffer_tail;
		// Send a byte from the buffer
		UART->UART_THR = serial_tx_buffer[ht];
			
		// Update tail position
		ht++;
		if (ht == TX_BUFFER_SIZE) 
			ht = 0;
			
		serial_tx_buffer_tail = ht;
		// Turn off Data Register Empty Interrupt to stop TX-streaming 
		// if this concludes the transfer
		if (ht == serial_tx_buffer_head) 
			UART->UART_IDR = UART_IDR_TXRDY; 
	}
}
//
void serial_init()
{
	UART_PIO_Init();
	UART_Init();
}
// Writes one byte to the TX serial buffer. Called by main program.
// TODO: Check if we can speed this up for writing strings, rather than single bytes.
void serial_write(uint8_t data) 
{
  // Calculate next head
  uint8_t next_head = serial_tx_buffer_head + 1;
  
  if (next_head == TX_BUFFER_SIZE) 
    next_head = 0;

  // Wait until there is space in the buffer
  while (next_head == serial_tx_buffer_tail) 
  { 
    // TODO: Restructure st_prep_buffer() calls to be executed here during a long print.    
    if (sys_rt_exec_state & EXEC_RESET) 
	  return;	// Only check for abort to avoid an endless loop.
  }

  // Store data and advance head
  serial_tx_buffer[serial_tx_buffer_head] = data;
  serial_tx_buffer_head = next_head;
  //Enable the TX buffer empty interrupt
  UART->UART_IER = UART_IER_TXRDY;
}
// Fetches the first byte in the serial read buffer. Called by main program.
uint8_t serial_read()
{
  uint8_t tail = serial_rx_buffer_tail; // Temporary serial_rx_buffer_tail (to optimize for volatile)
  
  if (serial_rx_buffer_head == tail) 
    return SERIAL_NO_DATA;
  else 
  {
    uint8_t data = serial_rx_buffer[tail];
    
    tail++;
    if (tail == RX_BUFFER_SIZE) 
		tail = 0;
		
    serial_rx_buffer_tail = tail;
    return data;
  }
  return 0;
}
//
void serial_reset_read_buffer() 
{
  serial_rx_buffer_tail = serial_rx_buffer_head;
}
