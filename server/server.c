/* 
 * Name: Abdullah M Alharbi
 * Course-Section: CS 360 01, Spring 2019 
 * Collaborators: none
 * Description: 
 * This program acts as the server. It first opens a text file and reads in the 
 * joke-punchline pairs into a linked list. Then, the server listens on port 5454 and 
 * waits for a packet to be sent from the client after accepting the connection. After the
 * first packet is received from the client, the server sends back a packet to greet the 
 * client. Then it waits for a second packet from the client, and once received, the 
 * server sends its second packet with a payload randomly picked from the linked list. 
 * Then the server waits for a third packet from the client. Once received, the server 
 * sends its third and final packet before the client automatically disconnects, which 
 * contains the punchline to the payload in the packet sent previously by the server. 
 * The linked list containing the jokes read from the text file will be deallocated after 
 * the server sends its final packet to the client. If, by any chance, the protocol is not
 * followed, the server sends out a prepared ‘protocol violation’ packet to the client and
 * disconnects.
 */

#include <sys/types.h>
#include <sys/socket.h> 

#include <arpa/inet.h> 
#include <netinet/in.h>

#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "protocol.h"

enum CONSTANTS {
    MAX_PENDING = 5, 
    MAX_LINE = 256,
    SERVER_PORT = 5454 
};

int readJokeFile(struct Joke **jokes); // not in the header file, specific to the server
struct Joke * randomlyPickJoke(struct Joke **head); // not in the header file, specific to the server
static void sig_handler(int);
volatile sig_atomic_t cleanup_exit;

