#pragma once
#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <fstream>
#include <string>
#include <filesystem>
#include<WS2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
#include <thread>
#include "request.h"
#include "response.h"
#define CLIENT_ID_SIZE 16
#define AES_ENC_SIZE 128
#define AES_SIZE 16
#define FILE_NAME_LENGTH 255
#define RESPOND_SIZE 512
class Operation {
private:
	int crcFailCounter = 0;
	char AES[AES_SIZE];
	std::string privetKey;
	std::string clientName;
	char clientID[CLIENT_ID_SIZE];
public:
	std::string getFileName();
	std::string getServerIp();
	void updateMeFile(std::string clientPrivetKey);
	int getServerPort();
	std::string getClientName();
	SOCKET createSocket();
	std::string  encrypt( char* fileContent,int size);
	char* sendFileRequest(request& req);
	char*  registertionRequest( request& req);
	char*  sendPublicKeyRequest(request& req);
	void createNewRequest(SOCKET s,uint16_t code);
	void getNewRespond(SOCKET s);
	void getClientID(char* clientID);
	char* CRCpayload(request& req);
	~Operation();
	std::string hexifyID(const char* clientID);
	void decryptAES(char* encryptedAES, char* AESfinal);
};