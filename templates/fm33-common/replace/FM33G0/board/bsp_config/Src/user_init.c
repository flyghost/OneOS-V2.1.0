#include "define_all.h"  

//cpu�δ�ʱ������(�����ʱ��)
void Init_SysTick(void)
{
    SysTick_Config(0x1000000UL);
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |//�ر��ж�
                    //SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;
}
//���������ʱǰ����������SysTick
void TicksDelay(uint32_t ClkNum)
{
    uint32_t last = SysTick->VAL;
    
    if(clkmode != 4)
    {
      ClkNum = ClkNum*clkmode;//��Ӧ��ͬ��Ƶ��8Mʱ���ʱ2ms��32Mʱ���ʱ0.5ms
    }
    else
    {
      ClkNum = ClkNum*clkmode*36/32;
    }
    if(ClkNum>0xF00000)
    {
        ClkNum = 0xF00000;
    }
    while(((last - SysTick->VAL)&0xFFFFFFUL ) < ClkNum);
}
//ms�����ʱ
void TicksDelayMs(uint32_t ms , ConditionHook Hook)
{
    uint32_t ClkNum;
    
    ClkNum = (__SYSTEM_CLOCK/1000) ;
    for(;ms>0;ms--)
    {
        if(Hook!=NULL)
        {
            if(Hook()) return ;
        }
        TicksDelay(ClkNum);
    }
}
//us�����ʱ
void TicksDelayUs(uint32_t us)
{
    uint32_t ClkNum;
    
    if(us>100000)//������100ms
    {
        us = 100000;
    }
    ClkNum = us*(__SYSTEM_CLOCK/1000000) ;
    TicksDelay(ClkNum);
}

//У��Ĵ���
unsigned char CheckSysReg( __IO uint32_t *RegAddr, uint32_t Value )
{
    if( *RegAddr != Value ) 
    {
        *RegAddr = Value;
        return 1;
    }
    else
    {
        return 0;
    }
}

//��ѯNVIC�Ĵ�����Ӧ�������ж��Ƿ��
//1 ��
//0 �ر�
unsigned char CheckNvicIrqEn( IRQn_Type IRQn )
{
    if( 0 == ( NVIC->ISER[0U] & ((uint32_t)(1UL << (((uint32_t)(int32_t)IRQn) & 0x1FUL)))) )
        return 0;
    else
        return 1;
}

//IOģ�⹦������:LCD/ADC
void AnalogIO( GPIOx_Type* GPIOx, uint32_t PinNum )
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitTypeDef  GPIO_InitStructureRun;
    
    GPIO_Get_InitPara(GPIOx, PinNum, &GPIO_InitStructureRun);
    
    if( (GPIO_InitStructureRun.Pin      != PinNum) ||
        (GPIO_InitStructureRun.PxINEN   != GPIO_IN_Dis) ||
        (GPIO_InitStructureRun.PxODEN   != GPIO_OD_En) ||
        (GPIO_InitStructureRun.PxPUEN   != GPIO_PU_Dis) ||
        (GPIO_InitStructureRun.PxFCR    != GPIO_FCR_ANA) )
    {
        GPIO_InitStructure.Pin = PinNum;
        GPIO_InitStructure.PxINEN = GPIO_IN_Dis;
        GPIO_InitStructure.PxODEN = GPIO_OD_En;
        GPIO_InitStructure.PxPUEN = GPIO_PU_Dis;
        GPIO_InitStructure.PxFCR = GPIO_FCR_ANA;
        
        GPIO_Init(GPIOx, &GPIO_InitStructure);      
    }   
}

//IO��������� 
//type 0 = ��ͨ 
//type 1 = ����
//#define IN_NORMAL 0
//#define IN_PULLUP 1
void InputtIO( GPIOx_Type* GPIOx, uint32_t PinNum, uint8_t Type )
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitTypeDef  GPIO_InitStructureRun;
    
    GPIO_Get_InitPara(GPIOx, PinNum, &GPIO_InitStructureRun);
        
    if( (GPIO_InitStructureRun.Pin      != PinNum) ||
        (GPIO_InitStructureRun.PxINEN   != GPIO_IN_En) ||
        (GPIO_InitStructureRun.PxODEN   != GPIO_OD_En) ||
        ((Type == IN_NORMAL)&&(GPIO_InitStructureRun.PxPUEN != GPIO_PU_Dis)) ||
        ((Type == IN_PULLUP)&&(GPIO_InitStructureRun.PxPUEN != GPIO_PU_En)) ||
        (GPIO_InitStructureRun.PxFCR    != GPIO_FCR_IN) )
    {
        GPIO_InitStructure.Pin = PinNum;    
        GPIO_InitStructure.PxINEN = GPIO_IN_En;
        GPIO_InitStructure.PxODEN = GPIO_OD_En;
        if(Type == IN_NORMAL)       GPIO_InitStructure.PxPUEN = GPIO_PU_Dis;
        else                        GPIO_InitStructure.PxPUEN = GPIO_PU_En; 
        GPIO_InitStructure.PxFCR = GPIO_FCR_IN;
        
        GPIO_Init(GPIOx, &GPIO_InitStructure);  
    }
}

