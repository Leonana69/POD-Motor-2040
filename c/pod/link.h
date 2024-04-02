#ifndef __POD_LINK_H__
#define __POD_LINK_H__

#include <stdint.h>
#include "podtp.h"

typedef struct {
    uint16_t speed[4];
} motor_2040_control_t;

bool linkBufferPutChar(uint8_t c);
void linkGetPacket(PodtpPacket *packet);

#endif // __POD_LINK_H__