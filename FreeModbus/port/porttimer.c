#include "port.h"
#include "mb.h"
#include "mbport.h"
#include "tim.h"

BOOL
xMBPortTimersInit(USHORT usTim1Timerout50us)
{
    /* TIM4 PSC=71已由CubeMX设定(1MHz/1µs精度) */
    /* FreeModbus传入50µs单位的超时值, 转换为1µs tick */
    __HAL_TIM_SET_AUTORELOAD(&htim4, (uint32_t)usTim1Timerout50us * 50);
    return TRUE;
}

/* 注意: 去掉inline关键字, 避免GCC C11链接问题 */
void
vMBPortTimersEnable(void)
{
    /* 停止定时器(如果在运行) */
    __HAL_TIM_DISABLE(&htim4);
    /* 复位计数器 */
    __HAL_TIM_SET_COUNTER(&htim4, 0);
    /* 清除挂起的中断标志 */
    __HAL_TIM_CLEAR_IT(&htim4, TIM_IT_UPDATE);
    /* 使能更新中断 */
    __HAL_TIM_ENABLE_IT(&htim4, TIM_IT_UPDATE);
    /* 启动定时器 */
    __HAL_TIM_ENABLE(&htim4);
}

void
vMBPortTimersDisable(void)
{
    __HAL_TIM_DISABLE(&htim4);
    __HAL_TIM_DISABLE_IT(&htim4, TIM_IT_UPDATE);
    __HAL_TIM_SET_COUNTER(&htim4, 0);
    __HAL_TIM_CLEAR_IT(&htim4, TIM_IT_UPDATE);
}

/* ===== HAL 弱回调重写 ===== */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM4) {
        /* 单次模式: 到期即停 */
        __HAL_TIM_DISABLE(&htim4);
        __HAL_TIM_DISABLE_IT(&htim4, TIM_IT_UPDATE);
        /* 通知FreeModbus定时器到期 */
        /* pxMBPortCBTimerExpired() -> xMBRTUTimerT35Expired() */
        (void)pxMBPortCBTimerExpired();
    }
}
