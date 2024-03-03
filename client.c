#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "fcs.h"

#define PORT 8080
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

#define START_OF_PACKET_ID 0XFFFF 			                                // Start of Packet identifier
#define END_OF_PACKET_ID 0XFFFF 			                                // End of Packet identifier
#define ACK_TIMEOUT 3						                                // ack_timer

uint8_t final_receiver_mac[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xDD};       //Address 1 field: Final receiver address MAC address (example: AABBCCDDEEDD)
uint8_t originator_mac[6] = {0x12, 0x45, 0xCC, 0xDD, 0xEE, 0x88};           //Address 2: Originator Address, (example:1245CCDDEE88)
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
    data.end_ID = END_OF_PACKET_ID;
    memset(data.payload.address4, 0, sizeof(data.payload.address4)); // Address 4 set to 000000000000
    data.payload.frame_control.to_ds = 1;
    data.payload.frame_control.from_ds = 0;
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

// Function to set frame control fields for an Association Request
void setAssociationRequest(struct ieee80211_frame *frame) {
    frame->frame_control.type = 0; // Management type
    frame->frame_control.sub_type = 0; // Association Request subtype
    frame->durationID = 0; 
    memcpy(frame->address1, final_receiver_mac, sizeof(frame->address1));
    memcpy(frame->address2, originator_mac, sizeof(frame->address2));
    memcpy(frame->address3, access_point_mac, sizeof(frame->address3));
}

// Function to set frame control fields for a Probe Request
void setProbeRequest(struct ieee80211_frame *frame) {
    frame->frame_control.type = 0; // Management type
    frame->frame_control.sub_type = 4; // Probe Request subtype (binary 0100)
    frame->durationID = 0; 
    memcpy(frame->address1, final_receiver_mac, sizeof(frame->address1));
    memcpy(frame->address2, originator_mac, sizeof(frame->address2));
    memcpy(frame->address3, access_point_mac, sizeof(frame->address3));
}

// Function to set frame control fields for a RTS (Request To Send)
void setRTS(struct ieee80211_frame *frame) {
    frame->frame_control.type = 1; // Control type
    frame->frame_control.sub_type = 11; // RTS subtype (binary 1011)
    frame->durationID = 4; 
    memcpy(frame->address1, final_receiver_mac, sizeof(frame->address1));
    memcpy(frame->address2, originator_mac, sizeof(frame->address2));
    memcpy(frame->address3, access_point_mac, sizeof(frame->address3));
}

// Function to set frame control fields for sending Data
void setData(struct ieee80211_frame *frame) {
    frame->frame_control.type = 2; // Data type
    frame->frame_control.sub_type = 0; // Subtype for Data
    frame->durationID = 2; 
    memcpy(frame->address1, final_receiver_mac, sizeof(frame->address1));
    memcpy(frame->address2, originator_mac, sizeof(frame->address2));
    memcpy(frame->address3, access_point_mac, sizeof(frame->address3));
}

//print  the packet
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
}


int processDataPacket(int option){
    int socket_fd;
    struct sockaddr_in servaddr;
    struct dataPacket responsePacket, requestPacket;
    memset(&requestPacket, 0, sizeof(requestPacket)); 
    memset(&responsePacket, 0, sizeof(responsePacket));
    char *message;
    FILE *fp;
    char line[255];
    int retryCounter = 0;
	int n = 0;
	struct timeval timer;
    socklen_t servaddr_len = sizeof(servaddr);

    // Creating socket file descriptor
    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);

    timer.tv_sec = ACK_TIMEOUT;						//Socket timeout
    timer.tv_usec = 0;
    setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timer,sizeof(struct timeval));

    requestPacket = fillDataPacket();
    while(n <= 0 && retryCounter < 3)
    { 
        if (option ==1){
            printf("Sending Association Request to Server......\n");
            // associationRequest message
            setAssociationRequest(&requestPacket.payload);
        }else if (option ==2){
            printf("Sending Probe Request to Server......\n");
            // probeRequest message
            setProbeRequest(&requestPacket.payload);
        } else if (option ==3){
            printf("Sending RTS Response to Server......\n");
            // RTS request message
            setRTS(&requestPacket.payload);
        } else if (option == 4){
            printf("Sending Data Packet to Server......\n");
            // Data packet
            setData(&requestPacket.payload);
        }
        requestPacket.payload.fcs = getCheckSumValue(&requestPacket.payload, sizeof(requestPacket.payload), 0, sizeof(requestPacket.payload.fcs));
        showPacket(requestPacket);
        //printStructureBytes(&requestPacket, sizeof(requestPacket));
        sendto(socket_fd, &requestPacket, sizeof(requestPacket), 0, (struct sockaddr *)&servaddr, servaddr_len);
        printf("\nFrame sent with calculated FCS.\n");
        
        n = recvfrom(socket_fd, &responsePacket, sizeof(responsePacket), 0 , (struct sockaddr *)&servaddr, &servaddr_len);
        printf("\n Response from Server : "); 
        if(n <= 0){
            printf("ERROR  Server does not respond");
            printf("\n Sending Packet Again.............. \n");
                retryCounter++;
        } else {
            uint32_t calculatedFCS = getCheckSumValue(&responsePacket.payload, sizeof(responsePacket.payload), 0, sizeof(responsePacket.payload.fcs));
            if (calculatedFCS == responsePacket.payload.fcs) {
            // FCS matches, process the packet
                printf("Received frame with valid FCS.\n");
                printf("\n Association Response from server... \n");
                showPacket(responsePacket);
            } else {
                // FCS mismatch, display error
                printf("Error: FCS (Frame Check Sequence) Error.\n");
            }
        }                 
    }
    if(retryCounter >= 3)
    {
        printf("\n ERROR : Access Point does not respond");
        exit(0);
    }
    printf("\n-----------------------------------------------------------------------------------------\n");
    
    close(socket_fd);
    return 0;
}



int main() {
    int number;
    printf(" Choose the type of message to send from client to server. Enter the option number (1, 2, 3 or 4) \n");
    printf("---------------------------------------------------------------------\n");
    printf("1. Association Request\n");
    printf("2. Probe Request\n");
    printf("3. RTS Request \n");
    printf("4. Data Request\n");
    scanf("%d", &number);
    if (number >=1 && number <=4) {
        processDataPacket(number);
    } else {
        printf("\n Retry again with correct option");
        exit(1);
    }
    return 0;
    
}

