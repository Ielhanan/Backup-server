#pragma once
#include "clientOpertions.h"
#include "RSAWrapper.h"
#include "AESWrapper.h"
#include "Base64Wrapper.h"
#include "CRC.h"
#include <iomanip>


//get File name from transfer.info 
std::string Operation::getFileName() {
	std::string filename;
	try {
		std::ifstream file;
		file.open("transfer.info");
		std::getline(file, filename);//port and ip
		std::getline(file, filename);//name
		std::getline(file, filename);//file path
		file.close();
	}

	catch (const std::exception&)
	{
		std::cout << "Error get file name from transfer.info" << std::endl;
	}

	return filename;
}


SOCKET Operation:: createSocket()
{
	try {
		int port=getServerPort();
		std::string ipAddress = getServerIp();
		WSADATA wsaData;
		int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
		SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		struct sockaddr_in sa = { 0 };
		sa.sin_family = AF_INET;
		sa.sin_port = htons(port);
		inet_pton(AF_INET, ipAddress.c_str(), &sa.sin_addr);
		int conection_status = connect(s, (struct sockaddr*)&sa, sizeof(sa));//

		return s;

	}
	catch (const std::exception&)
	{
		std::cout << "Error creat socket" << std::endl;
		return NULL;
	}
}

//encrypt file content with server AES key
std::string Operation::encrypt( char *fileContent ,int size) {
	try {
		AESWrapper aes((const unsigned char*)this->AES, AES_SIZE);
		std::string encrypted = aes.encrypt(fileContent, size);
		return encrypted;
	}
	catch (const std::exception&)
	{
		std::cout << "Error encrypt filecontent " << std::endl;
	}
	
}


//this methode handle a send file request returned packed payload
char* Operation::sendFileRequest(request& req) {
	int size=0;
	std::string fileName = this->getFileName();
	char* fileContent=NULL;
	try {
		//read file content and get his size
		std::ifstream in(fileName, std::ifstream::ate | std::ifstream::binary);
		size = in.tellg();
		fileContent =new char[size];
		memset(fileContent, 0, size);//
		in.clear();
		in.seekg(0,std::ios::beg);
		in.read(fileContent, size);
		in.close();		
	}
	catch (const std::exception&)
	{
		std::cout << "Error read filecontent from file desired to sent" << std::endl;
	}
	
	std::string encryptedFileContent = this->encrypt( fileContent,size);
	delete [] fileContent;//free memomry no nessecery anymore
	char* packedBuffer = new char[request::payloadSize::SENDFILE_SIZE + encryptedFileContent.size()];
	memset(packedBuffer, 0, request::payloadSize::SENDFILE_SIZE + encryptedFileContent.size());
	char* ptr = packedBuffer;
	
	memcpy(ptr, req.FReqHead.ReqHead.clientID, CLIENT_ID_SIZE);
	ptr += CLIENT_ID_SIZE;
	size_t encryptedsize = encryptedFileContent.size();
	memcpy(ptr, &encryptedsize, sizeof(uint32_t));
	ptr += sizeof(uint32_t);
	memcpy(ptr, fileName.c_str(), fileName.size());
	ptr += 255;
	memcpy(ptr, encryptedFileContent.c_str(), encryptedFileContent.size());
	req.FReqHead.ReqHead.payloadSize = 255 + 16 + 4 + encryptedFileContent.size();
	
	return packedBuffer;

}


