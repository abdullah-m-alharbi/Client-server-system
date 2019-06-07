/* 
 * Name: Abdullah M Alharbi
 * Course-Section: CS 360 01, Spring 2019 
 * Collaborators: none
 * Description: header file.
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

extern char *opcodes [7][2];

enum sizes {
    whoResponseSize = 127,  
    punchLineSize = 127,  
    opcodeSize = 2,      
    payloadSize = 127   
};

struct Packet {
    char opcode [opcodeSize];
    char payload [payloadSize];
};

struct Joke {
    char whoResponse[whoResponseSize]; 
    char punchLine[punchLineSize]; 
    struct Joke *next;
};

void deallocateLinkedList(struct Joke **head);
void makeProtocolViolationPacket(struct Packet *packet);
void preparePacket(struct Packet *packet, int opcode);
void preparePacketWithJoke(struct Packet *packetToClient, int opcode, struct Joke *jokeNode);
void makeResponseWhoPacket(struct Packet *packetFromServer, struct Packet *packetToServer);
void printPayload(struct Packet *packet);

#endif /* PROTOCOL_H */

