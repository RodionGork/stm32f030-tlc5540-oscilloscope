#ifndef __STM32F030_H_
#define __STM32F030_H_

#define REG_L(X,Y) ((volatile long*)((void*)((X) + (Y))))[0]
#define REG_B(X,Y) ((volatile char*)((void*)((X) + (Y))))[0]

#define TIM3_BASE 0x40000400

#define DMA_BASE 0x40020000
#define RCC_BASE 0x40021000

#define GPIOA_BASE 0x48000000
#define GPIOB_BASE 0x48000400
#define GPIOF_BASE 0x48001400

#define EXTI_BASE 0x40010400
#define SYSCFG_BASE 0x40010000
#define FLASH_BASE 0x40022000
#define NVIC_BASE 0xE000E100

#define USART_BASE 0x40013800
#define ADC_BASE 0x40012400

#define RCC_CR 0x00
#define RCC_CFGR 0x04
#define RCC_AHBENR 0x14
#define RCC_AHB2ENR 0x18
#define RCC_AHB1ENR 0x1C

#define DMA_ISR 0x00
#define DMA_IFCR 0x04
#define DMA_CCR(n) (0x08 + 20 * ((n) - 1))
#define DMA_CNDTR(n) (0x0C + 20 * ((n) - 1))
#define DMA_CPAR(n) (0x10 + 20 * ((n) - 1))
#define DMA_CMAR(n) (0x14 + 20 * ((n) - 1))

#define TIM_CR1 0x00
#define TIM_CR2 0x04
#define TIM_SMCR 0x08
#define TIM_DIER 0x0C
#define TIM_SR 0x10
#define TIM_EGR 0x14
#define TIM_CCMR1 0x18
#define TIM_CCMR2 0x1C
#define TIM_CCER 0x20
#define TIM_CNT 0x24
#define TIM_PSC 0x28
#define TIM_ARR 0x2C
#define TIM_CCR1 0x34
#define TIM_CCR2 0x38
#define TIM_CCR3 0x3C
#define TIM_CCR4 0x40

#define GPIO_MODER 0x00
#define GPIO_OTYPER 0x04
#define GPIO_SPEEDR 0x08
#define GPIO_IDR 0x10
#define GPIO_ODR 0x14
#define GPIO_BSRR 0x18
#define GPIO_AFRL 0x20
#define GPIO_AFRH 0x24

#define EXTI_IMR 0x00
#define EXTI_RTSR 0x08
#define EXTI_FTSR 0x0C
#define EXTI_PR 0x14

#define SYSCFG_EXTICR1 0x08

#define FLASH_ACR 0x00

#define NVIC_ISER 0x00

#define ADC_ISR 0x00
#define ADC_CR 0x08
#define ADC_CCR 0x308
#define ADC_DR 0x40
#define ADC_SMPR 0x14

#define ADC_CHSELR 0x28

#define USART_CR1 0x00
#define USART_BRR 0x0C
#define USART_RDR 0x24
#define USART_TDR 0x28
#define USART_ISR 0x1C


#endif


