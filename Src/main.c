/**
 ******************************************************************************
 * @file           : main.c
 * @author         : beshoy
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

#include "../Inc/LIB/STD_TYPES.h"
#include "../Inc/LIB/BIT_MATH.h"
#include "../Inc/MCAL/DIO/DIO_interface.h"
#include "../Inc/MCAL/USART/USART_interface.h"
#include"../Inc/MCAL/NVIC/NVIC_interface.h"
#include "../Inc/MCAL/RCC/RCC_interface.h"
#include	"../Inc/MCAL/SYSTIC/STK_interface.h"
#include "../Inc/MCAL/PWM/PWM_interface.h"
#include "../Inc/MCAL/USART/USART_interface.h"
#include "../Inc/MCAL/SPI/SPI_interface.h"
#include "../Inc/MCAL/T2DELAY/T2DELAY_interface.h"
#include "../Inc/MCAL/EXTI/EXTI_interface.h"
#include "../Inc/HAL/TFT/ILI9341_STM32_Driver.h"
#include "../Inc/HAL/TFT/ILI9341_GFX.h"
#include "../Inc/HAL/MD/MD_interface.h"
void return_uart1(u8 data);
void	timer_CallFuncCalcEq	(void);
void EXTI_CallFuncCounterFunc(void);
u8 volatile temperature = 0;
u8 volatile BPM = 0;
u8 volatile speed=0;
u8 GlobalEncoder_u8Counter=0;
int main(void)
{
	MRCC_voidPerClock_State(APB2 ,IOPA_PERIPHERAL ,PClock_enable );
	MRCC_voidPerClock_State(APB2 ,IOPB_PERIPHERAL ,PClock_enable );
	MRCC_voidPerClock_State(APB2, SPI1_PERIPHERAL, PClock_enable);
	MRCC_voidPerClock_State(APB2 ,USART1_PERIPHERAL ,PClock_enable );
	MRCC_voidPerClock_State(APB1 ,USART2_PERIPHERAL ,PClock_enable );
	MRCC_voidPerClock_State(APB1 ,TIM3_PERIPHERAL ,PClock_enable );
	MRCC_voidPerClock_State(APB1 ,TIM2_PERIPHERAL ,PClock_enable );

	/***********************************************************************************************/
	MRCC_voidInit			();
	MSTK_voidInit 			();
	MNVIC_voidInit			();
	HMD_voidInit			();

	/*Node MCU pin*/
	//MGPIO_voidSetPinDirection   (GPIOA , PIN12  , INPUT_FLOATING);


	/*EXTI PIN Speed encoder */
	MGPIO_voidSetPinDirection(GPIOA , PIN15  , INPUT_PULL_UP_DOWN);
	MNVIC_voidEnableInterrupt(EXTI15_10);
	MEXTI_voidSetCallBack	(EXTI_CallFuncCounterFunc);
	MEXTI_voidSetExtiEdge	(MEXTI_LINE15 , MEXTI_FALLING_EDGE);
	MEXTI_voidSetEXTILinePin	(MEXTI_LINE15, EXTI_PORTA);
	MEXTI_voidEnableExti	(MEXTI_LINE15);
	/*initialize UART 1 to communicate with arduino*/
	MUSART_SetCallBack(return_uart1);
	MUSART_voidInit(USART1);
	MNVIC_voidEnableInterrupt(USART11);
	MUSART_voidInit(USART2);
	MNVIC_voidEnableInterrupt(USART22);
/*	MUSART_voidInit(USART2);
	MNVIC_voidEnableInterrupt(USART11);*/

	/*initialize PWM for 38KHZ IR*/
	MPWM_voidInit(TIMER4, CH1,212);
	MPWM_SETValuesOfPWM(TIMER4, CH1, 106);


	/*initialize timer3 for interrupt for speed encoder*/
	MNVIC_voidEnableInterrupt(TIM3);
	MTIMER2_voidSetCallBack(timer_CallFuncCalcEq);
	MTIMER2_voidSetinterruptTimer();
	/*SPI TFT PINS*/
	MGPIO_voidSetPinDirection   (GPIOA , PIN6  , INPUT_FLOATING);
	MGPIO_voidSetPinDirection   (GPIOA , PIN5  , OUTPUT_SPEED_50MHZ_AFPP);
	MGPIO_voidSetPinDirection   (GPIOA , PIN7  , OUTPUT_SPEED_50MHZ_AFPP);


	/*SPI TFT configuration*/

	SPI_config_t SPI1_config = { 1 , 0 , 0 , 0 , 0 , 1, 0 ,0 ,0 };    /* Loop forever */
	SPI_u8ConfigureCh(SPI1 , &SPI1_config );
	MSTK_voidSetBusyWait 		  (1000000);
	ILI9341_Init();

    u8 traffic=255;
    u8 welcoming_flag=0;
    ILI9341_WelcomingMessage();
	while(1)
	{
		if(/*MGPIO_u8GetPinValue      (GPIOA , PIN12 ) == GPIO_HIGH*/1)
		{
			if(welcoming_flag == 1)
			{
			    ILI9341_WelcomingMessage();
			    welcoming_flag = 0;
			}
			MUSART_u8GetRecievedValue(USART2, &traffic);
			ILI9341_ShowParametersV2(BPM, temperature, speed);
			ILI9341_ShowRoadSigns(traffic);
			traffic=255;
			//HMD_voidForward();
		}
		else
		{
			ILI9341_FillScreen(WHITE);
			welcoming_flag = 1;
		}
	}
}
void return_uart1(u8 data)
{
	static u8 flag=0;
	if (flag == 1)
	{
		BPM = data;
		flag = 0;
	}
	else if(flag == 2)
	{
		temperature = data;
		flag = 0;
	}
	if(data == '*')
	{
		flag=1;
	}
	else if(data == '&')
	{
		flag=2;
	}

}
void	timer_CallFuncCalcEq	(void)
{
	speed = ((GlobalEncoder_u8Counter*207.345115137/3) *.036)/1.5;
	GlobalEncoder_u8Counter=0;
}
void EXTI_CallFuncCounterFunc(void)
{
	GlobalEncoder_u8Counter++;
}