//IO��������� 
//type 0 = ��ͨ 
//type 1 = OD
//#define OUT_PUSHPULL  0
//#define OUT_OPENDRAIN 1
void OutputIO( GPIOx_Type* GPIOx, uint32_t PinNum, uint8_t Type )
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitTypeDef  GPIO_InitStructureRun;
    
    GPIO_Get_InitPara(GPIOx, PinNum, &GPIO_InitStructureRun);
    
    if( (GPIO_InitStructureRun.Pin      != PinNum) ||
        (GPIO_InitStructureRun.PxINEN   != GPIO_IN_Dis) ||
        ((Type == OUT_PUSHPULL)&&(GPIO_InitStructureRun.PxODEN  != GPIO_OD_Dis)) ||
        ((Type == OUT_OPENDRAIN)&&(GPIO_InitStructureRun.PxODEN != GPIO_OD_En)) ||
        (GPIO_InitStructureRun.PxPUEN   != GPIO_PU_Dis) ||
        (GPIO_InitStructureRun.PxFCR    != GPIO_FCR_OUT) )
    {
        GPIO_InitStructure.Pin = PinNum;
        GPIO_InitStructure.PxINEN = GPIO_IN_Dis;
        if(Type == OUT_PUSHPULL)    GPIO_InitStructure.PxODEN = GPIO_OD_Dis;
        else                        GPIO_InitStructure.PxODEN = GPIO_OD_En;
        GPIO_InitStructure.PxPUEN = GPIO_PU_Dis;
        GPIO_InitStructure.PxFCR = GPIO_FCR_OUT;
         
        GPIO_Init(GPIOx, &GPIO_InitStructure);      
    }
}
//IO�������⹦�ܿ� 
//type 0 = ��ͨ 
//type 1 = OD (OD���ܽ��������⹦��֧��)
//type 2 = ��ͨ+���� 
//type 3 = OD+����
//#define ALTFUN_NORMAL             0
//#define ALTFUN_OPENDRAIN          1
//#define ALTFUN_PULLUP             2
//#define ALTFUN_OPENDRAIN_PULLUP   3
void AltFunIO( GPIOx_Type* GPIOx, uint32_t PinNum, uint8_t Type  )
{                                                               
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitTypeDef  GPIO_InitStructureRun;
    
    GPIO_Get_InitPara(GPIOx, PinNum, &GPIO_InitStructureRun);
    
    if( (GPIO_InitStructureRun.Pin      != PinNum) ||
        (GPIO_InitStructureRun.PxINEN   != GPIO_IN_Dis) ||
        (((Type & 0x01) == 0)&&(GPIO_InitStructureRun.PxODEN    != GPIO_OD_Dis)) ||
        (((Type & 0x01) != 0)&&(GPIO_InitStructureRun.PxODEN    != GPIO_OD_En)) ||
        (((Type & 0x02) == 0)&&(GPIO_InitStructureRun.PxPUEN    != GPIO_PU_Dis)) ||
        (((Type & 0x02) != 0)&&(GPIO_InitStructureRun.PxPUEN    != GPIO_PU_En)) ||
        (GPIO_InitStructureRun.PxFCR    != GPIO_FCR_DIG) )
    {
        GPIO_InitStructure.Pin = PinNum;
        GPIO_InitStructure.PxINEN = GPIO_IN_Dis;
        if( (Type & 0x01) == 0 )    GPIO_InitStructure.PxODEN = GPIO_OD_Dis;
        else                        GPIO_InitStructure.PxODEN = GPIO_OD_En;
        if( (Type & 0x02) == 0 )    GPIO_InitStructure.PxPUEN = GPIO_PU_Dis;
        else                        GPIO_InitStructure.PxPUEN = GPIO_PU_En; 
        GPIO_InitStructure.PxFCR = GPIO_FCR_DIG;
         
        GPIO_Init(GPIOx, &GPIO_InitStructure);      
    }
}

//IO�رգ�od����ߣ�
//������ʹ����Чʱ������ⲿ�źŸ��գ�Ҳ���ܵ���FM385�ܽ�©�磻
//���Խ�FCR����Ϊ01��GPIO�������ODEN����Ϊ1����α��©������ر�����ʹ�ܣ��������������Ϊ1
//ע��SWD�ӿڵ�PG8,9�������ı����ǵ����ý��޷�����
void CloseeIO( GPIOx_Type* GPIOx, uint32_t PinNum )
{
    GPIO_InitTypeDef  GPIO_InitStructureRun;
    
    GPIO_Get_InitPara(GPIOx, PinNum, &GPIO_InitStructureRun);
        
    if((GPIO_InitStructureRun.PxFCR != GPIO_FCR_OUT))
    {
        GPIO_SetBits(GPIOx, PinNum);
        OutputIO( GPIOx, PinNum, OUT_OPENDRAIN );
    }
    else
    {
        OutputIO( GPIOx, PinNum, OUT_OPENDRAIN );
        GPIO_SetBits(GPIOx, PinNum);        
    }
}

