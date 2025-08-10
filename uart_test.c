#include <avr/io.h>
#include <util/delay.h>

void uart_init() {
    UBRR0H = 0;
    UBRR0L = 103; // 9600 baud @16MHz
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_tx(char c) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = c;
}

void uart_print(const char* s) {
    while (*s) {
        uart_tx(*s++);
    }
}

int main(void) {
    uart_init();
    uart_print("UART test...\n");
    while (1) {
        uart_print("Hello!\n");
        _delay_ms(500);
    }
}

