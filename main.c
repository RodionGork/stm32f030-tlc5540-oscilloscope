#include "stm32f030.h"

#define SYS_CLK_MHZ 48
#define SAMPLES 3000

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

    REG_L(GPIOB_BASE, GPIO_SPEEDR) = (3 << (0 * 2)); // pa0 high-speed
    REG_L(GPIOA_BASE, GPIO_MODER) = (1 << (0 * 2)); // pa0 is output
    REG_L(GPIOB_BASE, GPIO_MODER) = (1 << (1 * 2)); // pb1 is output
}

void setupTimer3() {
    REG_L(GPIOB_BASE, GPIO_MODER) &= ~(3 << (0 * 2));
    REG_L(GPIOB_BASE, GPIO_MODER) |= (2 << (0 * 2)); // alternate function for pb0
    REG_L(GPIOB_BASE, GPIO_AFRL) &= ~(15 << (0 * 4));
    REG_L(GPIOB_BASE, GPIO_AFRL) |= (1 << (0 * 4)); // alternate function 1 (tim3_ch3)
    REG_L(RCC_BASE, RCC_AHB1ENR) |= (1 << 1); // enable clock to timer 3
    REG_L(TIM3_BASE, TIM_PSC) = 0;
    REG_L(TIM3_BASE, TIM_ARR) = 5;
    REG_L(TIM3_BASE, TIM_CCR3) = 3;
    REG_L(TIM3_BASE, TIM_EGR) != (1 << 3); // CC3 generation enable
    REG_L(TIM3_BASE, TIM_CCER) |= (1 << 8) | (1 << 9); // CC3 output enable
    REG_L(TIM3_BASE, TIM_CCMR2) = 0x68; // CC3 in PWM
    REG_L(TIM3_BASE, TIM_DIER) = (1 << 11); // DMA from CC3
    REG_L(TIM3_BASE, TIM_CR1) = 0x81; // enable preload and timer itself
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

void setupDma() {
    REG_L(RCC_BASE, RCC_AHBENR) |= (1 << 0);
}

void measureWithDma() {
    REG_L(DMA_BASE, DMA_IFCR) = (1 << (1 + (2 - 1) * 4));
    REG_L(DMA_BASE, DMA_CNDTR(2)) = SAMPLES;
    REG_L(DMA_BASE, DMA_CPAR(2)) = GPIOA_BASE + GPIO_IDR;
    REG_L(DMA_BASE, DMA_CMAR(2)) = (unsigned long) samples;
    REG_L(DMA_BASE, DMA_CCR(2)) = (3 << 12) | (1 << 7) | (1 << 0);
    asm volatile (
        ".syntax unified\r\n"
        "push {r1}\r\n"
        
        "ldr r1, =%[n]\r\n"
        "measure_l0:\r\n"
        "nop\r\n"
        "nop\r\n"
        "nop\r\n"
        "subs r1, #1\r\n"
        "bne measure_l0\r\n"
        
        "pop {r1}"
        :: [n] "X" (SAMPLES):
    );
    REG_L(DMA_BASE, DMA_CCR(2)) &= ~(1 << 0);
}

int main(void) {
    int i, a;
    
    setupPll(SYS_CLK_MHZ);
    
    setupPorts();
    
    setupTimer3();

    setupDma();

    uartEnable(SYS_CLK_MHZ * 1000000 / 115200);
    
    a = 0;
    while(1) {
        measureWithDma(samples);

        for (i = 0; i < SAMPLES; i++) {
            sendHex(samples[i], 2);
            send(' ');
        }
        sends("\r\n");
        for (i = 0; i < 1700000; i++) {
        }
        REG_L(GPIOB_BASE, GPIO_BSRR) |= 1 << (a + 1);
        a = 16 - a;
    }
}

