#ifndef __DS18B20_H
#define __DS18B20_H

#include "main.h"

void DS18B20_Init(void);
void DS18B20_StartConversion(void);
float DS18B20_ReadTemp(void);
float DS18B20_GetTemp(void); // Blocking version

#endif /* __DS18B20_H */