//IO�ڳ�ʼ���״̬����
void Init_Pad_Io(void)
{   
    GPIOx_DO_Write(GPIOA, 0x0000);  //
    GPIOx_DO_Write(GPIOB, 0x0000);  //
    GPIOx_DO_Write(GPIOC, 0x0000);  //
    GPIOx_DO_Write(GPIOD, 0x0000);  //
    GPIOx_DO_Write(GPIOE, 0x0000);  //
    GPIOx_DO_Write(GPIOF, 0x0000);  //
    GPIOx_DO_Write(GPIOG, 0x0000);  //
}

//�ر�80��оƬ����ʹ�õ�IO
void Close_None_GPIO_80pin(void)
{
    CloseeIO( GPIOC, GPIO_Pin_0 );
    CloseeIO( GPIOC, GPIO_Pin_1 );

    CloseeIO( GPIOD, GPIO_Pin_8);
    CloseeIO( GPIOD, GPIO_Pin_9 );
    CloseeIO( GPIOD, GPIO_Pin_10 ); 
    CloseeIO( GPIOD, GPIO_Pin_11 ); 
    CloseeIO( GPIOD, GPIO_Pin_12 ); 
    CloseeIO( GPIOD, GPIO_Pin_13 ); 
    CloseeIO( GPIOD, GPIO_Pin_14 ); 
    CloseeIO( GPIOD, GPIO_Pin_15 ); 

    CloseeIO( GPIOE, GPIO_Pin_0 );
    CloseeIO( GPIOE, GPIO_Pin_1 );
    CloseeIO( GPIOE, GPIO_Pin_5 );
    CloseeIO( GPIOE, GPIO_Pin_6 );
    CloseeIO( GPIOE, GPIO_Pin_7 );
    CloseeIO( GPIOE, GPIO_Pin_8 );
    CloseeIO( GPIOE, GPIO_Pin_9 );
    CloseeIO( GPIOE, GPIO_Pin_10 );
    CloseeIO( GPIOE, GPIO_Pin_11 );
    CloseeIO( GPIOE, GPIO_Pin_12 );
    CloseeIO( GPIOE, GPIO_Pin_13 );
    CloseeIO( GPIOE, GPIO_Pin_14 );
    CloseeIO( GPIOE, GPIO_Pin_15 );

    CloseeIO( GPIOF, GPIO_Pin_0 );
    CloseeIO( GPIOF, GPIO_Pin_1 );
    CloseeIO( GPIOF, GPIO_Pin_2 );
    CloseeIO( GPIOF, GPIO_Pin_7 );
    CloseeIO( GPIOF, GPIO_Pin_8 );
    CloseeIO( GPIOF, GPIO_Pin_9 );
    CloseeIO( GPIOF, GPIO_Pin_10 );

    CloseeIO( GPIOG, GPIO_Pin_0 );
    CloseeIO( GPIOG, GPIO_Pin_1 );
    CloseeIO( GPIOG, GPIO_Pin_4 );
    CloseeIO( GPIOG, GPIO_Pin_5 );
    CloseeIO( GPIOG, GPIO_Pin_10 );
    CloseeIO( GPIOG, GPIO_Pin_11 );
    CloseeIO( GPIOG, GPIO_Pin_12 );
    CloseeIO( GPIOG, GPIO_Pin_13 );
    CloseeIO( GPIOG, GPIO_Pin_14 );
    CloseeIO( GPIOG, GPIO_Pin_15 );
}

void Close_AllIOEXTI(void)
{
    CheckSysReg( &GPIO->EXTI0_SEL   , 0xFFFF0000 );
    CheckSysReg( &GPIO->EXTI1_SEL   , 0xFFFF0000 );
    CheckSysReg( &GPIO->EXTI2_SEL   , 0xFFFF0000 );
}

void IO_AnalogFunSet(void)
{
    /* PE4ģ�⹦��ѡ�� */
    GPIO_ANASEL_PE4ANS_Set(GPIO_ANASEL_PE4ANS_ACMP2_INP1);

    /* PE3ģ�⹦��ѡ�� */
    GPIO_ANASEL_PE3ANS_Set(GPIO_ANASEL_PE3ANS_ACMP2_INN1);

    /* PC15ģ�⹦��ѡ�� */
    GPIO_ANASEL_PC15ANS_Set(GPIO_ANASEL_PC15ANS_ACMP2_INP0_ADC_IN6);

    /* PC14ģ�⹦��ѡ�� */
    GPIO_ANASEL_PC14ANS_Set(GPIO_ANASEL_PC14ANS_ACMP2_INN0);

    /* PC13ģ�⹦��ѡ�� */
    GPIO_ANASEL_PC13ANS_Set(GPIO_ANASEL_PC13ANS_ADC_IN2);

    /* PC12ģ�⹦��ѡ�� */
    GPIO_ANASEL_PC12ANS_Set(GPIO_ANASEL_PC12ANS_ADC_IN1);
}

