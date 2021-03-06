#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "steering_engine.h"
#include "uart.h"
#include "utils.h"

void LED_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

int led_test(void)
{
	LED_init();
	delay_ms(1000);
	while(1)
	{
		GPIO_SetBits(GPIOC,GPIO_Pin_13);
		delay_ms(1000);
		GPIO_ResetBits(GPIOC,GPIO_Pin_13);
		delay_ms(1000);
	}
}

void serial_test(void)
{
	u8 i;
	u8 on[] = "on";
	u8 off[] = "off";
	uint16_t order;
	LED_init();
	USART1_config();
	while(1)
	{
		// receive code snippet
		{
			while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != SET);
			order = USART_ReceiveData(USART1);
		}
		if (order == 0)
		{
			GPIO_SetBits(GPIOC, GPIO_Pin_13);
			// USART_ClearFlag(USART1, USART_FLAG_TC);
			for (i = 0; i < sizeof(off)/sizeof(off[0]); i++)
			{
				USART_SendData(USART1, off[i]);
				while(USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET); // wait send data completely
			}
		}
		else
		{
			GPIO_ResetBits(GPIOC, GPIO_Pin_13);
			// USART_ClearFlag(USART1, USART_FLAG_TC);
			for (i = 0; i < sizeof(on)/sizeof(on[0]); i++)
			{
				USART_SendData(USART1, on[i]);
				while(USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET); // wait send data completely
			}
		}
	}

}

/*******************
IN1(A) ---- PB5
IN2(B) ---- PB6
IN3(C) ---- PB7
IN4(D) ---- PB8
+   ---- +5V
-   ---- GND

正转 电机导通相序 A-B-C-D
反转 电机导通相序 D-C-B-A
*********************/
void streeing_engine_test(void)
{
	int i = 0;
	int speed = 1500; // speed 实际为脉冲的间隔时间，因此 speed 越大，转速越慢
					  // 实测表明 1500 为最快值, 有 bug，1400时，反转正常，正转不行。囧
	StreeingEngine engine;
	StreeingEngine_init(&engine, GPIOB, RCC_APB2Periph_GPIOB, GPIO_Pin_5, GPIO_Pin_6, GPIO_Pin_7, GPIO_Pin_8);
	LED_init();
	
	delay_ms(225);
	while(1)
	{
		GPIO_ResetBits(GPIOC, GPIO_Pin_13);
		for (i = 0; i < 64 * 64 / 8; i++)
		{
			StreeingEngine_clockwise(&engine, speed);
			delay_us(speed + 10);
		}
		StreeingEngine_stop(&engine);
		GPIO_SetBits(GPIOC, GPIO_Pin_13);
		delay_ms(1000);
		GPIO_ResetBits(GPIOC, GPIO_Pin_13);
		for (i = 0; i < 64 * 64 / 8; i++)
		{
			StreeingEngine_counterClockwise(&engine, speed);
			delay_us(speed + 10);
		}
		StreeingEngine_stop(&engine);
		GPIO_SetBits(GPIOC, GPIO_Pin_13);
		delay_ms(1000);
	}
}

int main()
{
//	led_test();
//	serial_test();
	streeing_engine_test();
	return 0;
}
