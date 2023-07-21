
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <WinSock2.h>
#include <string>

int main()
{
	std::cout << "Attempting to create a server\n";

	const std::string IP_ADDR{ "192.168.1.9" };
	const u_short PORT{ 6969 };

	SOCKET wsocket;
	SOCKET new_wsocket;

	WSADATA wsaData;
	struct sockaddr_in server;
	int server_len;
	int BUFFER_SIZE = 30720;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cerr << "Could not initialize\n";
	}

	// Create a socket
	wsocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (wsocket == INVALID_SOCKET)
	{
		std::cerr << "Could not create socket\n";
	}

	// Bind socket to address
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(IP_ADDR.c_str());
	server.sin_port = htons(PORT);
	server_len = sizeof(server);

	if (bind(wsocket, (SOCKADDR*)&server, server_len) != 0)
	{
		std::cerr << "Could not bind socket\n" << WSAGetLastError() << "\n";
	}

	// Listen to address
	if (listen(wsocket, 20) != 0)
	{
		std::cerr << "Could not start listening\n";
	}

	std::cout << "Listening on " << IP_ADDR << ":" << PORT << "\n";

	int bytes = 0;
	while (true)
	{
		// Accept client request
		new_wsocket = accept(wsocket, (SOCKADDR*)&server, &server_len);
		if (new_wsocket == INVALID_SOCKET)
		{
			std::cerr << "Could not accept\n";
		}

		// Read request data
		char buff[30720] = { 0 };
		bytes = recv(new_wsocket, buff, BUFFER_SIZE, 0);
		if (bytes < 0)
		{
			std::cerr << "could not read client request\n";
		}

		// Send response
		std::string serverMessage = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";
		std::string response = "<html><h1>Hello World</h1></html>";
		serverMessage.append(std::to_string(response.size()));
		serverMessage.append("\n\n");
		serverMessage.append(response);

		int bytesSent = 0;
		int totalBytesSent = 0;
		while (totalBytesSent < serverMessage.size())
		{
			bytesSent = send(new_wsocket, serverMessage.c_str(), serverMessage.size(), 0);
			if (bytesSent < 0)
			{
				std::cerr << "Could not send response\n";
			}
			totalBytesSent += bytesSent;
		}

		std::cout << "Sent response to client\n";

		// Close socket
		closesocket(new_wsocket);
	}

	closesocket(wsocket);
	WSACleanup();
	return EXIT_SUCCESS;
}