#include "r_syscall.h"
RSyscallPort sysport_x86[] = {
{ 0x3, "HELLO WORLD" },
{ 0x378, "lp1" },
{ 0, NULL }
};
RSyscallPort sysport_avr[] = {
{ 0x3f, "SREG: flags" },
{ 0x3e, "SPH: Stack higher bits SP8-SP10" },
{ 0x3d, "SPL: Stack lower bits SP0-SP7"},
{ 0x3c, "OCR0: Timer/Counter0 Output Compare Register."},
{ 0x3b, "GICR: General Interrupt Control Register" },
{ 0x3a, "GIFR: General Interrupt Flag Register"},
{ 0x39, "TIMSK: Timer/Counter Interrupt Mask"},
{ 0x38, "TIFR: Timer/Counter Interrupt Flag Register"},
{ 0x37, "SPMCR: Store Program Memory Control Register"},
{ 0x36, "TWCR: I2C (Two-wire) Control Register"},
{ 0x35, "MCUCR: MCU (Power Management) Control Register"},
{ 0x34, "MCUCSR: MCU Control and Status Register. Watchdog, Brown-out, Power-on..." },
{ 0x33, "TCCR0: Timer/Counter Control Register 0"},
{ 0x32, "TCNT0: Timer/Counter Register 0 (8 bits)"},
{ 0x31, "OSCCAL: (Internal) Oscillator Calibration Register"},
{ 0x30, "SFIOR: Special Function IO Register"},
{ 0x2f, "TCCR1A: Timer/Counter Control Register 1A (16 bits). Used for (fast) PWM."},
{ 0x2e, "TCCR1B: Timer/Counter Control Register 1B (16 bits). PWM mode select."},
{ 0x2d, "TCNT1H: Timer/Counter1 Register High byte."},
{ 0x2c, "TCNT1L: Timer/Counter1 Register Low byte."},
{ 0x2b, "OCR1AH: Timer/Counter1 Output Compare Register A High byte."},
{ 0x2a, "OCR1AL: Timer/Counter1 Output Compare Register A Low byte."},
{ 0x29, "OCR1BH: Timer/Counter1 Output Compare Register B High byte."},
{ 0x28, "OCR1BL: Timer/Counter1 Output Compare Register B Low byte."},
{ 0x27, "ICR1H: Timer/Counter1 Input Capture Register High byte."},
{ 0x26, "ICR1L: Timer/Counter1 Input Capture Register Low byte."},
{ 0x25, "TCCR2: Timer/Counter2 Control Register (8 bits)."},
{ 0x24, "TCNT2: Timer/Counter2 (8 bits)."},
{ 0x23, "OCR2: Timer/Counter2 Output Compare Register."},
{ 0x22, "ASSR: Asynchronous Operation of the Timer/Counter."},
{ 0x21, "WDTCR: Watchdog Timer Control Register."},
{ 0x20, "UBRRH, UCSRC: USART Baud Rate Registers, High byte and Control. A.k.a setting serial port speed."},
{ 0x1f, "EEARH: EEPROM Address Register High byte."},
{ 0x1e, "EEARL: EEPROM Address Register Low byte."},
{ 0x1d, "EEDR: EEPROM Data Register."},
{ 0x1c, "EECR: EEPROM Control Register."},
{ 0x1b, "PORTA: Output pins/pullups address for port A."},
{ 0x1a, "DDRA: Data Direction Register for Port A."},
{ 0x19, "PINA: Input Pins Address for Port A."},
{ 0x18, "PORTB: Output pins/pullups address for port B."},
{ 0x17, "DDRB: Data Direction Register for Port B."},
{ 0x16, "PINB: Input Pins Address for Port B."},
{ 0x15, "PORTC: Output pins/pullups address for port C."},
{ 0x14, "DDRC: Data Direction Register for Port C."},
{ 0x13, "PINC: Input Pins Address for Port C."},
{ 0x12, "PORTD: Output pins/pullups address for port D."},
{ 0x11, "DDRD: Data Direction Register for Port D."},
{ 0x10, "PIND: Input Pins Address for Port D."},
{ 0x0f, "SPDR: SPI Data Register."},
{ 0x0e, "SPSR: SPI Status Register."},
{ 0x0d, "SPCR: SPI Control Register."},
{ 0x0c, "UDR: USART I/O Data Register."},
{ 0x0b, "UCSRA: USART Control and Status Register A."},
{ 0x0a, "UCSRB: USART Control and Status Register B."},
{ 0x09, "UBRRL: USART Baud Rate Registers Low byte. A.k.a setting serial port speed."},
{ 0x08, "ACSR: Analog Comparator Control and Status Register."},
{ 0x07, "ADMUX: ADC Multiplexer Selection Register."},
{ 0x06, "ADCSRA: ADC Control and Status Register A."},
{ 0x05, "ADCH: ADC Data Register High byte."},
{ 0x04, "ADCL: ADC Data Register Low byte."},
{ 0x03, "TWDR: I2C (Two-wire) Serial Interface Data Register."},
{ 0x02, "TWAR: I2C (Two-wire) Serial Interface (Slave) Address Register."},
{ 0x01, "TWSR: I2C (Two-wire) Serial Interface Status Register."},
{ 0x00, "TWBR: I2C (Two-wire) Serial Interface Bit Rate Register."},
{ 0, NULL }
};
