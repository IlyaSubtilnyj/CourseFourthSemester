#pragma once
#include <assert.h>
#include <WinSock2.h>
#include <string>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")

#pragma warning(disable: 4996);

#define BUFFER_SIZE 1024

namespace Net
{

	class Client
	{
	private:
		WSADATA wsa;
		SOCKET client_socket;
		int port;
		std::string ipaddress;
		char buffer[BUFFER_SIZE];
		struct sockaddr_in info;
		int infolength;
		int recvlength;
		std::string message;
	public:
		Client(int, std::string);
		~Client();
	public:
		void connect();
	private:
		void init();
		void send();
		void receive();
		void process();
	};

}
