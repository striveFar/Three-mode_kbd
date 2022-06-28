#ifndef __KEY_SCAN_H
#define __KEY_SCAN_H

#include <stdbool.h>
#include <stdint.h>

#define Key_S0                      (R32_PA_PIN&GPIO_Pin_0)
#define Key_S1                      (R32_PA_PIN&GPIO_Pin_1)
#define Key_S2                      (R32_PA_PIN&GPIO_Pin_2)
#define Key_S3                      (R32_PA_PIN&GPIO_Pin_3)
#define Key_S4                      (R32_PA_PIN&GPIO_Pin_4)
#define Key_S5                      (R32_PA_PIN&GPIO_Pin_5)

#define LONGKEY_TIME                600
#define SHORTKEY_TIME               100

#define KEY_TYPE_8                  8
#define KEY_TYPE_16                  16

void keyInit(void);
bool readKeyVal(void);
void RstAllPins(void);

#endif
