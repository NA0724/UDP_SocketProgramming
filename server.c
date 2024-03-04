#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "fcs.h"

#define PORT 8080
#define BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"

#define START_OF_PACKET_ID 0XFFFF 			                                // Start of Packet identifier
#define END_OF_PACKET_ID 0XFFFF 			                                // End of Packet identifier


uint8_t originator_mac[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xDD};       //Address 2: Final receiver address MAC address (example: AABBCCDDEEDD)
uint8_t final_receiver_mac[6]  = {0x12, 0x45, 0xCC, 0xDD, 0xEE, 0x88};           //Address 1: Originator Address, (example:1245CCDDEE88)
uint8_t access_point_mac[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xDD};         //Address 3: Access point address (example: AABBCCDDEEDD)


// Define the frame_control structure
struct __attribute__((packed)) frame_control {
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
struct __attribute__((packed)) dataPacket{
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
    data.payload.frame_control.to_ds = 0;
    data.payload.frame_control.from_ds = 1;
    memset(data.payload.address4, 0, sizeof(data.payload.address4)); // Address 4 set to 000000000000
    data.end_ID = END_OF_PACKET_ID;
    return data;
}

void printStructureBytes(const void *ptr, size_t size) {
    const unsigned char *byte = ptr;
    printf("Structure bytes: ");
    for (size_t i = 0; i < size; ++i) {
        printf("%02X ", byte[i]);
    }
    printf("\n");
}

// Function to set frame control fields for an Association Response
void setAssociationResponse(struct ieee80211_frame *frame,  uint16_t durationID) {
    frame->frame_control.type = 0; // Management type
    frame->frame_control.sub_type = 1; // Association Request subtype
    frame->durationID = durationID; 
    memcpy(frame->address1, final_receiver_mac, sizeof(frame->address1));
    memcpy(frame->address2, originator_mac, sizeof(frame->address2));
    memcpy(frame->address3, access_point_mac, sizeof(frame->address3));
}

// Function to set frame control fields for a Probe Request
void setProbeResponse(struct ieee80211_frame *frame) {
    //frame->frame_control.type = 0; // Management type
    //frame->frame_control.sub_type = 4; // Probe Request subtype (binary 0100)
    frame->durationID = 0; 
    memcpy(frame->address1, final_receiver_mac, sizeof(frame->address1));
    memcpy(frame->address2, originator_mac, sizeof(frame->address2));
    memcpy(frame->address3, access_point_mac, sizeof(frame->address3));
}

// Function to set frame control fields for a RTS (Request To Send)
void setCTS(struct ieee80211_frame *frame) {
    frame->frame_control.type = 1; // Control type
    frame->frame_control.sub_type = 12; // CTS subtype (binary 1100)
    frame->durationID = 3; 
    memcpy(frame->address1, final_receiver_mac, sizeof(frame->address1));
    memcpy(frame->address2, originator_mac, sizeof(frame->address2));
    memcpy(frame->address3, access_point_mac, sizeof(frame->address3));
}

// Function to set frame control fields for sending Data
void setAck(struct ieee80211_frame *frame) {
    frame->frame_control.type = 1; // Data type
    frame->frame_control.sub_type = 13; // Sub Type = 1101
    frame->durationID = 1; 
    memcpy(frame->address1, final_receiver_mac, sizeof(frame->address1));
    memcpy(frame->address2, originator_mac, sizeof(frame->address2));
    memcpy(frame->address3, access_point_mac, sizeof(frame->address3));
}

//print the packet
void showPacket(struct dataPacket data) {
    
    printf("\n------Packet Information------ \n");
    printf("Address 1: %02X%02X%02X%02X%02X%02X\n",
           data.payload.address1[0], data.payload.address1[1], data.payload.address1[2],
           data.payload.address1[3], data.payload.address1[4], data.payload.address1[5]);
   
    printf("Address 2: %02X%02X%02X%02X%02X%02X\n",
           data.payload.address2[0], data.payload.address2[1], data.payload.address2[2],
           data.payload.address2[3], data.payload.address2[4], data.payload.address2[5]);
   
    printf("Address 3: %02X%02X%02X%02X%02X%02X\n",
           data.payload.address3[0], data.payload.address3[1], data.payload.address3[2],
           data.payload.address3[3], data.payload.address3[4], data.payload.address3[5]);

    printf("Address 4: %02X%02X%02X%02X%02X%02X\n",
           data.payload.address4[0], data.payload.address4[1], data.payload.address4[2],
           data.payload.address4[3], data.payload.address4[4], data.payload.address4[5]);
   
    printf("Type: %u\n", data.payload.frame_control.type);
    printf("Sub Type: %u\n", data.payload.frame_control.sub_type);
    printf("FCS: %u\n", data.payload.fcs);
    printf("Duration ID: %u\n", data.payload.durationID);
}

int verifyFCS(struct dataPacket data, uint32_t receivedFCS) {
    // Calculate FCS excluding the FCS field itself
    uint32_t calculatedFCS = getCheckSumValue(&data.payload, sizeof(data.payload) , 0, sizeof(data.payload.fcs));
    printf("Calculated FCS: %u\n", calculatedFCS);
    return (calculatedFCS == receivedFCS) ? 1 : 0;
}

int main() {
    int socket_fd;
    struct sockaddr_in cliaddr;
    struct dataPacket requestPacket, responsePacket;
    memset(&requestPacket, 0, sizeof(requestPacket)); 
    memset(&responsePacket, 0, sizeof(responsePacket));
    char *message;
    int n, len;
    socklen_t cliaddr_len = sizeof(cliaddr);
    unsigned int type; // 2 bits
    unsigned int sub_type; // 4 bits

    // Creating socket file descriptor
    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    memset(&cliaddr, 0, sizeof(cliaddr));
    // Filling server information
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(PORT);
    cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(socket_fd,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
    printf("Server started successfully........\n");
    
    while(1)
    {
        printf("----------------------------------------------------------------\n");
        n = recvfrom(socket_fd, &requestPacket, sizeof(requestPacket), 0 , (struct sockaddr *)&cliaddr, &cliaddr_len);
        printf(" Packet recieved from client........\n");
        showPacket(requestPacket);
        //printStructureBytes(&requestPacket, sizeof(requestPacket));
        if (verifyFCS(requestPacket, requestPacket.payload.fcs)) {
            type = requestPacket.payload.frame_control.type;
            sub_type = requestPacket.payload.frame_control.sub_type;
            responsePacket = fillDataPacket();
            if (type == 0 && sub_type == 0) {
                setAssociationResponse(&responsePacket.payload, requestPacket.payload.durationID);
                printf("Sending Association Response...\n");
            } else if (type == 0 && sub_type == 4){
                setProbeResponse(&responsePacket.payload);
                printf("Sending Probe Response...\n");
            } else if (type == 1 && sub_type == 11){
                setCTS(&responsePacket.payload);
                if (requestPacket.payload.durationID == 12){
                    responsePacket.payload.durationID = 11;
                }
                printf("Sending CTS Response...\n");
            } else if (type == 2 && sub_type == 0 ){
                setAck(&responsePacket.payload);
                memcpy(responsePacket.payload.payload, requestPacket.payload.payload, sizeof(requestPacket.payload.payload));
                printf("Sending ACK Response...\n");
            } 
            responsePacket.payload.fcs = getCheckSumValue(&responsePacket.payload, sizeof(responsePacket.payload), 0, sizeof(responsePacket.payload.fcs));
            if (sendto(socket_fd, &responsePacket, sizeof(responsePacket), 0, (struct sockaddr *)&cliaddr, cliaddr_len) < 0) {
                perror("sendto failed");
            } else {
                printf("Response sent.\n");
                showPacket(responsePacket);
            }
        } else{
            // FCS mismatch, display error
                printf("Error: FCS (Frame Check Sequence) Error.\n");
        } 
    }
    printf("\n-----------------------------------------------------------------------------------------\n");
    
    
}
