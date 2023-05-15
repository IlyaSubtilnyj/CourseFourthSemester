#include "Server.h"

namespace Net
{

	Server::Server(int port, std::string ipaddress)
		:
		port(port),
		ipaddress(ipaddress),
		info{0},
		wsa{0},
		infolength(sizeof(info)),
		server_socket(INVALID_SOCKET) {}

	void Server::init()
	{
		info.sin_family = AF_INET;
		info.sin_port = htons(port);
		info.sin_addr.s_addr = inet_addr(ipaddress.c_str());

		assert(!WSAStartup(MAKEWORD(2, 2), &wsa));
		 
		assert(!((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == SOCKET_ERROR));

		assert(!bind(server_socket, (struct sockaddr*)&info, infolength));

		printf("Server started at %s:%d", inet_ntoa(info.sin_addr), ntohs(info.sin_port));
	}

	void Server::start()
	{
		init();

		for (; ;)
		{
			receive();
			process();
			send();
		}
	}

	void Server::receive()
	{
		if (recvlength = recvfrom(server_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&info, &infolength) == SOCKET_ERROR)
		{
			printf("recvfrom failed...");
			exit(EXIT_FAILURE);
		}
	}

	void Server::process()
	{
		printf("recvform %s:%d", inet_ntoa(info.sin_addr), ntohs(info.sin_port));
		for (size_t i = 0; i < recvlength; i++)
		{
			printf("%c", buffer[i]);
		}
	}

	void Server::send()
	{
		if (sendto(server_socket, buffer, recvlength, 0, (struct sockaddr*)&info, infolength) == SOCKET_ERROR)
		{
			printf("sendto failed...");
			exit(EXIT_FAILURE);
		}
	}

	Server::~Server()
	{
		closesocket(server_socket);
		WSACleanup();
	}
}