void IO_DFFunSet(void)
{
    GPIO_IODF_SetableEx(IODF_PF3    , DISABLE);
    GPIO_IODF_SetableEx(IODF_PE5    , DISABLE);
    GPIO_IODF_SetableEx(IODF_PE2    , DISABLE);
    GPIO_IODF_SetableEx(IODF_PD3    , DISABLE);
    GPIO_IODF_SetableEx(IODF_PD2    , DISABLE);
    GPIO_IODF_SetableEx(IODF_PD1    , DISABLE);
    GPIO_IODF_SetableEx(IODF_PD0    , DISABLE);
    GPIO_IODF_SetableEx(IODF_PC15   , DISABLE);
    GPIO_IODF_SetableEx(IODF_PC14   , DISABLE);
    GPIO_IODF_SetableEx(IODF_PC13   , DISABLE);
    GPIO_IODF_SetableEx(IODF_PC12   , DISABLE);
    GPIO_IODF_SetableEx(IODF_PB7    , DISABLE);
    GPIO_IODF_SetableEx(IODF_PB6    , DISABLE);
    GPIO_IODF_SetableEx(IODF_PB5    , DISABLE);
    GPIO_IODF_SetableEx(IODF_PB4    , DISABLE);
    GPIO_IODF_SetableEx(IODF_PA11   , DISABLE);
    GPIO_IODF_SetableEx(IODF_PA10   , DISABLE);
    GPIO_IODF_SetableEx(IODF_PA9    , DISABLE);
    GPIO_IODF_SetableEx(IODF_PA8    , DISABLE);
}

void IO_WKENFunSet(void)
{
    GPIO_PINWKEN_SetableEx(PINWKEN_PD6  , DISABLE);
    GPIO_PINWKEN_SetableEx(PINWKEN_PC14 , DISABLE);
    GPIO_PINWKEN_SetableEx(PINWKEN_PE2  , DISABLE);
    GPIO_PINWKEN_SetableEx(PINWKEN_PA13 , DISABLE);
    GPIO_PINWKEN_SetableEx(PINWKEN_PG7  , DISABLE);
    GPIO_PINWKEN_SetableEx(PINWKEN_PC13 , DISABLE);
    GPIO_PINWKEN_SetableEx(PINWKEN_PB0  , DISABLE);
    GPIO_PINWKEN_SetableEx(PINWKEN_PF5  , DISABLE);
}

