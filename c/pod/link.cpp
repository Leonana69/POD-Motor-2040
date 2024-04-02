#include "podtp.h"

typedef enum {
    PODTP_STATE_START_1,
    PODTP_STATE_START_2,
    PODTP_STATE_LENGTH,
    PODTP_STATE_RAW_DATA,
    PODTP_STATE_CRC_1,
    PODTP_STATE_CRC_2
} LinkState;

static PodtpPacket packet = { 0 };
void linkGetPacket(PodtpPacket *packet) {
    *packet = ::packet;
}

bool linkBufferPutChar(uint8_t c) {
    static LinkState state = PODTP_STATE_START_1;
    static uint8_t length = 0;
    static uint8_t check_sum[2] = { 0 };
    
    switch (state) {
        case PODTP_STATE_START_1:
            if (c == PODTP_START_BYTE_1) {
                state = PODTP_STATE_START_2;
            }
            break;
        case PODTP_STATE_START_2:
            state = (c == PODTP_START_BYTE_2) ? PODTP_STATE_LENGTH : PODTP_STATE_START_1;
            break;
        case PODTP_STATE_LENGTH:
            length = c;
            if (length > PODTP_MAX_DATA_LEN || length == 0) {
                state = PODTP_STATE_START_1;
            } else {
                packet.length = 0;
                check_sum[0] = check_sum[1] = c;
                state = PODTP_STATE_RAW_DATA;
            }
            break;
        case PODTP_STATE_RAW_DATA:
            packet.raw[packet.length++] = c;
            check_sum[0] += c;
            check_sum[1] += check_sum[0];
            if (packet.length >= length) {
                state = PODTP_STATE_CRC_1;
            }
            break;
        case PODTP_STATE_CRC_1:
            state = (c == check_sum[0]) ? PODTP_STATE_CRC_2 : PODTP_STATE_START_1;
            break;
        case PODTP_STATE_CRC_2:
            state = PODTP_STATE_START_1;
            if (c == check_sum[1])
                return true;
            break;
    }
    return false;
}