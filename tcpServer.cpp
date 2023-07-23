
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")

// STL includes
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>
#include <map>

// Windows includes
#include <WinSock2.h>

// Library includes
#include <yaml-cpp/yaml.h>


/* getResponse()
* Returns the server response as a string
*/
std::string getResponse()
{
	std::string serverMessage = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";
	std::stringstream response{ "<html>" };
	
	YAML::Node config = YAML::LoadFile("config.yaml");

	if (config["title"])
	{
		response << "<head><title>" << config["title"] << "</title></head>";
	}
	response << "<body>";
	if (config["contents"])
	{
		for (std::size_t i = 0; i < config["contents"].size(); i++)
		{
			YAML::Node item = config["contents"][i];
			if (item.IsMap())
			{
				response << "<" << item["type"] << " style=\"" << item["style"] << "\">" << item["content"] << "</" << item["type"] << ">";
			}
		}
	}

	serverMessage.append(std::to_string(response.str().size()));
	serverMessage.append("\n\n");
	serverMessage.append(response.str());
	return serverMessage;
}

int main()
{
	// Create config file if it does not already exist
	if (!std::filesystem::exists("config.yaml"))
	{
		// Create the file
		std::ofstream config("config.yaml");
		config << "---\ntitle: # Scalar\ncontents: # Sequence of Maps\nstyles: # Sequence of Sequences\n...";
		config.close();

		// Log location of config file
		std::filesystem::path file{ "config.yaml" };
		std::filesystem::path absPath{ std::filesystem::absolute(file) };
		std::cout << "Created config file at " << absPath.string() << "\n";

		// Exit
		std::cout << "Populate the config file and restart the server.\n";
		return EXIT_SUCCESS;
	}

	// Load the config file
	YAML::Node config = YAML::LoadFile("config.yaml");
	if (config.Type() != YAML::NodeType::Map)
	{
		std::cerr << "Improper configuration file structure.\n";
		return EXIT_SUCCESS;
	}

	// Start webserver
	std::cout << "Attempting to create a server\n";

	const std::string IP_ADDR{ "192.168.1.9" };
	const u_short PORT{ 6969 }; // Nice

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
		std::string response{ getResponse() };

		int bytesSent = 0;
		int totalBytesSent = 0;
		while (totalBytesSent < response.size())
		{
			bytesSent = send(new_wsocket, response.c_str(), response.size(), 0);
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