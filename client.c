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
#define TOTAL_FRAGMENTS 10

#define START_OF_PACKET_ID 0XFFFF 			                                // Start of Packet identifier
#define END_OF_PACKET_ID 0XFFFF 			                                // End of Packet identifier
#define ACK_TIMEOUT 3						                                // ack_timer

uint8_t final_receiver_mac[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xDD};       //Address 1 field: Final receiver address MAC address (example: AABBCCDDEEDD)
uint8_t originator_mac[6] = {0x12, 0x45, 0xCC, 0xDD, 0xEE, 0x88};           //Address 2: Originator Address, (example:1245CCDDEE88)
uint8_t access_point_mac[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xDD};         //Address 3: Access point address (example: AABBCCDDEEDD)
uint8_t incorrect_mac[6] = {0x00, 0x00, 0x45, 0x67, 0xE8, 0xD0};

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
    memcpy(data.payload.address1, final_receiver_mac, sizeof(data.payload.address1));
    memcpy(data.payload.address2, originator_mac, sizeof(data.payload.address2));
    memcpy(data.payload.address3, access_point_mac, sizeof(data.payload.address3));
    return data;
}

// Function to set frame control fields for an Association Request
void setAssociationRequest(struct ieee80211_frame *frame) {
    frame->frame_control.type = 0; // Management type
    frame->frame_control.sub_type = 0; // Association Request subtype
    frame->durationID = 0; 
}

// Function to set frame control fields for a Probe Request
void setProbeRequest(struct ieee80211_frame *frame) {
    frame->frame_control.type = 0; // Management type
    frame->frame_control.sub_type = 4; // Probe Request subtype (binary 0100)
    frame->durationID = 0; 
}

// Function to set frame control fields for a RTS (Request To Send)
void setRTS(struct ieee80211_frame *frame) {
    frame->frame_control.type = 1; // Control type
    frame->frame_control.sub_type = 11; // RTS subtype (binary 1011)
    frame->durationID = 4; 
}

// Function to set frame control fields for sending Data
void setData(struct ieee80211_frame *frame, const char* dataSegment, int segmentSize) {
    memcpy(frame->payload, dataSegment, segmentSize);
    frame->frame_control.type = 2; // Data type
    frame->frame_control.sub_type = 0; // Subtype for Data
    frame->durationID = 2; 
}

//print  the packet
void showPacket(struct dataPacket data) {
    
    printf("\n\033[34m------Packet Information------ \n");
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
    printf("More fragments: %u\n", data.payload.frame_control.more_fragment);
    printf("Sequence Control: %u\n", data.payload.sequence_control);
    printf("\n\033[34m----------------------------------------------- \033[0m\n");
}

// populate fields of fragmented frames
struct dataPacket fillDataPacketWithSegmentAndFragmentation(uint16_t sequenceNumber, uint8_t fragmentNumber, const char* dataSegment, int segmentSize) {
    struct dataPacket data = fillDataPacket();
    setData(&data.payload, dataSegment, segmentSize);
    data.payload.sequence_control = (sequenceNumber << 5) | (fragmentNumber & 0x0F); // Shift sequence number and add fragment number
    data.payload.frame_control.order = 1;
    return data;
}



// Send first set of 5 correct fragmented frames
struct dataPacket* sendMultipleFrames() {
    struct dataPacket* packets = malloc(TOTAL_FRAGMENTS * sizeof(struct dataPacket));
    if (packets == NULL) {
        perror("Memory allocation failed");
        return NULL; // Allocation failed
    }
    FILE *file = fopen("payload.txt", "r");
    if (file == NULL) {
        perror("Failed to open file");
        free(packets);
        return NULL;
    }
    char line[1024];
    for (int i = 0; i < TOTAL_FRAGMENTS; ++i) {
        if (fgets(line, sizeof(line), file) == NULL) break; // End of file or error
        line[strcspn(line, "\n")] = 0; 
        packets[i] = fillDataPacketWithSegmentAndFragmentation(1, i, line, strlen(line));
        // Adjusting Duration ID and More Fragment bit for each packet
        packets[i].payload.durationID = 12 - i;
        packets[i].payload.frame_control.more_fragment = (i < TOTAL_FRAGMENTS - 1) ? 1 : 0;
        if ( i == 5) {
            packets[i] = fillDataPacketWithSegmentAndFragmentation(2, i, line, strlen(line));
        } else if (i == 6){
            // Out of Sequence Error Frame
            packets[i].payload.frame_control.order = 0;
            packets[i].payload.sequence_control = 707; 
        } else if ( i == 7) {
            //Incorrect MAC Address Error Frame
            packets[i] = fillDataPacketWithSegmentAndFragmentation(2, i+1, line, strlen(line));
            memcpy(&packets[i].payload.address1, incorrect_mac, sizeof(packets[i].payload.address1));
        } else if (i == 8) {
            // Duplicate Frame
            packets[i].payload.sequence_control = 707; 
        } else if (i == 9) {
            // End of Frame Error
            packets[i] = fillDataPacketWithSegmentAndFragmentation(2, i+1, line, strlen(line));
            packets[i].end_ID = 0;
        }  
        packets[i].payload.fcs = getCheckSumValue(&packets[i].payload, sizeof(packets[i].payload), 0, sizeof(packets[i].payload.fcs));
        
    }
    fclose(file);
    return packets;
}

