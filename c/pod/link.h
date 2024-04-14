#ifndef __POD_LINK_H__
#define __POD_LINK_H__

#include <stdint.h>
#include "podtp.h"

typedef struct {
    int16_t speed[4];
} motor_2040_control_t;

typedef struct {
    int32_t count[4];
} motor_encoder_t;

bool linkBufferPutChar(uint8_t c);
void linkGetPacket(PodtpPacket *packet);
uint8_t *linkPackData(uint8_t *data, uint8_t *length);

#endif // __POD_LINK_H__