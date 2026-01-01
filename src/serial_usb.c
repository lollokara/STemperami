#include "serial_usb.h"
#include "usbd_cdc_if.h"
#include <string.h>

#define RING_BUFFER_SIZE 128
static volatile uint8_t ring_buffer[RING_BUFFER_SIZE];
static volatile uint16_t ring_head = 0;
static volatile uint16_t ring_tail = 0;

void Serial_Print(const char *str) {
    // Note: CDC_Transmit_FS is non-blocking but might return USBD_BUSY.
    // ideally we should wait or retry.
    // We implement a timeout using a loop counter to avoid dependency on Systick in broken states.
    uint8_t status;
    volatile uint32_t i = 0;
    // Approx 100ms timeout at 72MHz (assuming ~10 cycles per loop, 7.2M loops -> 100ms)
    // USBD_BUSY means the buffer is still being sent.
    const uint32_t MAX_WAIT_LOOPS = 1000000;

    do {
        status = CDC_Transmit_FS((uint8_t*)str, strlen(str));
        if (status == USBD_BUSY) {
            i++;
        } else {
            break;
        }
    } while(i < MAX_WAIT_LOOPS);
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