int processDataPacket(int option){
    int socket_fd;
    struct sockaddr_in servaddr;
    struct dataPacket responsePacket, requestPacket;
    memset(&requestPacket, 0, sizeof(requestPacket)); 
    memset(&responsePacket, 0, sizeof(responsePacket));
    int fcsoption = 0;
    int multiple = 0;
    FILE *fp;
    char line[255];
    int retryCounter = 0;
	int n = 0;
	struct timeval timer;
    socklen_t servaddr_len = sizeof(servaddr);
    const char* dataSegment = "Sample Data Packet";
    int segmentSize = strlen(dataSegment);

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
            printf("\033[35mSending Association Request to Server......\033[0m\n");
            // associationRequest message
            setAssociationRequest(&requestPacket.payload);
        }else if (option ==2){
            printf("\033[35mSending Probe Request to Server......\033[0m\n");
            // probeRequest message
            setProbeRequest(&requestPacket.payload);
        } else if (option ==3){
            printf("\033[35mSending RTS Request to Server......\033[0m\n");
            // RTS request message
            setRTS(&requestPacket.payload);
        } else if (option == 4){
            printf("\033[35mSending Data Packet to Server......\033[0m\n");
            // Data packet
            setData(&requestPacket.payload, dataSegment, segmentSize);
        } else if (option == 5){
            fcsoption = 1;
            setData(&requestPacket.payload, dataSegment, segmentSize);
            printf("\033[35mData Packet sent to Server......\033[0m\n");
        } else if (option == 6){
            printf("\033[35m RTS frame sent to server....\033[0m\n");
            setRTS(&requestPacket.payload);
            requestPacket.payload.durationID = 12;
            multiple = 1;
        }
        requestPacket.payload.fcs = getCheckSumValue(&requestPacket.payload, sizeof(requestPacket.payload), 0, sizeof(requestPacket.payload.fcs));
        showPacket(requestPacket);
        //printStructureBytes(&requestPacket, sizeof(requestPacket));
        sendto(socket_fd, &requestPacket, sizeof(requestPacket), 0, (struct sockaddr *)&servaddr, servaddr_len);
        n = recvfrom(socket_fd, &responsePacket, sizeof(responsePacket), 0 , (struct sockaddr *)&servaddr, &servaddr_len);
        printf("\n Response from Server : "); 
        if(n <= 0){
            printf("\033[31mERROR  Server does not respond\033[0m\n");
            printf("\n\033[35m Sending Packet Again..............\033[0m\n");
            retryCounter++;
        } else {
            uint32_t calculatedFCS = getCheckSumValue(&responsePacket.payload, sizeof(responsePacket.payload), 0, sizeof(responsePacket.payload.fcs));
            if (calculatedFCS == responsePacket.payload.fcs) {
                if (fcsoption == 1 && responsePacket.payload.frame_control.type == 1 && responsePacket.payload.frame_control.sub_type == 13){
                    printf("\033[32mACK received for valid frame\033[0m\n");
                    showPacket(responsePacket);
                    // Now send a frame with incorrect FCS
                    struct dataPacket faultyPacket = fillDataPacket(); // Fill your packet as usual
                    faultyPacket.payload.fcs = 0xFFFFFFFF; // Incorrect FCS
                    printf("\n****************************************************\n");
                    printf("\n\033[31m Sending Invalid data packet request\033[0m\n");
                    showPacket(faultyPacket);
                    sendto(socket_fd, &faultyPacket, sizeof(faultyPacket), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
                } else if (multiple == 1){
                    printf("\033[32mCTS frame recieved from server......\033[0m\n");
                    showPacket(responsePacket);
                    struct dataPacket* multipleFrames = sendMultipleFrames();
                    if (multipleFrames == NULL) {
                        return EXIT_FAILURE;
                    }
                    printf("\n****************************************************\n");
                    printf("\033[35m Sending fragmented frames to server......\033[0m\n");
                    for (int i = 0; i < TOTAL_FRAGMENTS; i++) {
                        //multipleFrames[i].payload.fcs = getCheckSumValue(&multipleFrames[i].payload, sizeof(multipleFrames[i].payload), 0, sizeof(multipleFrames[i].payload.fcs));
                        sendto(socket_fd, &multipleFrames[i], sizeof(multipleFrames[i]), 0, (const struct sockaddr*)&servaddr, sizeof(servaddr));
                    }
                    free(multipleFrames);
                    printf("\033[35m Total fragmented frames sent: %d\033[0m\n", TOTAL_FRAGMENTS);
                }
                else {
                    // FCS matches, process the packet
                    printf("\033[32mReceived frame with valid FCS.\033[0m\n");
                    showPacket(responsePacket);
                }
            } else {
                // FCS mismatch, display error
                printf("\033[31mError: FCS (Frame Check Sequence) Error.\033[0m\n");
            }
        } 
    }                 
    
    if(retryCounter >= 3)
    {
        printf("\n\033[31m ERROR : No ACK received from AP \033[0m\n");
        printf("\n");
        exit(0);
    }
    printf("\n-----------------------------------------------------------------------------------------\n");
    
    close(socket_fd);
    return 0;
}

int main() {
    int number;
    printf("\n****************************************************\n");
    printf(" Choose the type of message to send from client to server. \n Enter the option number (1, 2, 3, 4, 5 or 6) \n");
    printf("---------------------------------------------------------------------\n");
    printf("1. Association Request\n");
    printf("2. Probe Request\n");
    printf("3. RTS Request \n");
    printf("4. Data Request\n");
    printf("5. Verify FCS Error Handling on Faulty Data Request Packet.\n");
    printf("6. Send Multiple Frames\n");
    scanf("%d", &number);
    if (number >=1 && number <=6) {
        processDataPacket(number);
    } else {
        printf("\n\033[31m ERROR Retry again with correct option \033[0m\n");
        printf("\n");
        exit(1);
    }
    return 0;
    
}

