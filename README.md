# UDP Socket Programming
@Author: Neha Raj

### Pre-requisites: 
Install gcc for C program compiler 
 
### Steps: 
- Copy the files 'client.c', 'server.c' and 'packet_payload.txt' to the desired location. 
- Run the below commands from the above location's terminal to compile the C programs: 
    `gcc server.c -o server`
    `gcc client.c -o client`
- First the server should be started. To start the server, run: 
    `./server`
- In a new terminal window, run below for running the client program: 
    `./client` 
- Packets would start transmitting and output would be visible

## OUTPUTS

At the client side, a user menu with options for different type of packets to be sent to server/AP is displayed.

 

Then as per the options selected, the client sends a packet to the server and the server responds accordingly.





1.	Association Request and Response 

 


2.	Probe Request and Response 

 










3.	RTS and CTS 

 


4.	Data Packet and ACK

 











5.	FCS Error Handling 

 


6.	Sending multiple fragmented frames

 

 

 

 

4 error fragmented frames

 




After ack_timer and retry_counter runs out, error message is displayed:

 



If user enters a wrong option, the client throws and error and exits.

 