//handle when get new respond
void Operation::getNewRespond(SOCKET s) {
	//need to fix error with get new data
	char buffer[RESPOND_SIZE];
	recv(s, buffer, RESPOND_SIZE, 0);
	respond res;
	res.unpackRespond(buffer);

	if (res.FResHead.ResHead.code == respond::resCode::REGISTER_SUCCSESS) {
		//need to updaete me file with new clientID
		memcpy(this->clientID, res.payload, CLIENT_ID_SIZE);
	}
	if (res.FResHead.ResHead.code == respond::resCode::REGISTER_FAILED) {
		//update client id  arrey from meFile
		this->getClientID(this->clientID);
	}
	if (res.FResHead.ResHead.code == respond::resCode::PUBLICKEY_SUCCSESS) {//need to decrypt the AES key 
		//we got back the encrypted AES key , we need to decrypt it and encrypte the desired
		//file content and send it to server.

		char* ptr = res.payload;
		char encAES[AES_ENC_SIZE] = { 0 };
		ptr += 16;//we want skeep the first 16 bytes they represent ID, to get the AES key
		memcpy(encAES, ptr, AES_ENC_SIZE);
		char* AES = new char[AES_SIZE];
		memset(AES, 0, AES_SIZE);
		this->decryptAES(encAES, AES);
		memcpy(this->AES, AES, AES_SIZE);
		delete[] AES;
		this->createNewRequest(s, request::code::SENDFILE);//we create new send file request
		this->getNewRespond(s);//waiting for respond with cksum

	}
	if (res.FResHead.ResHead.code == respond::resCode::SENDFILE_SUCCSESS) {
		//calc check sum and get new request depand result of checksum
		char* ptr = res.payload;
		std::string filepath = this->getFileName();
		uint32_t serverChecksome;
		//memcpy(&filepath, ptr + 20, FILE_NAME_LENGTH);
		memcpy(&serverChecksome, ptr + 20 + FILE_NAME_LENGTH, sizeof(uint32_t));
		CRC crc;
		uint32_t checksum = crc.calcCrc(filepath);
		if (checksum == serverChecksome) {
			this->crcFailCounter = 0;//delete previos fails
			this->createNewRequest(s, request::code::FIXCRC);
			this->getNewRespond(s);
			//send back succ crc
		}
		else {
			this->crcFailCounter += 1;
			if (this->crcFailCounter == 3) {
				std::cout << "error 3 times" << std::endl;
				this->createNewRequest(s, request::code::ERR4CRC);
			}
			else {
				this->createNewRequest(s, request::code::ERRCRC);//send server error crc
				this->createNewRequest(s, request::code::SENDFILE);//try to resent the file
				this->getNewRespond(s);//we wait to respond from server for new CRC
			}
			
		}
	}
}

	//get req and fill payload,returning packed payload
	char* Operation::registertionRequest(request & req) {
		try {
			char* payload = new char[255];
			memset(payload, 0, 255);
			req.payload = payload;
			this->clientName = this->getClientName();
			memcpy(req.payload, this->clientName.c_str(), this->clientName.size());
		}
		catch (const std::exception&)
		{
			std::cout << "Error on  register requset" << std::endl;

		}
		return req.payload;

	}


//get server ip  from transfer.info
std::string Operation::getServerIp() {
	std::string line;
	char tmp[16];//max is 16 chars for ip 
	try {
		std::ifstream file;
		file.open("transfer.info");
		
		std::getline(file, line);
		
		memcpy(tmp, line.c_str(), line.size());
		int i = 0;
		while (tmp[i] != ':') {//get the server ip untill reach : its the ip address
			i += 1;
		}
		tmp[i] = '\0';//null terminated
		file.close();
	}
	
	catch (const std::exception&)
	{
		std::cout << "Error read server ip from transfer.info" << std::endl;
	}
	std::string ip(tmp);
	return ip;

}

//get server port from transfer.info
int Operation::getServerPort() {
	std::string line;
	std::string tmp;
	char p[5]={0};//need 20 chars for port and server ip  
	try {
		std::ifstream file;
		file.open("transfer.info");

		std::getline(file, line);
		int i = 0;//some index like 
		while (line[i] != ':') {
			i += 1;
		}
		int j = 0;//some index like i 
		while (i < line.size()) {//copy only port
			p[j++] = line[++i];
		}
		
		file.close();
	}
	catch (const std::exception&)
	{
		std::cout << "Error read server port from transfer.info" << std::endl;
	}
	
	int port = std::stoi(p);
	return port;
}

//get client Name from transfer.info
std::string Operation:: getClientName() {
	std::string name;
	//need to read the name from file
	try {
		std::ifstream file;
		file.open("transfer.info", std::ios::in);
		std::getline(file, name);//ignroe the port and ip
		std::getline(file, name);
		file.close();
		std::ofstream meFile;
		std::string fileName = "Me.info";
		std::ifstream infile(fileName);
		if (infile.good() == 0) {//if me file not exist creat new one 
			meFile.open("Me.info");
			meFile << name << std::endl;
			meFile.close();
		}
	}
	catch (const std::exception&)
	{
		std::cout << "Error read client name from transfer.info" << std::endl;
	}
	
	return name;
}

// create pubkey and privete key , save it in mefile and send the pubkey , returned packed payload
char* Operation:: sendPublicKeyRequest(request& req) {
	try {
		RSAPrivateWrapper rsapriv;
		std::string privetKey = Base64Wrapper::encode(rsapriv.getPrivateKey());
		this->updateMeFile( privetKey);
		this->privetKey = privetKey;
		std::string pubkey = rsapriv.getPublicKey();
		char* payload = new char[request::payloadSize::PUBLICKEY_SIZE];
		memset(payload, 0, request::payloadSize::PUBLICKEY_SIZE);
		req.payload = payload;
		std::string clientname = this->getClientName();
		memcpy(payload, clientname.c_str(), clientname.size());
		payload += 255;
		memcpy(payload, pubkey.c_str(), pubkey.size());
	}
	catch (const std::exception&)
	{
		std::cout << "Error send public key request " << std::endl;
		return NULL;
	}
	return req.payload;
}

