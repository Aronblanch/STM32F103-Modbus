#ifndef _PORT_H
#define _PORT_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
#define PR_BEGIN_EXTERN_C extern "C" {
#define PR_END_EXTERN_C }
#else
#define PR_BEGIN_EXTERN_C
#define PR_END_EXTERN_C
#endif

#define INLINE                      inline

/* 临界区 - 嵌套计数 */
extern volatile uint8_t ucCriticalNesting;
void vMBPortEnterCritical(void);
void vMBPortExitCritical(void);
#define ENTER_CRITICAL_SECTION()  vMBPortEnterCritical()
#define EXIT_CRITICAL_SECTION()   vMBPortExitCritical()

/* 类型定义 */
typedef uint8_t BOOL;
typedef unsigned char UCHAR;
typedef char CHAR;
typedef uint16_t USHORT;
typedef int16_t SHORT;
typedef uint32_t ULONG;
typedef int32_t LONG;

#ifndef TRUE
#define TRUE            1U
#endif

#ifndef FALSE
#define FALSE           0U
#endif

#endif /* _PORT_H */
