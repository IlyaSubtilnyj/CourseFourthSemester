#pragma once
#include <string>
#include <assert.h>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

#pragma warning(disable: 4996);

#define BUFFER_SIZE 1024

namespace Net
{

	class Server
	{
	private:
		WSADATA wsa;
		SOCKET server_socket;
		int port;
		std::string ipaddress;
		char buffer[BUFFER_SIZE];
		struct sockaddr_in info;
		int infolength;
		int recvlength;
	public:
		Server(int, std::string);
		~Server();
	public:
		void start();
		void end();
	private:
		void init();
		void receive();
		void process();
		void send();
	};

}


