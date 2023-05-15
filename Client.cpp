#include "Client.h"

namespace Net
{

	Client::Client(int port, std::string ipaddress)
		:
		port(port),
		ipaddress(ipaddress),
		info{ 0 },
		wsa{ 0 },
		infolength(sizeof(info)),
		client_socket(INVALID_SOCKET) {}

	void Client::init()
	{
		info.sin_family = AF_INET;
		info.sin_port = htons(port);
		info.sin_addr.s_addr = inet_addr(ipaddress.c_str());

		assert(!WSAStartup(MAKEWORD(2, 2), &wsa));

		assert(!((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) == SOCKET_ERROR));

		printf("Client started");
	}

	void Client::connect()
	{
		init();

		for (; ;)
		{
			receive();
			process();
			send();
		}
	}

	void Client::send()
	{
		printf("Enter a message: ");
		std::getline(std::cin, message);
		if (sendto(client_socket, message.c_str(), message.length(), 0, (struct sockaddr*)&info, infolength) == SOCKET_ERROR)
		{
			printf("sendto failed...");
			exit(EXIT_FAILURE);
		}
	}

	void Client::receive()
	{
		if (recvlength = recvfrom(client_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&info, &infolength) == SOCKET_ERROR)
		{
			printf("recvfrom failed...");
			exit(EXIT_FAILURE);
		}
	}

	void Client::process()
	{
		printf("recvform %s:%d", inet_ntoa(info.sin_addr), ntohs(info.sin_port));
		for (size_t i = 0; i < recvlength; i++)
		{
			printf("%c", buffer[i]);
		}
	}

	Client::~Client()
	{
		closesocket(client_socket);
		WSACleanup();
	}
}
