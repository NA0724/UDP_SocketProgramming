#include <stdio.h>
#include <unistd.h>
#include <cstdint>
#include <string.h>
#include "fcs.h"

#define START_OF_PACKET_ID 0XFFFF 			                                // Start of Packet identifier
#define END_OF_PACKET_ID 0XFFFF 			                                // End of Packet identifier


// Define the frame_control structure
struct frame_control {
    unsigned int protocol_version: 2; // 2 bits
    unsigned int type: 2; // 2 bits
    unsigned int sub_type: 4; // 4 bits
    unsigned int to_ds: 1; // 1 bit
    unsigned int from_ds: 1; // 1 bit
    unsigned int more_fragment: 1; // 1 bit
    unsigned int retry: 1; // 1 bit
    unsigned int power_management: 1; // 1 bit
    unsigned int more_data: 1; // 1 bit
    unsigned int wep: 1; // 1 bit
    unsigned int order: 1; // 1 bit
};

// Define the IEEE 802.11 frame structure
struct __attribute__((packed)) ieee80211_frame {
    struct frame_control frame_control; // 2 bytes
    uint16_t durationID; // 2 bytes
    uint8_t address1[6]; // 6 bytes
    uint8_t address2[6]; // 6 bytes
    uint8_t address3[6]; // 6 bytes
    uint16_t sequence_control; // 2 bytes
    uint8_t address4[6]; // 6 bytes
    uint8_t payload[2312]; // Variable size, up to 2312 bytes for this example
    uint32_t fcs; // 4 bytes, assuming you will calculate this
};

//DATA packet format
struct dataPacket{
    uint16_t start_ID;
    struct ieee80211_frame payload;
    uint16_t end_ID;
};

//fill data in dataPacket to create the data packet
struct dataPacket fillDataPacket(){
    struct dataPacket data;
    struct frame_control fc;
    data.start_ID = START_OF_PACKET_ID;
    fc.protocol_version = 00;
    fc.more_data = 0;
    fc.more_fragment = 0;
    fc.retry = 0;
    fc.power_management = 0;
    fc.wep = 0;
    fc.order = 0;
    data.payload.sequence_control = 0x0000;
    data.end_ID = END_OF_PACKET_ID;
    memset(data.payload.address4, 0, sizeof(data.payload.address4)); // Address 4 set to 000000000000
    data.payload.fcs = getCheckSumValue(&data.payload, sizeof(data.payload), 0, sizeof(data.payload.fcs));
    data.payload.frame_control.to_ds = 1;
    data.payload.frame_control.from_ds = 0;
    return data;
}
