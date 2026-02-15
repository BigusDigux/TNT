#ifndef ENCODER_H
#define ENCODER_H

#include "main.h"

void ENCODER_Init(void);
int32_t ENCODER_GetLeft(void);
int32_t ENCODER_GetRight(void);
void ENCODER_ResetLeft(void);
void ENCODER_ResetRight(void);

void ENCODER_Reset(void);

#endif
