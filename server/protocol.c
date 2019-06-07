/* 
 * Name: Abdullah M Alharbi
 * Course-Section: CS 360 01, Spring 2019 
 * Collaborators: none
 * Description: header implementation file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "protocol.h"

// Global array of C-strings containing opcodes and corresponding payloads.
// Payloads that are missing are variable and will be supplied appropriately from the 
// server and client.
char *opcodes [7][2] = {
    {"1", "tell me a joke"},
    {"2", "knock, knock"},
    {"3", "who's there"},
    {"4", ""},
    {"5", " who"},
    {"6", ""},
    {"7", "Protocol violation"}
};

/*
 * Deallocates the linked list.
 */
void deallocateLinkedList(struct Joke **head) {
    struct Joke *current = *head;
    struct Joke *next;

    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    *head = NULL;
}

/*
* Puts opcode '7' in the opcode field of the packet and "Protocol violation" in the 
* payload field.
*/
void makeProtocolViolationPacket(struct Packet *packet){
    strcpy(packet->opcode, opcodes[6][0]);
    strcpy(packet->payload, opcodes[6][1]);
}

/*
 * Takes the packet in the parameter and fills the opcode field with the parameter opcode,
 * and fills the payload field with the corresponding payload. All opcodes are defined in
 * the beginning of this file.
 */
void preparePacket(struct Packet *packet, int opcode){		
    strcpy(packet->opcode, opcodes[opcode-1][0]);
    strcpy(packet->payload, opcodes[opcode-1][1]);
}

/*
 * Takes the packet in the parameter and fills opcode field with the parameter opcode, and
 * fills the payload field with the parameter joke.
 */
void preparePacketWithJoke(struct Packet *packetToClient, int opcode, struct Joke *jokeNode){
    
    if (opcode == 4){
        strcpy(packetToClient->opcode, opcodes[opcode-1][0]);
        strcpy(packetToClient->payload, jokeNode->whoResponse);
    }
    else {
        strcpy(packetToClient->opcode, opcodes[opcode-1][0]);
        strcpy(packetToClient->payload, jokeNode->punchLine);        
    }
}

/*
 * Takes the second parameter packet and fills it with the opcode and payload of the first
 * parameter packet, and adds the string " who?" to the payload of the second parameter. 
 * 
 */
void makeResponseWhoPacket(struct Packet *packetFromServer, struct Packet *packetToServer){

		int len = 0;
		    
    strcpy(packetToServer->opcode, opcodes[4][0]);
    strcpy(packetToServer->payload, packetFromServer->payload);

		len = strlen(packetToServer->payload);

		packetToServer->payload[len-1] = ' ';
		packetToServer->payload[len] = 'w';
		packetToServer->payload[len+1] = 'h';
		packetToServer->payload[len+2] = 'o';
		packetToServer->payload[len+3] = '?';
}


/*
 * Prints the payload of the parameter packet.
 */
void printPayload(struct Packet *packet){
    
    if (packet->payload[strlen(packet->payload)-1] == '\n')
	  		printf("%s", packet->payload);    
	  else
	  		printf("%s\n", packet->payload);
}

