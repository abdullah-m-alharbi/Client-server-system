/* 
 * Name: Abdullah M Alharbi
 * Course-Section: CS 360 01, Spring 2019 
 * Collaborators: none
 * Description: this program acts as the client. It starts the conversation by sending a 
 * packet with payload "knock, knock" to the server, and will follow the protocol 
 * described by the assignment sheet. If the protocol is not followed, a protocol 
 * violation packet is sent to the server, and an exit is performed with code 1.
 */

#include <sys/types.h>
#include <sys/socket.h> 

#include <arpa/inet.h> 
#include <netdb.h>
#include <netinet/in.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "protocol.h"

enum CONSTANTS 
{
    MAX_LINE = 256,
    SERVER_PORT = 5454
};

int 
main(int argc, char *argv[])
{
    struct Packet packetFromServer = {"", ""};
    struct Packet packetToServer = {"", ""};
    
    /************** Begin setting socket and connection **************/
    struct sockaddr_in sa;
    struct hostent *hostp;
    sa.sin_port = htons(SERVER_PORT);
    int s;
    char *host;

    if (argc == 2)
        host = argv[1];
    else
        errx(1, "Error, how to use: program_name server_name\n");

    /* get the IP address of the server (parameter from command line) */
    hostp = gethostbyname(host);
    if (!hostp)
        errx(2, "Unable to resolve %s\n", host);

    /* initialize address data structure */
    memset(&sa, '\0', sizeof (sa));
    sa.sin_family = AF_INET;
    memcpy(&sa.sin_addr, hostp->h_addr, sizeof (sa.sin_addr));
    sa.sin_port = htons(SERVER_PORT);

    /* active open: socket, connect */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        errx(3, "Socket open failed");

    if ((connect(s, (struct sockaddr *) &sa, sizeof (sa))) == -1) {
        close(s);
        errx(4, "Connect failed");
    }
    /************** End setting socket and connection **************/
    
    memset(&packetFromServer.opcode, '\0', sizeof(packetFromServer.opcode));
    memset(&packetFromServer.payload, '\0', sizeof(packetFromServer.payload));
    memset(&packetToServer.opcode, '\0', sizeof(packetToServer.opcode));
    memset(&packetToServer.payload, '\0', sizeof(packetToServer.payload));

    // start the conversation with the server
    preparePacket(&packetToServer, 1);
    send(s, &packetToServer, sizeof(packetToServer), 0);
    printPayload(&packetToServer);
    recv(s, &packetFromServer, sizeof(packetFromServer), 0); // receiving the first message from the server
    
    
    // after receiving the first message from the server
    if (strcmp(packetFromServer.opcode, opcodes[1][0]) == 0){
        printPayload(&packetFromServer);
        memset(&packetToServer.opcode, '\0', sizeof(packetToServer.opcode));
        memset(&packetToServer.payload, '\0', sizeof(packetToServer.payload));
        preparePacket(&packetToServer, 3);    
        send(s, &packetToServer, sizeof(packetToServer), 0); // sending "who's there"
        printPayload(&packetToServer);
    }
    else {
        makeProtocolViolationPacket(&packetToServer);        
        send(s, &packetToServer, sizeof(packetToServer), 0);
        printPayload(&packetToServer);
        close(s);
        exit(1);
    }
    
    memset(&packetFromServer.opcode, '\0', sizeof(packetFromServer.opcode));
    memset(&packetFromServer.payload, '\0', sizeof(packetFromServer.payload));
    recv(s, &packetFromServer, sizeof(packetFromServer), 0); // receiving 'who response'
    
    // after receiving 'who response'
    if (strcmp(packetFromServer.opcode, opcodes[3][0]) == 0){
        printPayload(&packetFromServer);
        memset(&packetToServer.opcode, '\0', sizeof(packetToServer.opcode));
        memset(&packetToServer.payload, '\0', sizeof(packetToServer.payload));
				
				// sending 'server-response' followed by 'who?'
        makeResponseWhoPacket(&packetFromServer, &packetToServer); 
        send(s, &packetToServer, sizeof(packetToServer), 0);
        printPayload(&packetToServer);
    }
    else {
        makeProtocolViolationPacket(&packetToServer);
        send(s, &packetToServer, sizeof(packetToServer), 0);
        printPayload(&packetToServer);
        close(s);
        exit(1);
    }
    
    memset(&packetFromServer.opcode, '\0', sizeof(packetFromServer.opcode));
    memset(&packetFromServer.payload, '\0', sizeof(packetFromServer.payload));
    recv(s, &packetFromServer, sizeof(packetFromServer), 0);  // receiving punchline
    
    if (strcmp(packetFromServer.opcode, opcodes[5][0]) == 0)
        printPayload(&packetFromServer);
    else {
        makeProtocolViolationPacket(&packetToServer);
        send(s, &packetToServer, sizeof(packetToServer), 0);
        printPayload(&packetToServer);
        close(s);
        exit(1);
    }
    
    
    close(s);
    return 0;
}