//�ر�80��оƬ��ʹ�õ�ȫ��IO(SWD�ڳ��⣬�رպ��ܷ���)
void Close_AllIO_GPIO_80pin( void )
{   
    //�ر�����IO���жϹ���
    Close_AllIOEXTI();
    
    //ǿ�����ر�
    GPIO_HDSEL_PE2HDEN_Setable(DISABLE);
    GPIO_HDSEL_PG6HDEN_Setable(DISABLE);
    
    //FOUT����ź�ѡ��
    GPIO_FOUTSEL_FOUTSEL_Set(GPIO_FOUTSEL_FOUTSEL_LSCLK);
    
    //ģ���Һ����AD����ѡ��  
    IO_AnalogFunSet();
    
    //����IO����������˲�����
    IO_DFFunSet();
    
    //�ر�NWKUP
    IO_WKENFunSet();
    
    CloseeIO( GPIOA, GPIO_Pin_0 );  //PA0; //
    CloseeIO( GPIOA, GPIO_Pin_1 );  //PA1; //
    CloseeIO( GPIOA, GPIO_Pin_2 );  //PA2; //
    CloseeIO( GPIOA, GPIO_Pin_3 );  //PA3; //
    CloseeIO( GPIOA, GPIO_Pin_4 );  //PA4; //
    CloseeIO( GPIOA, GPIO_Pin_5 );  //PA5; //
    CloseeIO( GPIOA, GPIO_Pin_6 );  //PA6; //
    CloseeIO( GPIOA, GPIO_Pin_7 );  //PA7; //
    CloseeIO( GPIOA, GPIO_Pin_8 );  //PA8; //
    CloseeIO( GPIOA, GPIO_Pin_9 );  //PA9; //
    CloseeIO( GPIOA, GPIO_Pin_10 ); //PA10;//
    CloseeIO( GPIOA, GPIO_Pin_11 ); //PA11;//
    CloseeIO( GPIOA, GPIO_Pin_12 ); //PA12;//
    CloseeIO( GPIOA, GPIO_Pin_13 ); //PA13;//
    CloseeIO( GPIOA, GPIO_Pin_14 ); //PA14;//
    CloseeIO( GPIOA, GPIO_Pin_15 ); //PA15;//

    CloseeIO( GPIOB, GPIO_Pin_0 );  //PB0; //
    CloseeIO( GPIOB, GPIO_Pin_1 );  //PB1; //
    CloseeIO( GPIOB, GPIO_Pin_2 );  //PB2; //
    CloseeIO( GPIOB, GPIO_Pin_3 );  //PB3; //
    CloseeIO( GPIOB, GPIO_Pin_4 );  //PB4; //
    CloseeIO( GPIOB, GPIO_Pin_5 );  //PB5; //
    CloseeIO( GPIOB, GPIO_Pin_6 );  //PB6; //
    CloseeIO( GPIOB, GPIO_Pin_7 );  //PB7; //
    CloseeIO( GPIOB, GPIO_Pin_8 );  //PB8; //
    CloseeIO( GPIOB, GPIO_Pin_9 );  //PB9; //
    CloseeIO( GPIOB, GPIO_Pin_10 ); //PB10;//
    CloseeIO( GPIOB, GPIO_Pin_11 ); //PB11;//
    CloseeIO( GPIOB, GPIO_Pin_12 ); //PB12;//
    CloseeIO( GPIOB, GPIO_Pin_13 ); //PB13;//
    CloseeIO( GPIOB, GPIO_Pin_14 ); //PB14;//
    CloseeIO( GPIOB, GPIO_Pin_15 ); //PB15;//
    
    CloseeIO( GPIOC, GPIO_Pin_2 );  //PC2; //
    CloseeIO( GPIOC, GPIO_Pin_3 );  //PC3; //
    CloseeIO( GPIOC, GPIO_Pin_4 );  //PC4; //
    CloseeIO( GPIOC, GPIO_Pin_5 );  //PC5; //
    CloseeIO( GPIOC, GPIO_Pin_6 );  //PC6; //
    CloseeIO( GPIOC, GPIO_Pin_7 );  //PC7; //
    CloseeIO( GPIOC, GPIO_Pin_8 );  //PC8; //
    CloseeIO( GPIOC, GPIO_Pin_9 );  //PC9; //
    CloseeIO( GPIOC, GPIO_Pin_10 ); //PC10;//
    CloseeIO( GPIOC, GPIO_Pin_11 ); //PC11;//
    CloseeIO( GPIOC, GPIO_Pin_12 ); //PC12;//
    CloseeIO( GPIOC, GPIO_Pin_13 ); //PC13;//
    CloseeIO( GPIOC, GPIO_Pin_14 ); //PC14;//
    CloseeIO( GPIOC, GPIO_Pin_15 ); //PC15;//
    
    CloseeIO( GPIOD, GPIO_Pin_0 );  //PD0;//
    CloseeIO( GPIOD, GPIO_Pin_1 );  //PD1;//
    CloseeIO( GPIOD, GPIO_Pin_2 );  //PD2;//
    CloseeIO( GPIOD, GPIO_Pin_3 );  //PD3;//
    CloseeIO( GPIOD, GPIO_Pin_4 );  //PD4;//
    CloseeIO( GPIOD, GPIO_Pin_5 );  //PD5;//
    CloseeIO( GPIOD, GPIO_Pin_6 );  //PD6;//
    CloseeIO( GPIOD, GPIO_Pin_7 );  //PD7;//
    
    CloseeIO( GPIOE, GPIO_Pin_2 );  //PE2;//
    CloseeIO( GPIOE, GPIO_Pin_3 );  //PE3;//
    CloseeIO( GPIOE, GPIO_Pin_4 );  //PE4;//

    CloseeIO( GPIOF, GPIO_Pin_3 );  //PF3; //
    CloseeIO( GPIOF, GPIO_Pin_4 );  //PF4; //
    CloseeIO( GPIOF, GPIO_Pin_5 );  //PF5; //
    CloseeIO( GPIOF, GPIO_Pin_6 );  //PF6; //
    CloseeIO( GPIOF, GPIO_Pin_11 ); //PF11;//
    CloseeIO( GPIOF, GPIO_Pin_12 ); //PF12;//
    CloseeIO( GPIOF, GPIO_Pin_13 ); //PF13;//
    CloseeIO( GPIOF, GPIO_Pin_14 ); //PF14;//
    CloseeIO( GPIOF, GPIO_Pin_15 ); //PF15;//
    
    CloseeIO( GPIOG, GPIO_Pin_2 );  //PG2;//
    CloseeIO( GPIOG, GPIO_Pin_3 );  //PG3;//
    CloseeIO( GPIOG, GPIO_Pin_6 );  //PG6;//
    CloseeIO( GPIOG, GPIO_Pin_7 );  //PG7;//
    //ע��SWD�ӿڵ�PG8,9�������ı����ǵ����ý��޷�����
    AltFunIO( GPIOG, GPIO_Pin_8, ALTFUN_NORMAL );   //PG8;//SWDTCK
    AltFunIO( GPIOG, GPIO_Pin_9, ALTFUN_NORMAL );   //PG9;//SWDTDO  
}

