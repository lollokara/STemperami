#ifndef __FLASH_STORE_H
#define __FLASH_STORE_H

#include "main.h"

// Define Flash Page for storage (Use the last page of 64KB Flash)
// STM32F103C8 has 64KB Flash. Page size is 1KB.
// Last page starts at 0x0800FC00.
#define FLASH_STORAGE_PAGE_ADDR 0x0800FC00

typedef struct {
    float min_temp;
    float max_temp;
    uint32_t magic_number; // To check if initialized
} AppSettings;

#define APP_SETTINGS_MAGIC 0xDEADBEEF

void FlashStore_Init(void);
void FlashStore_Save(AppSettings *settings);
void FlashStore_Load(AppSettings *settings);

#endif /* __FLASH_STORE_H */
