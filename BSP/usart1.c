#include "FIFO.h"
#include "main.h"

/*-----USART1_RX-----PB7----*/ 

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/
static uint8_t _USART1_DMA_RX_BUF[2][BSP_USART1_DMA_RX_BUF_LEN];

static uint8_t _USART1_RX_BUF[BSP_USART1_RX_BUF_SIZE_IN_FRAMES * BSP_USART1_DMA_RX_BUF_LEN];

static FIFO_t  _USART1_RX_FIFO;

static RC_Raw_t rc_raw_data;	 //remote raw data 

void USART1_Configuration(uint32_t baud_rate)
{
    GPIO_InitTypeDef gpio;
	  USART_InitTypeDef usart;
	  NVIC_InitTypeDef nvic;
    DMA_InitTypeDef dma;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); 
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); 
    
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
    
    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = GPIO_Pin_10;
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &gpio);
    
    USART_DeInit(USART1);
    USART_StructInit(&usart);
    usart.USART_BaudRate = baud_rate;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_Even;
    usart.USART_Mode = USART_Mode_Rx;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &usart);
    
    USART1_FIFO_Init();
    
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
    
    DMA_DeInit(DMA2_Stream2);
    DMA_StructInit(&dma);
    dma.DMA_Channel = DMA_Channel_4;
    dma.DMA_PeripheralBaseAddr = (uint32_t)(&USART1->DR);
    dma.DMA_Memory0BaseAddr = (uint32_t)&_USART1_DMA_RX_BUF[0][0];
    dma.DMA_DIR = DMA_DIR_PeripheralToMemory;
    dma.DMA_BufferSize = sizeof(_USART1_DMA_RX_BUF)/2;
    dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    dma.DMA_Mode = DMA_Mode_Circular;
    dma.DMA_Priority = DMA_Priority_Medium;
    dma.DMA_FIFOMode = DMA_FIFOMode_Disable;
    dma.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
    dma.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    dma.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA2_Stream2, &dma);
    
    //配置Memory1,Memory0是第一个使用的Memory
    DMA_DoubleBufferModeConfig(DMA2_Stream2, (uint32_t)&_USART1_DMA_RX_BUF[1][0], DMA_Memory_0);   //first used memory configuration
    DMA_DoubleBufferModeCmd(DMA2_Stream2, ENABLE);
    
    DMA_Cmd(DMA2_Stream2, ENABLE);
    
    nvic.NVIC_IRQChannel = USART1_IRQn;                          
		nvic.NVIC_IRQChannelPreemptionPriority = 0;   //pre-emption priority 
		nvic.NVIC_IRQChannelSubPriority = 0;		    //subpriority 
		nvic.NVIC_IRQChannelCmd = ENABLE;			
		NVIC_Init(&nvic);	
    
		USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);        //usart rx idle interrupt  enabled
    USART_Cmd(USART1, ENABLE);

}

//串口接收中断服务函数
void USART1_IRQHandler(void)
{
	static uint32_t this_time_rx_len = 0;
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
	{
		//clear the idle pending flag 
		(void)USART1->SR;
		(void)USART1->DR;

		//Target is Memory0
		if(DMA_GetCurrentMemoryTarget(DMA2_Stream2) == 0)
		{
			DMA_Cmd(DMA2_Stream2, DISABLE);
			this_time_rx_len = BSP_USART1_DMA_RX_BUF_LEN - DMA_GetCurrDataCounter(DMA2_Stream2);
			DMA2_Stream2->NDTR = (uint16_t)BSP_USART1_DMA_RX_BUF_LEN;     //relocate the dma memory pointer to the beginning position
			DMA2_Stream2->CR |= (uint32_t)(DMA_SxCR_CT);                  //enable the current selected memory is Memory 1
			DMA_Cmd(DMA2_Stream2, ENABLE);
            if(this_time_rx_len == RC_FRAME_LENGTH)
			{
				RemoteDataPrcess(_USART1_DMA_RX_BUF[0]);
                //LostCounterFeed(GetLostCounter(LOST_COUNTER_INDEX_RC));
			}
            
//			if(this_time_rx_len == RC_FRAME_LENGTH)
//			{
//				FIFO_Put(&_USART1_RX_FIFO,(void *)&_USART1_DMA_RX_BUF[0][0]);
//                LostCounterFeed(GetLostCounter(LOST_COUNTER_INDEX_RC));
//            }

//			if(FIFO_Get(USART1_GetRxBuf(), (void *)&rc_raw_data) == 0)
//			{
//				//call the control input functions 
//				
//                RemoteDataPrcess(rc_raw_data.rc_bytes);
//			}
		}
		//Target is Memory1
		else 
		{
			DMA_Cmd(DMA2_Stream2, DISABLE);
			this_time_rx_len = BSP_USART1_DMA_RX_BUF_LEN - DMA_GetCurrDataCounter(DMA2_Stream2);
			DMA2_Stream2->NDTR = (uint16_t)BSP_USART1_DMA_RX_BUF_LEN;      //relocate the dma memory pointer to the beginning position
			DMA2_Stream2->CR &= ~(uint32_t)(DMA_SxCR_CT);                  //enable the current selected memory is Memory 0
			DMA_Cmd(DMA2_Stream2, ENABLE);
            if(this_time_rx_len == RC_FRAME_LENGTH)
			{
				RemoteDataPrcess(_USART1_DMA_RX_BUF[1]);
                //LostCounterFeed(GetLostCounter(LOST_COUNTER_INDEX_RC));
			}
//			if(this_time_rx_len == RC_FRAME_LENGTH)
//			{
//				FIFO_Put(&_USART1_RX_FIFO,(void *)&_USART1_DMA_RX_BUF[1][0]);
//				LostCounterFeed(GetLostCounter(LOST_COUNTER_INDEX_RC));
//				//RemoteDataPrcess(rc_raw_data.rc_bytes);
//			}
//            if(FIFO_Get(USART1_GetRxBuf(), (void *)&rc_raw_data) == 0)
//			{
//				//call the control input functions 
//				RemoteDataPrcess(rc_raw_data.rc_bytes);
//			}
		}
	}       
}

static void USART1_FIFO_Init(void)
{
    FIFO_Init(&_USART1_RX_FIFO, (void *)_USART1_RX_BUF, RC_FRAME_LENGTH, BSP_USART1_RX_BUF_SIZE_IN_FRAMES); 
}

void *USART1_GetRxBuf(void)
{
    return (void *)&_USART1_RX_FIFO;
}