//Ĭ�Ͽ����󲿷�����ʱ�ӣ��û��ɰ�����رղ���Ҫ��ʱ��
//ʱ�ӿ����رնԹ���Ӱ�첻��
void Init_RCC_PERIPH_clk(void)
{
    //PERCLKCON1
    RCC_PERCLK_SetableEx( DCUCLK,       ENABLE );       //debug controʱ��ʹ�ܣ������
    RCC_PERCLK_SetableEx( EXTI2CLK,     ENABLE );       //EXTI�ⲿ�����жϲ���ʱ�ӣ�IO�����˲�ʱ��ʹ��
    RCC_PERCLK_SetableEx( EXTI1CLK,     ENABLE );       //EXTI�ⲿ�����жϲ���ʱ�ӣ�IO�����˲�ʱ��ʹ��
    RCC_PERCLK_SetableEx( EXTI0CLK,     ENABLE );       //EXTI�ⲿ�����жϲ���ʱ�ӣ�IO�����˲�ʱ��ʹ��
    RCC_PERCLK_SetableEx( PDCCLK,       ENABLE );       //IO����ʱ�ӼĴ���ʹ��
    RCC_PERCLK_SetableEx( ANACCLK,      ENABLE );       //ģ���·����ʱ��ʹ��
    RCC_PERCLK_SetableEx( IWDTCLK,      ENABLE );       //IWDT����ʱ��ʹ��
    RCC_PERCLK_SetableEx( SCUCLK,       ENABLE );       //system controlʱ��ʹ�ܣ������
    RCC_PERCLK_SetableEx( PMUCLK,       ENABLE );       //��Դ����ģ��ʱ��ʹ��
    RCC_PERCLK_SetableEx( RTCCLK,       ENABLE );       //RTC����ʱ��ʹ��
    RCC_PERCLK_SetableEx( LPTFCLK,      ENABLE );       //LPTIM����ʱ��ʹ��
    RCC_PERCLK_SetableEx( LPTRCLK,      ENABLE );       //LPTIM����ʱ��ʹ��
                
    //PERCLKCON2 SETTING
    RCC_PERCLK_SetableEx( ADCCLK,       ENABLE );       //ADCʱ��ʹ��
    RCC_PERCLK_SetableEx( WWDTCLK,      ENABLE );       //WWDTʱ��ʹ��
    RCC_PERCLK_SetableEx( RAMBISTCLK,   DISABLE );      //RAMBISTʱ��ʹ�ܣ�����ر�
    RCC_PERCLK_SetableEx( FLSEPCLK,     DISABLE );      //Flash��д������ʱ��ʹ�ܣ�����͹�
    RCC_PERCLK_SetableEx( DMACLK,       ENABLE );       //DMAʱ��ʹ��
    RCC_PERCLK_SetableEx( LCDCLK,       ENABLE );       //LCDʱ��ʹ��
    RCC_PERCLK_SetableEx( AESCLK,       ENABLE );       //AESʱ��ʹ��
    RCC_PERCLK_SetableEx( TRNGCLK,      ENABLE );       //TRNGʱ��ʹ��
    RCC_PERCLK_SetableEx( CRCCLK,       ENABLE );       //CRCʱ��ʹ��

    //PERCLKCON3 SETTING
    RCC_PERCLK_SetableEx( I2CCLK,       ENABLE );       //I2Cʱ��ʹ��
  RCC_PERCLK_SetableEx( LPUFCKEN, ENABLE );     //LPUFCKEN����ʱ��ʹ��
    RCC_PERCLK_SetableEx( U7816CLK1,    ENABLE );       //78161ʱ��ʹ��
    RCC_PERCLK_SetableEx( U7816CLK0,    ENABLE );       //78160ʱ��ʹ��
  RCC_PERCLK_SetableEx( LPUARTCKEN,     ENABLE );       //LPUART�Ĵ�������ʱ��ʹ��
    RCC_PERCLK_SetableEx( UARTCOMCLK,   ENABLE );       //UART0~5����Ĵ���ʱ��ʹ��
    RCC_PERCLK_SetableEx( UART5CLK,     ENABLE );       //UART5ʱ��ʹ��
    RCC_PERCLK_SetableEx( UART4CLK,     ENABLE );       //UART4ʱ��ʹ��
    RCC_PERCLK_SetableEx( UART3CLK,     ENABLE );       //UART3ʱ��ʹ��
    RCC_PERCLK_SetableEx( UART2CLK,     ENABLE );       //UART2ʱ��ʹ��
    RCC_PERCLK_SetableEx( UART1CLK,     ENABLE );       //UART1ʱ��ʹ��
    RCC_PERCLK_SetableEx( UART0CLK,     ENABLE );       //UART0ʱ��ʹ��
    RCC_PERCLK_SetableEx( HSPICLK,      ENABLE );       //HSPIʱ��ʹ��
    RCC_PERCLK_SetableEx( SPI2CLK,      ENABLE );       //SPI2ʱ��ʹ��
    RCC_PERCLK_SetableEx( SPI1CLK,      ENABLE );       //SPI1ʱ��ʹ��

    //PERCLKCON4 SETTING
    RCC_PERCLK_SetableEx( ET4CLK,       ENABLE );       //ET4ʱ��ʹ��
    RCC_PERCLK_SetableEx( ET3CLK,       ENABLE );       //ET3ʱ��ʹ��
    RCC_PERCLK_SetableEx( ET2CLK,       ENABLE );       //ET2ʱ��ʹ��
    RCC_PERCLK_SetableEx( ET1CLK,       ENABLE );       //ET1ʱ��ʹ��
    RCC_PERCLK_SetableEx( BT2CLK,       ENABLE );       //BT2ʱ��ʹ��
    RCC_PERCLK_SetableEx( BT1CLK,       ENABLE );       //BT1ʱ��ʹ��
}

