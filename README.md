# UDP Socket Programming
@Author: Neha Raj

### Pre-requisites: 
Install gcc for C program compiler 
 
### Steps: 
- Copy the files 'client.c', 'server.c', 'fcs.h' and 'packet_payload.txt' to the desired location. 
- Run the below commands from the above location's terminal to compile the C programs: 
    `gcc server.c -o server`
    `gcc client.c -o client`
    `gcc fcs.h -o fcs`
- First the server should be started. To start the server, run: 
    `./server`
- In a new terminal window, run below for running the client program: 
    `./client` 
- Packets would start transmitting and output would be visible

### Description

- File client.c is responsible for creating request packets and sending them to the server/access point. 
- File server.c is responsible for receiving request packets and processing them and responding back with respective response packets. 
- File fcs.h is the header file that contains the logic for Frame Check Sequence caluclation for the data packets. FCS calculatiion and verification is done in both client and access point side. The verification is done before and after the frame is sent and recieved respectively.

## OUTPUTS

At the client side, a user menu with options for different type of packets to be sent to server/AP is displayed.

 <img width="452" alt="image" src="https://github.com/NA0724/UDP_SocketProgramming/assets/115744904/cc16f536-b806-4fd4-b907-ee8fccaf6c00">


Then as per the options selected, the client sends a packet to the server and the server responds accordingly.





1.	Association Request and Response 

 
<img width="452" alt="image" src="https://github.com/NA0724/UDP_SocketProgramming/assets/115744904/389c3c53-da57-4a82-a3f0-db7477db9549">


2.	Probe Request and Response 

 
<img width="452" alt="image" src="https://github.com/NA0724/UDP_SocketProgramming/assets/115744904/aaf58b41-3599-41bf-b044-dea626de652b">










3.	RTS and CTS 

 
<img width="452" alt="image" src="https://github.com/NA0724/UDP_SocketProgramming/assets/115744904/72bee5cc-32b7-48d1-a575-95a175afbf14">


4.	Data Packet and ACK

 
<img width="452" alt="image" src="https://github.com/NA0724/UDP_SocketProgramming/assets/115744904/e6cc5e7c-d6aa-46d5-836e-d15c43170d60">











5.	FCS Error Handling 

 
<img width="452" alt="image" src="https://github.com/NA0724/UDP_SocketProgramming/assets/115744904/5403687a-5c29-4c2b-90ae-e9ef7700c3a2">


6.	Sending multiple fragmented frames

 <img width="452" alt="image" src="https://github.com/NA0724/UDP_SocketProgramming/assets/115744904/71d01be3-4c90-4e14-bbab-cab0bf5919e8">


  

 

 
![image](https://github.com/NA0724/UDP_SocketProgramming/assets/115744904/a64f3b93-9722-48f6-85ce-488632dc4b7d)


 

 

4 error fragmented frames

 
<img width="452" alt="image" src="https://github.com/NA0724/UDP_SocketProgramming/assets/115744904/fc6dc398-e4eb-44be-8420-0356c6c6de8d">




After ack_timer and retry_counter runs out, error message is displayed:

 <img width="452" alt="image" src="https://github.com/NA0724/UDP_SocketProgramming/assets/115744904/e6b1fe1c-3838-4f95-9a95-2892bcb2ce75">




If user enters a wrong option, the client throws and error and exits.

 <img width="452" alt="image" src="https://github.com/NA0724/UDP_SocketProgramming/assets/115744904/abdef085-d5d7-4049-a64b-adae281323f4">