int 
main ()
{
    struct sockaddr_in sa;
    int s;

    cleanup_exit = false;

    /* initalize address data structure*/
    memset(&sa, '\0', sizeof (sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons(SERVER_PORT);

    /* passive open (the server): socket, bind, listen, accept */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        err(1, "Socket open failed");
    
    if ((bind(s, (struct sockaddr*) &sa, sizeof (sa))) == -1)
        err(2, "Socket bind failed");

    if ((listen(s, MAX_PENDING)) == -1)
        err(3, "listen failed");

    signal(SIGTERM, sig_handler);

    while (!cleanup_exit) {
        struct sockaddr_storage addr;
        struct Packet packetFromClient = {"", ""};
        struct Packet packetToClient = {"", ""};
        struct Joke *head = NULL;
        struct Joke *pickedJokeNode = NULL;
        socklen_t addrlen;
        int numberOfJokes = 0;
        int new_s;

        addrlen = sizeof (addr);
        if ((new_s = accept(s, (struct sockaddr*) &addr, &addrlen)) == -1)
            err(4, "accept failed");
        
        memset(&packetFromClient.opcode, '\0', sizeof(packetFromClient.opcode));
        memset(&packetFromClient.payload, '\0', sizeof(packetFromClient.payload));
        memset(&packetToClient.opcode, '\0', sizeof(packetToClient.opcode));
        memset(&packetToClient.payload, '\0', sizeof(packetToClient.payload));
        
        numberOfJokes = readJokeFile(&head);
        
        recv(new_s, &packetFromClient, sizeof(packetFromClient), 0);

        // if packet received is opcode 1 'tell me a joke', send opcode 2
        // 'knock, knock'. Else, send a protocol violation packet
        if (strcmp(packetFromClient.opcode, opcodes[0][0]) == 0){
            preparePacket(&packetToClient, 2);
            send(new_s, &packetToClient, sizeof(packetToClient), 0);
        }
        else {
            makeProtocolViolationPacket(&packetToClient);
            send(new_s, &packetToClient, sizeof(packetToClient), 0);
            deallocateLinkedList(&head);
            close(new_s);
            exit(1);
        }
        
        recv(new_s, &packetFromClient, sizeof(packetFromClient), 0);
        
        // if packet received is opcode 3 'who's there', send opcode 4, which is
        // a joke picked randomly from the linked list. 
        // Else, send a protocol violation packet
        if (strcmp(packetFromClient.opcode, opcodes[2][0]) == 0){
            pickedJokeNode = randomlyPickJoke(&head);
            memset(&packetToClient.opcode, '\0', sizeof(packetToClient.opcode));
            memset(&packetToClient.payload, '\0', sizeof(packetToClient.payload));
            preparePacketWithJoke(&packetToClient, 4, pickedJokeNode);
            send(new_s, &packetToClient, sizeof(packetToClient), 0);
            
            recv(new_s, &packetFromClient, sizeof(packetFromClient), 0);
            // if packet received is opcode 5 '.. who?', send opcode 6, which is
            // the punchline that comes with the randomly picked joke. 
            // Else, send a protocol violation packet
            if (strcmp(packetFromClient.opcode, opcodes[4][0]) == 0){
                memset(&packetToClient.opcode, '\0', sizeof(packetToClient.opcode));
                memset(&packetToClient.payload, '\0', sizeof(packetToClient.payload));
                preparePacketWithJoke(&packetToClient, 6, pickedJokeNode);
                send(new_s, &packetToClient, sizeof(packetToClient), 0);
                deallocateLinkedList(&head); // deallocating the linked list
            }
            else {
                makeProtocolViolationPacket(&packetToClient);
                send(new_s, &packetToClient, sizeof(packetToClient), 0);
                deallocateLinkedList(&head);
                close(new_s);
                exit(1);
            }
        }
        else {
            makeProtocolViolationPacket(&packetToClient);
            send(new_s, &packetToClient, sizeof(packetToClient), 0);
            deallocateLinkedList(&head);
            close(new_s);
            exit(1);
        }

        close(new_s);
    }
    
    close(s);    
    
    return 0;
}

/*
 * Reads jokes into a linked list. Each node is of type 'struct Joke'.
 * Returns the number of joke pairs read from the file. This function is not 
 * included in the header file since only the server uses it. 
 */
int readJokeFile(struct Joke **head) {
    int numberOfJokes = 0;
    struct Joke *node = (struct Joke *)malloc(sizeof(struct Joke)); 
    FILE *file;
    file = fopen("jokefile.txt", "r");
    
    if (file) {
        
        while (!feof(file)) {
            
            if (numberOfJokes == 0){ // assigning the head the first time
                
                fgets(node->whoResponse, whoResponseSize, file);
                fgets(node->punchLine, punchLineSize, file);
                
                if (strlen(node->whoResponse) > payloadSize || strlen(node->punchLine) > payloadSize) 
                    exit(1);
                
                node->next = NULL;
                (*head) = node;
            }
            else {
								node->next = malloc(sizeof(struct Joke)); 
                node = node->next;

                fgets(node->whoResponse, whoResponseSize, file);
                fgets(node->punchLine, punchLineSize, file);

                node->next = NULL;
            }
            numberOfJokes++;
        }
        fclose(file);
    } else {
        exit(1);
    }
    
    return numberOfJokes;
}

/*
 * Takes the head of the linked list and traverses to the next node 'jokeNumber'
 * times. The node stopped at will be returned as the picked joke from the 
 * linked list. 'jokeNumber' is determined by the arc4random function modulo 11
 * to return a number between 0 and 10, where 0 represents the first node in the
 * linked list. This function is not included in the header file since only the
 * server uses it.
 */
struct Joke * randomlyPickJoke(struct Joke **head) {
    
    struct Joke *pickedJokeNode = (struct Joke *)malloc(sizeof(struct Joke));
    int jokeNumber = -1;
    
    jokeNumber = arc4random() % 11; // from 0 to 10
    
    pickedJokeNode = *head;
    
    if (jokeNumber != 0) {
        
        for (int i = 0; i < jokeNumber; i++){   
            pickedJokeNode = pickedJokeNode->next;
        }
    }
    
    return pickedJokeNode;
}

/*
 * Signal handler function. 
 */
static void
sig_handler(int sig){

    int save_errno = errno;
	
    switch(sig) {
    case SIGINT :
        puts("CTRL-C pressed.");
        break;
    case SIGTERM:
        puts("SIGTERM received.");
        cleanup_exit = true;
        break;
    default:
        printf("%d not handled\n", sig);
    }
	
    errno = save_errno;
}