void Init_PLL(void)
{
    RCC_PLL_InitTypeDef PLL_InitStruct;
    
    PLL_InitStruct.PLLDB = 499; //pll��Ƶ�� = PLLDB + 1
    PLL_InitStruct.PLLINSEL = RCC_PLLCON_PLLINSEL_XTLF; //PLLʱ��Դѡ��XTLF
    PLL_InitStruct.PLLOSEL = RCC_PLLCON_PLLOSEL_MUL1;   //Ĭ��ѡ��1�������������PLLDB��1023ʱ����ʹ��2�����ʵ�ָ��ߵı�Ƶ
    PLL_InitStruct.PLLEN = DISABLE; //Ĭ�Ϲر�PLL
    
    RCC_PLL_Init(&PLL_InitStruct);
    RCC_PLLCON_PLLEN_Setable(DISABLE);//�ر�PLL
}

//ϵͳʱ������
//ʹ��RCHF����ʱ��,define_all.h ��SYSCLKdef�����ϵͳʱ��Ƶ��
void Init_SysClk(void)
{
    RCC_RCHF_InitTypeDef RCHF_InitStruct;
    RCC_SYSCLK_InitTypeDef SYSCLK_InitStruct;
    
    RCHF_InitStruct.FSEL = SYSCLKdef;//define_all.h ��SYSCLKdef�����ϵͳʱ��Ƶ��
    RCHF_InitStruct.RCHFEN = ENABLE;//��RCHF
    
    RCC_RCHF_Init(&RCHF_InitStruct);
    
    SYSCLK_InitStruct.SYSCLKSEL = RCC_SYSCLKSEL_SYSCLKSEL_RCHF; //ѡ��RCHF����ʱ��
    SYSCLK_InitStruct.AHBPRES = RCC_SYSCLKSEL_AHBPRES_DIV1;     //AHB����Ƶ
    SYSCLK_InitStruct.APBPRES = RCC_SYSCLKSEL_APBPRES_DIV1;     //APB����Ƶ
    SYSCLK_InitStruct.EXTICKSEL = RCC_SYSCLKSEL_EXTICKSEL_AHBCLK;   //EXTI,�����˲�ʱ��ʹ��AHBʱ��
    SYSCLK_InitStruct.SLP_ENEXTI = ENABLE;//����ģʽʹ���ⲿ�жϲ���
    SYSCLK_InitStruct.LPM_RCLP_OFF = DISABLE;//����ģʽ�¿���RCLP  
    
    RCC_SysClk_Init(&SYSCLK_InitStruct);
}

//Mode:0 ����ģʽ�����п��Ź����������ж�ʱ��
//Mode:1 ����ģʽ�رտ��Ź����������ж�ʱ��
void SCU_Init(uint08 Mode)
{
    if(Mode == 1)//debug
    {
        SCU_MCUDBGCR_DBG_WWDT_STOP_Setable(ENABLE);//����ģʽ�¹ر�WWDT
        SCU_MCUDBGCR_DBG_IWDT_STOP_Setable(ENABLE);//����ģʽ�¹ر�IWDT
    }
    else//release
    {
        SCU_MCUDBGCR_DBG_WWDT_STOP_Setable(DISABLE);//����ģʽ������WWDT
        SCU_MCUDBGCR_DBG_IWDT_STOP_Setable(DISABLE);//����ģʽ������IWDT
    }
    
    SCU_MCUDBGCR_DBG_ET4_STOP_Setable(DISABLE);//����ģʽ������ET4
    SCU_MCUDBGCR_DBG_ET3_STOP_Setable(DISABLE);//����ģʽ������ET3
    SCU_MCUDBGCR_DBG_ET2_STOP_Setable(DISABLE);//����ģʽ������ET2
    SCU_MCUDBGCR_DBG_ET1_STOP_Setable(DISABLE);//����ģʽ������ET1
    SCU_MCUDBGCR_DBG_BT2_STOP_Setable(DISABLE);//����ģʽ������BT2
    SCU_MCUDBGCR_DBG_BT1_STOP_Setable(DISABLE);//����ģʽ������BT1
}

