#include "mb.h"
#include "mbport.h"

/* 保持寄存器 (功能码 03/06/10/23) */
#define REG_HOLDING_START 1
#define REG_HOLDING_NREGS 100
static USHORT usRegHoldingBuf[REG_HOLDING_NREGS] = {
    /* 前10个填测试数据便于验证 */
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004,
    0x0005, 0x0006, 0x0007, 0x0008, 0x0009
};

/* 输入寄存器 (功能码 04) */
#define REG_INPUT_START 1
#define REG_INPUT_NREGS 100
static USHORT usRegInputBuf[REG_INPUT_NREGS];

/* 线圈 (功能码 01/05/0F) */
#define REG_COILS_START 1
#define REG_COILS_SIZE 100
static UCHAR ucRegCoilsBuf[(REG_COILS_SIZE + 7) / 8];

/* 离散输入 (功能码 02) */
#define REG_DISCRETE_START 1
#define REG_DISCRETE_SIZE 100
static UCHAR ucRegDiscreteBuf[(REG_DISCRETE_SIZE + 7) / 8];

eMBErrorCode
eMBRegHoldingCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode)
{
    eMBErrorCode eStatus = MB_ENOERR;
    USHORT usRegIndex;

    if ((usAddress >= REG_HOLDING_START) &&
        (usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS)) {
        usRegIndex = usAddress - REG_HOLDING_START;
        switch (eMode) {
        case MB_REG_READ:
            while (usNRegs > 0) {
                *pucRegBuffer++ = (UCHAR)(usRegHoldingBuf[usRegIndex] >> 8);
                *pucRegBuffer++ = (UCHAR)(usRegHoldingBuf[usRegIndex] & 0xFF);
                usRegIndex++;
                usNRegs--;
            }
            break;
        case MB_REG_WRITE:
            while (usNRegs > 0) {
                usRegHoldingBuf[usRegIndex] = *pucRegBuffer++ << 8;
                usRegHoldingBuf[usRegIndex] |= *pucRegBuffer++;
                usRegIndex++;
                usNRegs--;
            }
            break;
        }
    } else {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

eMBErrorCode
eMBRegInputCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs)
{
    eMBErrorCode eStatus = MB_ENOERR;
    USHORT usRegIndex;

    if ((usAddress >= REG_INPUT_START) &&
        (usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS)) {
        usRegIndex = usAddress - REG_INPUT_START;
        while (usNRegs > 0) {
            *pucRegBuffer++ = (UCHAR)(usRegInputBuf[usRegIndex] >> 8);
            *pucRegBuffer++ = (UCHAR)(usRegInputBuf[usRegIndex] & 0xFF);
            usRegIndex++;
            usNRegs--;
        }
    } else {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

eMBErrorCode
eMBRegCoilsCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode)
{
    eMBErrorCode eStatus = MB_ENOERR;
    int iRegIndex;
    UCHAR ucBits[2];
    USHORT usBitIndex;

    if ((usAddress >= REG_COILS_START) &&
        (usAddress + usNCoils <= REG_COILS_START + REG_COILS_SIZE)) {
        usBitIndex = (USHORT)(usAddress - REG_COILS_START);
        switch (eMode) {
        case MB_REG_READ:
            while (usNCoils > 0) {
                ucBits[0] = 0;
                ucBits[1] = 0;
                for (iRegIndex = 0; iRegIndex < 8 && usNCoils > 0; iRegIndex++, usNCoils--) {
                    if (ucRegCoilsBuf[usBitIndex / 8] & (1 << (usBitIndex % 8))) {
                        ucBits[0] |= (1 << iRegIndex);
                    }
                    usBitIndex++;
                }
                *pucRegBuffer++ = ucBits[0];
            }
            break;
        case MB_REG_WRITE:
            while (usNCoils > 0) {
                ucBits[0] = *pucRegBuffer++;
                for (iRegIndex = 0; iRegIndex < 8 && usNCoils > 0; iRegIndex++, usNCoils--) {
                    if (ucBits[0] & (1 << iRegIndex)) {
                        ucRegCoilsBuf[usBitIndex / 8] |= (1 << (usBitIndex % 8));
                    } else {
                        ucRegCoilsBuf[usBitIndex / 8] &= ~(1 << (usBitIndex % 8));
                    }
                    usBitIndex++;
                }
            }
            break;
        }
    } else {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

eMBErrorCode
eMBRegDiscreteCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNDiscrete)
{
    eMBErrorCode eStatus = MB_ENOERR;
    int iRegIndex;
    USHORT usBitIndex;

    if ((usAddress >= REG_DISCRETE_START) &&
        (usAddress + usNDiscrete <= REG_DISCRETE_START + REG_DISCRETE_SIZE)) {
        usBitIndex = (USHORT)(usAddress - REG_DISCRETE_START);
        while (usNDiscrete > 0) {
            *pucRegBuffer = 0;
            for (iRegIndex = 0; iRegIndex < 8 && usNDiscrete > 0; iRegIndex++, usNDiscrete--) {
                if (ucRegDiscreteBuf[usBitIndex / 8] & (1 << (usBitIndex % 8))) {
                    *pucRegBuffer |= (1 << iRegIndex);
                }
                usBitIndex++;
            }
            pucRegBuffer++;
        }
    } else {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}
