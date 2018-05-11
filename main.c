#include "stm32f030.h"

#define SYS_CLK_MHZ 52
#define SAMPLES 2048

char hex[] = "0123456789ABCDEF";
char samples[SAMPLES];

int intDiv(int a, int b) {
    int res = 0;
    int power = 1;
    while (a - b >= b) {
        b <<= 1;
        power <<= 1;
    }
    while (power > 0) {
        if (a - b >= 0) {
            a -= b;
            res += power;
        }
        b >>= 1;
        power >>= 1;
    }
    return res;
}

void uartEnable(int divisor) {
    REG_L(GPIOA_BASE, GPIO_MODER) &= ~(3 << (9 * 2));
    REG_L(GPIOA_BASE, GPIO_MODER) |= (2 << (9 * 2));
    REG_L(GPIOA_BASE, GPIO_AFRH) &= ~(0xF << ((9 - 8) * 4));
    REG_L(GPIOA_BASE, GPIO_AFRH) |= (1 << ((9 - 8) * 4));
    REG_L(GPIOA_BASE, GPIO_MODER) &= ~(3 << (10 * 2));
    REG_L(GPIOA_BASE, GPIO_MODER) |= (2 << (10 * 2));
    REG_L(GPIOA_BASE, GPIO_AFRH) &= ~(0xF << ((10 - 8) * 4));
    REG_L(GPIOA_BASE, GPIO_AFRH) |= (1 << ((10 - 8) * 4));
    REG_L(RCC_BASE, RCC_AHB2ENR) |= (1 << 14);
    REG_L(USART_BASE, USART_BRR) = divisor;
    REG_L(USART_BASE, USART_CR1) |= 1;
    REG_L(USART_BASE, USART_CR1) |= (1 << 3);
}

void send(int c) {
    REG_L(USART_BASE, USART_TDR) = c;
    while ((REG_L(USART_BASE, USART_ISR) & (1 << 6)) == 0);
}

void sends(char* s) {
    while (*s) {
        send(*(s++));
    }
}

void sendHex(int x, int d) {
    while (d-- > 0) {
        send(hex[(x >> (d * 4)) & 0xF]);
    }
}

void sendDec(int x) {
    static char s[10];
    int i, x1;
    i = 0;
    while (x > 0) {
        x1 = intDiv(x, 10);
        s[i++] = x - x1 * 10;
        x = x1;
    }
    if (i == 0) {
        s[i++] = 0;
    }
    while (i > 0) {
        send('0' + s[--i]);
    }
}

void setupPorts() {
    char i;
    REG_L(RCC_BASE, RCC_AHBENR) |= (1 << 17) | (1 << 18); //port A, B

    REG_L(GPIOA_BASE, GPIO_SPEEDR) = (3 << (0 * 2)); // pa0 high-speed
    REG_L(GPIOA_BASE, GPIO_MODER) = (1 << (0 * 2)); // pa0 is output
    REG_L(GPIOB_BASE, GPIO_MODER) = (1 << (1 * 2)); // pb1 is output
}

void setupPll(int mhz) {
    int boost = mhz / 4 - 2;
    REG_L(RCC_BASE, RCC_CR) &= ~(1 << 24); // PLLON = 0
    while ((REG_L(RCC_BASE, RCC_CR) & (1 << 25)) != 0);
    REG_L(RCC_BASE, RCC_CFGR) = (boost & 0xF) << 18;
    REG_L(RCC_BASE, RCC_CR) |= (1 << 24); // PLLON = 1
    while ((REG_L(RCC_BASE, RCC_CR) & (1 << 25)) == 0);
    REG_L(RCC_BASE, RCC_CFGR) |= (1 << 1);
    while (((REG_L(RCC_BASE, RCC_CFGR) >> 2) & 0x3) != 2);
}

void measure(char* samples) {
    int i;
    REG_L(GPIOA_BASE, GPIO_BSRR) |= 16;
    for (i = 0; i < 20;) {
        REG_L(GPIOA_BASE, GPIO_BSRR) |= 1;
        i++;
        REG_L(GPIOA_BASE, GPIO_BSRR) |= 1 << 16;
    }
    asm volatile (
        ".syntax unified\r\n"
        "push {r1,r2,r3,r4,r5,r6,r7}\r\n"
        
        "mov r1, %[smp]\r\n"
        "ldr r2, =%[absrr]\r\n"
        "ldr r3, =%[aidr]\r\n"
        "movs r4, #1\r\n"
        "ldr r5, =1<<16\r\n"
        "ldr r6, =%[n]-1\r\n"
        
        ".align 8\r\n"
        "measure_l0:\r\n"
        "str r4, [r2]\r\n"
        "ldrb r7, [r3]\r\n"
        "strb r7, [r1,r6]\r\n"
        "str r5, [r2]\r\n"
        "subs r6, #1\r\n"
        "bpl measure_l0\r\n"
        
        "pop {r1,r2,r3,r4,r5,r6,r7}"
        :: [smp] "r" (samples), [absrr] "X" (GPIOA_BASE + GPIO_BSRR),
            [aidr] "X" (GPIOA_BASE + GPIO_IDR), [n] "X" (SAMPLES):
    );
}

int main(void) {
    int i, a;
    
    setupPll(SYS_CLK_MHZ);
    
    setupPorts();
    
    uartEnable(SYS_CLK_MHZ * 1000000 / 115200);
    
    a = 0;
    while(1) {
        measure(samples);
        for (i = SAMPLES - 1; i >= 0; i--) {
            sendHex(samples[i], 2);
            send(' ');
        }
        sends("\r\n");
        for (i = 0; i < 170000; i++) {
        }
        REG_L(GPIOB_BASE, GPIO_BSRR) |= 1 << (a + 1);
        a = 16 - a;
    }
}

