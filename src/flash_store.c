#include "flash_store.h"
#include <string.h>

void FlashStore_Init(void) {
    // Optional: Check if page is blank or valid, if not, write defaults?
    // For now, load handles defaults if magic is invalid.
}

static uint8_t IsFlashAddressValid(void) {
    uint16_t flash_size_kb = *(__IO uint16_t *)0x1FFFF7E0;
    uint32_t flash_end_addr = 0x08000000 + (flash_size_kb * 1024);
    if (FLASH_STORAGE_PAGE_ADDR >= flash_end_addr) {
        return 0; // Invalid
    }
    return 1; // Valid
}

void FlashStore_Save(AppSettings *settings) {
    if (!IsFlashAddressValid()) return;

    HAL_FLASH_Unlock();

    // Erase the page
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PageError;

    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = FLASH_STORAGE_PAGE_ADDR;
    EraseInitStruct.NbPages     = 1;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK) {
        // Handle Error
        HAL_FLASH_Lock();
        return;
    }

    // Write data
    uint32_t address = FLASH_STORAGE_PAGE_ADDR;
    uint32_t *data = (uint32_t *)settings;
    int size = sizeof(AppSettings) / sizeof(uint32_t);

    for (int i = 0; i < size; i++) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data[i]) != HAL_OK) {
            // Handle Error
            break;
        }
        address += 4;
    }

    HAL_FLASH_Lock();
}

void FlashStore_Load(AppSettings *settings) {
    if (!IsFlashAddressValid()) {
        // Defaults if flash is invalid
        settings->min_temp = 30.0f;
        settings->max_temp = 60.0f;
        settings->magic_number = APP_SETTINGS_MAGIC;
        return;
    }

    AppSettings *stored_settings = (AppSettings *)FLASH_STORAGE_PAGE_ADDR;

    if (stored_settings->magic_number == APP_SETTINGS_MAGIC) {
        memcpy(settings, stored_settings, sizeof(AppSettings));
    } else {
        // Defaults
        settings->min_temp = 30.0f;
        settings->max_temp = 60.0f;
        settings->magic_number = APP_SETTINGS_MAGIC;
    }
}
