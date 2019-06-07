Abdullah M Alharbi
Client-server system

Description:
This system implements the protocol below. 

Protocol:
Opcode		Description						Example payload
1		Hello server, sent from client				“tell me a joke”
2		Hello client, sent from the server			“knock, knock”
3		Ask who setup, sent from client Who 			“who’s there?”
4		Who response, sent from server				“yoda lady”	(from joke file)
5		ask “response, who?”, sent from client			“yoda lady, who?” (1st part from joke file)
6		Joke punchline, sent from server			“good job yodeling” (from joke file)
7		Error, may be sent from client or server		“Protocol violation”


Server description:  the server first opens a text file and reads in the joke-punchline pairs into a linked list. 
Then, the server listens on port 5454 and waits for a packet to be sent from the client after accepting the connection. 
After the first packet is received from the client, the server sends back a packet to greet the client. Then it waits 
for a second packet from the client, and once received, the server sends its second packet with a payload randomly 
picked from the linked list. Then the server waits for a third packet from the client. Once received, the server sends 
its third and final packet before the client automatically disconnects, which contains the punchline to the payload in 
the second packet sent by the server. The linked list containing the jokes read from the text file will be deallocated
after the server sends its final packet to the client. If, by any chance, the protocol is not followed, the server sends 
out a prepared ‘protocol violation’ packet to the client and disconnects.

Client description: the client starts the conversation by sending a packet with payload “tell me a joke” to the server after
connection and prints out the payload to the console. Then, the client waits for a packet from the server with the payload 
“knock, knock”. After the client receives the first packet from the server, it prints out the payload to the console and 
sends its second packet containing the payload “who’s there?” and prints out the payload. Then, the client waits for a second
packet from the server. After the client receives the second packet, it prints out the payload to the console and 
appends “, who?” to its payload and sends it back to the server and prints out the payload. Then the client waits for a third 
packet from the server. The server then sends out a joke punchline packet to the client. The client then prints out the 
payload to the console and disconnects. If this protocol is not followed, a prepared ‘protocol violation’ packet is sent to 
the server, and the connection is terminated by the client.
