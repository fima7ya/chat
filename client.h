#pragma once

#include "proto.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <system_error>
#include <cstring>
#include <iostream>

namespace client 
{
	class Server
	{
	public:
		explicit Server(const std::string host, uint16_t port)
			: host_(host)
			, port_(port) 
			, sd_(SocketOpen())
		{}
	public:
		bool Connect()
		{
			struct sockaddr_in server_addr;
			memset(&server_addr, '0', sizeof(server_addr));

			server_addr.sin_family = AF_INET;
			server_addr.sin_port = htons(port_);
			inet_pton(AF_INET, host_.c_str(), &server_addr.sin_addr);			

			if (connect(sd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
				std::cerr << "can't connect" << std::endl;
				return false;
			}
			return true;
		}

		ResponsePtr Do(const SigninRequest& request)
		{
			char buf[1024];
			int len = 1024;

			len = SigninRequest::serialize(request, buf, len); 
			if (len == -1) {
				std::cerr << "signin serialize failed" << std::endl;
				return nullptr;	
			}
			std::cout << "Send signin request for " << request.name << std::endl;

			send(sd_, buf, len, 0);

			std::cout << "Receive response" << std::endl;
			len = recv(sd_, buf, 256, 0);
		
			std::cout << "Signin response received" << std::endl;

			return ReadResponse(buf, len);
		}

		ResponsePtr Do(const LoginRequest& request)
		{
			char buf[1024];
			int len = 1024;

			len = LoginRequest::serialize(request, buf, len); 
			if (len == -1) {
				std::cerr << "login serialize failed" << std::endl;
				return nullptr;	
			}
			std::cout << "Send login request for " << request.name << std::endl;
			send(sd_, buf, len, 0);

			std::cout << "Receive response" << std::endl;
			len = recv(sd_, buf, 256, 0);

			std::cout << "Login response received" << std::endl;

			return ReadResponse(buf, len);
		}

	private:
		static int SocketOpen() 
		{
			int sd = socket(AF_INET, SOCK_STREAM, 0);
			if (sd == -1) {
				throw std::system_error(errno, std::generic_category(), "can't opean socket");
			}
			return sd;
		}

	private:
		std::string host_;
		uint16_t port_;
		int sd_;
	};
}


