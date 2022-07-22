#include "proto.h"
#include "client.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>

int main()
{
	std::cout << "Chat client started" << std::endl;

	/*

	struct sockaddr_in server_addr;

	int serverd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverd == -1) {
		std::cerr << "can't create socket" << std::endl;
		return -1;
	}

	memset(&server_addr, '0', sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(5000);
	inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

	if (connect(serverd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
		std::cerr << "can't connect" << std::endl;
	}

	*/

	client::Server server("127.0.0.1", 5000);
	if (!server.Connect()) {
		return -1;
	}

	std::cout << "conneted" << std::endl;

	while(true) {
		std::string command;
		std::cin >> command;
		if (command == "signin") {
			SigninRequest request;
			std::cin >> request.name;
			server.Do(request);

			/*
			char buf[1024];
			int len = 1024;
	
			len = SigninRequest::serialize(request, buf, len); 
			if (len == -1) {
				std::cerr << "signin serialize failed" << std::endl;
				return -1;	
			}
			std::cout << "Send signin request for " << request.name << std::endl;

			send(serverd, buf, len, 0);

			std::cout << "Receive response" << std::endl;
			len = recv(serverd, buf, 256, 0);
		
			ResponsePtr response = ReadResponse(buf, len);
			*/


		} else if (command == "login") {
			LoginRequest request;
			std::cin >> request.name;
			server.Do(request);
			/*
			char buf[1024];
			int len = 1024;

			len = LoginRequest::serialize(request, buf, len); 
			if (len == -1) {
				std::cerr << "login serialize failed" << std::endl;
				return -1;	
			}
			std::cout << "Send login request for " << request.name << std::endl;
			send(serverd, buf, len, 0);
			*/
		}

	}

	return 0;
}





