/******************************************************************************
* Copyright 2013-2014 Espressif Systems (Wuxi)
*
* FileName: hw_timer.c
*
* Description: hw_timer driver
*
* Modification history:
*     2014/5/1, v1.0 create this file.
*******************************************************************************/

#include "hw_timer.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"

/******************************************************************************
* FunctionName : hw_timer_arm
* Description  : set a trigger timer delay for this timer.
* Parameters   : uint32 val :
in autoload mode
                        50 ~ 0x7fffff;  for FRC1 source.
                        100 ~ 0x7fffff;  for NMI source.
in non autoload mode:
                        10 ~ 0x7fffff;
* Returns      : NONE
*******************************************************************************/
void  hw_timer_arm(u32 val)
{
    RTC_REG_WRITE(FRC1_LOAD_ADDRESS, US_TO_RTC_TIMER_TICKS(val));
}

/******************************************************************************
* FunctionName : hw_timer_set_func
* Description  : set the func, when trigger timer is up.
* Parameters   : void (* user_hw_timer_cb_set)(void):
                        timer callback function,
* Returns      : NONE
*******************************************************************************/
void  hw_timer_set_func(void (* user_hw_timer_cb_set)(void))
{
    user_hw_timer_cb = user_hw_timer_cb_set;
}

static void  hw_timer_isr_cb(void)
{
    if (user_hw_timer_cb != NULL) {
        (*(user_hw_timer_cb))();
    }
}

/******************************************************************************
* FunctionName : hw_timer_init
* Description  : initilize the hardware isr timer
* Parameters   :
FRC1_TIMER_SOURCE_TYPE source_type:
                        FRC1_SOURCE,    timer use frc1 isr as isr source.
                        NMI_SOURCE,     timer use nmi isr as isr source.
u8 req:
                        0,  not autoload,
                        1,  autoload mode,
* Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR hw_timer_init(FRC1_TIMER_SOURCE_TYPE source_type, u8 req)
{
    if (req == 1) {
        RTC_REG_WRITE(FRC1_CTRL_ADDRESS,
                      FRC1_AUTO_LOAD | DIVDED_BY_16 | FRC1_ENABLE_TIMER | TM_EDGE_INT);
    } else {
        RTC_REG_WRITE(FRC1_CTRL_ADDRESS,
                      DIVDED_BY_16 | FRC1_ENABLE_TIMER | TM_EDGE_INT);
    }

    if (source_type == NMI_SOURCE) {
        ETS_FRC_TIMER1_NMI_INTR_ATTACH(hw_timer_isr_cb);
    } else {
        ETS_FRC_TIMER1_INTR_ATTACH(hw_timer_isr_cb, NULL);
    }

    TM1_EDGE_INT_ENABLE();
    ETS_FRC1_INTR_ENABLE();
}
