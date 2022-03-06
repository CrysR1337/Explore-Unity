#include "stm32f4xx.h"
#include "gun.h"

#include "timer.h"

void PWM_Configuration(void)
{
		// todo PWM  port pa9 tim1 
    TIM_OCInitTypeDef     TIM_OCInitStrue;
    TIM_TimeBaseInitTypeDef     tim;
	
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
		//RCC_APB2PeriphClockCmd(RCC_APBlPeriph_AFIO,ENABLE);
		RCC_APB2PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	
		Led_Configuration();	
	
		tim.TIM_Period = 0xFFFFFFFF;  //   ��ֵ��ȷ����д����
    tim.TIM_Prescaler = 168-1;	 //1M ��ʱ��  
    tim.TIM_ClockDivision = TIM_CKD_DIV1;	
    tim.TIM_CounterMode = TIM_CounterMode_Up;  
    TIM_TimeBaseInit(TIM3, &tim);
	
		TIM_OCInitStrue.TIM_OCMode=TIM_OCMode_PWM2;        // PWM??2:CNT>CCR?????
    TIM_OCInitStrue.TIM_OCPolarity=TIM_OCPolarity_High;// ????-??????
    TIM_OCInitStrue.TIM_OutputState=TIM_OutputState_Enable;// ????
    TIM_OC2Init(TIM3,&TIM_OCInitStrue);
	
		TIM_OC2PreloadConfig(TIM3,TIM_OCPreload_Enable);
		
		TIM_Cmd(TIM3,ENABLE);
}



//void PWM_Configuration(void)
//{
//	// todo PWM  port pa9 tim1 
//	//debugģʽ�ǲ��Ǳ���Ҫ����ʲô��������ʹ�ð������������������һ�´�����û�����⣬���Ƿ��ֻ��ǲ�������
//	//ʹ��һ��ʱ���ǲ��Ǿ��Ǽ����ʱ�ӵ���˼��
//	//����TIMʱ�ӵ�Ԥ��Ƶֵ�����ϼ���ģʽ��ʲô��˼
//	//nvic.NVIC_IRQChannel  = Ӧ������ʲô��ʵ��û���ҵ�
//	//û���ҵ�usart�������ݲ����ܵ��ĵ�
//}
