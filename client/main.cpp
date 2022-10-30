#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include<WS2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
#include <thread>
#include "clientOpertions.h"

/*this example for client who ask register, if he registed he got me.info in folder
* and register request dont hurt the flow of main.
* he will get respond from server and save the UUID
* next he generate privete and pub key and send .
* when he get the 2nd responde he got the encrypted AES key .
* after get AES key we dycrypt it and then encrypt the file content .
* then we send file and wait for cksum from server , we send new request depands on cksum.
*/



int main() {
	SOCKET s;
	Operation client;
	s = client.createSocket();
	client.createNewRequest(s,request::code::REGISTER);
	client.getNewRespond(s);//get clientID respond
	client.createNewRequest(s, request::code::PUBLICKEY);
	client.getNewRespond(s);//get encrypted AES key 	
	closesocket(s);
	WSACleanup();

	//return 0;
}

#