//requests 1104 1105 1106 got same payload , her we pack the payload and return it 
char* Operation::CRCpayload(request& req) {
	try {
		char clientID[CLIENT_ID_SIZE];
		memcpy(clientID, this->clientID, CLIENT_ID_SIZE);
		std::string fileName = this->getFileName();
		char* payload = new char[CLIENT_ID_SIZE + FILE_NAME_LENGTH];
		memset(payload, 0, CLIENT_ID_SIZE + FILE_NAME_LENGTH);
		req.payload = payload;
		memcpy(payload, clientID, CLIENT_ID_SIZE);
		payload += CLIENT_ID_SIZE;
		memcpy(payload, fileName.c_str(), fileName.size());
		return req.payload;
	}
	catch (const std::exception&)
	{
		std::cout << "Error send crc request " << std::endl;
		return NULL;
	}
}
//get client id from me.info file
void Operation::getClientID(char* clientID) {
	std::string id = "";
	try {
		std::ifstream file;
		file.open("Me.info", std::ios::in);
		std::getline(file, id);//ignore name
		std::getline(file, id);
		file.close();
		int counter = 0;
		for (size_t i = 0; i < id.length(); i += 2)  // +2 because we get two characters at a time
		{
			std::string byte_string(&id[i], 2);  // Construct temporary string for
			// the next two character from the input

			int byte_value = std::stoi(byte_string, nullptr, 16);  // Base 16
			clientID[counter++] = byte_value;
		}

	}
	catch (const std::exception&)
	{
		std::cout << "Error read client id from me.info" << std::endl;
	}
	
}

//creating new request , and send it at the end
void Operation::createNewRequest(SOCKET s, uint16_t code) {
	int size = HEADER_SIZE;
	request req;


	memcpy(&req.FReqHead.ReqHead.clientID, this->clientID, CLIENT_ID_SIZE);
	req.FReqHead.ReqHead.code = code;
	req.FReqHead.ReqHead.version = VERSION;
	char* finalBuffer = NULL;
	

	//her we prepare the payload to send server , payload created depands on request code .
	switch (code)
	{
	case request::code ::REGISTER :{
		char* registerPayloadPacked = this->registertionRequest(req);
		req.FReqHead.ReqHead.payloadSize = request::payloadSize::REGISTER_SIZE;
		finalBuffer = req.packRequest(registerPayloadPacked,req.FReqHead.ReqHead.payloadSize);
		size += req.FReqHead.ReqHead.payloadSize;
		
		break;
	}
	case request::code::PUBLICKEY: {
		char* publikeyPayloadPacked = this->sendPublicKeyRequest(req);
		req.FReqHead.ReqHead.payloadSize = request::payloadSize::PUBLICKEY_SIZE;
		finalBuffer = req.packRequest(publikeyPayloadPacked, req.FReqHead.ReqHead.payloadSize);
		size += req.FReqHead.ReqHead.payloadSize;
		
		break;
	}
	case request::code::SENDFILE:{
		char* sendFilePayloadPacked = this->sendFileRequest(req);
		finalBuffer = req.packRequest(sendFilePayloadPacked, req.FReqHead.ReqHead.payloadSize);
		size += req.FReqHead.ReqHead.payloadSize;//need to add the payload size
		
		break;
	}
	case request::code::FIXCRC: case request::code::ERR4CRC: case request::code::ERRCRC: {
		req.FReqHead.ReqHead.payloadSize = request::FIXCRC_SIZE;
		char* CrcePayloadPacked = this->CRCpayload(req);
		finalBuffer = req.packRequest(CrcePayloadPacked, req.FReqHead.ReqHead.payloadSize);
		size += req.FReqHead.ReqHead.payloadSize;//need to add the payload size
		
		break;
	}
	}
	req.sendPackets(s, finalBuffer,size);//sending the finalbuffer
	delete finalBuffer; // we free the memory alocated at packRequest methode 
}

//upadting me file with client name and id and privet key
void Operation::updateMeFile(std::string clientPrivetKey) {

	try {
		std::ofstream meFile("Me.info", std::ios::out);
		
			meFile << this->clientName+"\n";
			meFile << hexifyID(this->clientID) +'\n';
			meFile<<clientPrivetKey;

		meFile.close();
	}
	catch (const std::exception&)
	{
		std::cout << "Error update me file" << std::endl;
	}
}
//represnt 16 bytes as 32 digit string
std::string Operation::hexifyID(const char* clientID) {
	int i = 0;
	std::stringstream stream;
	std::string result;
	while (i < CLIENT_ID_SIZE) {
		stream << std::setfill('0') << std::setw(2) << std::hex << (0xFF & clientID[i]);
		i++;
	}
	result = (stream.str());
	return result;

}

//used to decrypt AES key from server
void Operation::decryptAES(char* encryptedAES, char* AESfinal) {
		RSAPrivateWrapper rsapriv_other(Base64Wrapper::decode(this->privetKey));
		std::string pubkey = rsapriv_other.getPublicKey();
		char tmp[160];
		memcpy(tmp, pubkey.c_str(), pubkey.size());
		std::string AES = rsapriv_other.decrypt(encryptedAES, AES_ENC_SIZE);
		memcpy(AESfinal, AES.c_str(), AES_SIZE);
	}
Operation::~Operation() {

}



