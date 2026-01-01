#include "serial_usb.h"
#include "usbd_cdc_if.h"
#include <string.h>

#define RING_BUFFER_SIZE 128
static volatile uint8_t ring_buffer[RING_BUFFER_SIZE];
static volatile uint16_t ring_head = 0;
static volatile uint16_t ring_tail = 0;

void Serial_Print(const char *str) {
    // Note: CDC_Transmit_FS is non-blocking but might return USBD_BUSY.
    // Ideally we should wait or retry. For now, simple retry loop.
    uint8_t status;
    uint32_t timeout = HAL_GetTick() + 100;
    do {
        status = CDC_Transmit_FS((uint8_t*)str, strlen(str));
    } while(status == USBD_BUSY && HAL_GetTick() < timeout);
}

void Serial_PrintLn(const char *str) {
    Serial_Print(str);
    Serial_Print("\r\n");
}

int _write(int file, char *ptr, int len) {
    CDC_Transmit_FS((uint8_t*)ptr, len);
    return len;
}

// Push to buffer
void Serial_RxCallback(uint8_t byte) {
    uint16_t next_head = (ring_head + 1) % RING_BUFFER_SIZE;
    if (next_head != ring_tail) {
        ring_buffer[ring_head] = byte;
        ring_head = next_head;
    }
}

int Serial_Available(void) {
    return (ring_head != ring_tail);
}

uint8_t Serial_Read(void) {
    if (ring_head == ring_tail) return 0;
    uint8_t data = ring_buffer[ring_tail];
    ring_tail = (ring_tail + 1) % RING_BUFFER_SIZE;
    return data;
}
