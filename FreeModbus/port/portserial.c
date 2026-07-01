#include "port.h"
#include "mb.h"
#include "mbport.h"
#include "usart.h"
#include "main.h"

/* 临界区嵌套计数 */
volatile uint8_t ucCriticalNesting = 0;

void vMBPortEnterCritical(void)
{
    if (ucCriticalNesting == 0) {
        __disable_irq();
    }
    ucCriticalNesting++;
}

void vMBPortExitCritical(void)
{
    ucCriticalNesting--;
    if (ucCriticalNesting == 0) {
        __enable_irq();
    }
}

/* 单字节收发缓冲 (static确保HAL中断读取时指针有效, 避免栈上局部变量悬空指针) */
static uint8_t ucRxByte;
static uint8_t ucTxByte;

/* RS485方向控制宏 */
#define RS485_TX_MODE()  HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_SET)
#define RS485_RX_MODE()  HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET)

BOOL
xMBPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity)
{
    /* UART已由CubeMX的MX_USART1_UART_Init()初始化, 这里只需确保处于接收模式 */
    RS485_RX_MODE();
    return TRUE;
}

void
vMBPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
    if (xRxEnable) {
        /* 切换到接收模式 */
        RS485_RX_MODE();
        /* 启动单字节接收 */
        HAL_UART_Receive_IT(&huart1, &ucRxByte, 1);
    }
    if (xTxEnable) {
        /* 切换到发送模式 */
        RS485_TX_MODE();
        /* 启动发送链 - 调用状态机发送首字节 */
        /* pxMBFrameCBTransmitterEmpty() -> xMBRTUTransmitFSM() -> xMBPortSerialPutByte() */
        pxMBFrameCBTransmitterEmpty();
    }
    if (!xRxEnable && !xTxEnable) {
        /* 同时禁用收发 (eMBRTUStop调用) - 不需要切换RS485方向 */
    }
}

BOOL
xMBPortSerialPutByte(CHAR ucByte)
{
    /* 单字节发送 - HAL会在TXE时写入DR, 在TC时调用TxCpltCallback
     * 必须使用static变量: HAL_UART_Transmit_IT存储的是数据指针而非副本,
     * TXE中断在函数返回后才触发读取, 栈上局部变量此时已失效 */
    ucTxByte = (uint8_t)ucByte;
    if (HAL_UART_Transmit_IT(&huart1, &ucTxByte, 1) == HAL_OK) {
        return TRUE;
    }
    return FALSE;
}

BOOL
xMBPortSerialGetByte(CHAR *pucByte)
{
    /* 从HAL接收缓冲中取出字节 */
    *pucByte = (CHAR)ucRxByte;
    return TRUE;
}

/* ===== HAL 弱回调重写 ===== */

/* 接收完成回调 - 每收到1字节触发 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        /* 通知FreeModbus状态机有新字节到达 */
        /* pxMBFrameCBByteReceived() -> xMBRTUReceiveFSM() -> xMBPortSerialGetByte() */
        pxMBFrameCBByteReceived();
        /* 重新启动单字节接收 */
        HAL_UART_Receive_IT(&huart1, &ucRxByte, 1);
    }
}

/* 发送完成回调 - 每字节TC(Transmission Complete)触发 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        /* 通知FreeModbus状态机发送缓冲已空 */
        /* pxMBFrameCBTransmitterEmpty() -> xMBRTUTransmitFSM():
         *   - 如果还有字节: xMBPortSerialPutByte() -> HAL_UART_Transmit_IT() 继续发送
         *   - 如果没有更多字节: vMBPortSerialEnable(TRUE, FALSE) 切换回接收模式
         *     (此时TC已触发, PE3拉低安全) */
        pxMBFrameCBTransmitterEmpty();
    }
}