void Init_SysClk_Gen( void )                //ʱ��ѡ�����
{   
    
    /*ϵͳʱ�ӳ���24M����Ҫ��wait1*/
    if( RCHFCLKCFG > 24 ) FLASH_FLSRDCON_WAIT_Set(FLASH_FLSRDCON_WAIT_1CYCLE);
    else FLASH_FLSRDCON_WAIT_Set(FLASH_FLSRDCON_WAIT_0CYCLE);   
    
    /*PLL����*/
    Init_PLL(); //Ĭ�Ϲر�PLL
    
    /*ϵͳʱ������*/
    Init_SysClk();  //Ĭ��ʹ��RCHF����ʱ��
    
    /*����ʱ��ʹ������*/    
    Init_RCC_PERIPH_clk();  //Ĭ�Ͽ����󲿷�����ʱ��
    
    /*DMA����RAM���ȼ�����*/
    RCC_MPRIL_MPRIL_Set(RCC_MPRIL_MPRIL_DMA);   //Ĭ��AHB Master���ȼ�����DMA����
    
    
    /*�µ縴λ����*/
    //pdr��bor�����µ縴λ����Ҫ��һ��
    //����Դ��ѹ�����µ縴λ��ѹʱ��оƬ�ᱻ��λס        
    //pdr��ѹ��λ��׼���ǹ��ļ��ͣ������޷�������
    //bor��ѹ��λ׼ȷ������Ҫ����2uA����
    ANAC_PDRCON_PDREN_Setable(ENABLE);      //��PDR
    ANAC_BORCON_OFF_BOR_Setable(DISABLE);   //��BOR
    
    /*������ƼĴ�������*/
    #ifdef __DEBUG
    SCU_Init(1);//����ʱ���ж�ʱ��,�رտ��Ź�
    #else
    SCU_Init(0);
    #endif
}

void IWDT_Init(void)
{
    RCC_PERCLK_SetableEx(IWDTCLK, ENABLE);      //IWDT����ʱ��ʹ��
    IWDT_Clr();                                 //��IWDT
    IWDT_IWDTCFG_IWDTOVP_Set(IWDT_IWDTCFG_IWDTOVP_2s);//����IWDT�������
    IWDT_IWDTCFG_IWDTSLP4096S_Setable(DISABLE); //��������ʱ�Ƿ�����4096s������
}

void Init_IO(void)
{
    LED0_OFF;

    OutputIO( LED0_GPIO, LED0_PIN, 0 );  //led0
    
    //fout ���ϵͳʱ��64��Ƶ
//  GPIO_FOUTSEL_FOUTSEL_Set(GPIO_FOUTSEL_FOUTSEL_AHBCLKD64);
//  AltFunIO( GPIOG, GPIO_Pin_6, ALTFUN_NORMAL );
}

void LED0_Flash(uint08 Times)
{
    uint08 i;
    
    for( i=0; i<Times; i++ )
    {
        LED0_ON;
        TicksDelayMs( 100, NULL );      
        LED0_OFF;
        TicksDelayMs( 100, NULL );  
    }
}


void Init_System(void)
{       
    /*����ϵͳ����*/
    __disable_irq();          //�ر�ȫ���ж�ʹ��
    //  IWDT_Init();              //ϵͳ���Ź�����
    //  IWDT_Clr();               //��ϵͳ���Ź�    
    //    Init_SysTick();             //cpu�δ�ʱ������(�����ʱ��) 
    //    TicksDelayMs( 10, NULL );   //�����ʱ,ϵͳ�ϵ��Ҫ���̽�ʱ���л�Ϊ��RCHF8M��Ҳ��Ҫ���̽����߷�����ܵ����޷����س���

    Init_SysClk_Gen();          //ϵͳʱ������
    RCC_Init_RCHF_Trim(clkmode);//RCHF����У׼ֵ����(оƬ��λ���Զ�����8M��У׼ֵ)��ֻ�ǵ�УRCHF���¾���

    /*�����ʼ������*/
    Init_Pad_Io();              //IO������Ĵ�����ʼ״̬����

    Close_None_GPIO_80pin();    //�ر�80��оƬ��֧�ֵ�IO
    Close_AllIO_GPIO_80pin();   //�ر�ȫ��IO


    /*RTC��ֵ�����Ĵ���*/
    RTC_ADJUST_Write(0);//RTCʱ���¶Ȳ���ֵд0�����粻���������Ĵ���������ֵ����һ���������RTCʱ�ӿ��ܻ�ƫ��ǳ���

    /*�û���ʼ������*/
    //    Init_IO();

    /*׼��������ѭ��*/
    //    TicksDelayMs( 100, NULL );  //�����ʱ

    //    LED0_Flash(5);              //�����ʱ���������

    __enable_irq();             //��ȫ���ж�ʹ��
}
