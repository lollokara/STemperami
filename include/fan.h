#ifndef __FAN_H
#define __FAN_H

#include "main.h"

void Fan_Init(void);
void Fan_SetSpeed(uint8_t percent);
void Fan_Update(float current_temp, float min_temp, float max_temp);

#endif /* __FAN_H */
