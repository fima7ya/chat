#pragma once

#include "proto.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <system_error>
#include <cstring>
#include <cassert>
#include <iostream>

namespace client 
{
	class Server : public proto::Target
	{
	public:
		explicit Server(const std::string host, uint16_t port)
			: host_(host)
			, port_(port) 
			, sd_(SocketOpen())
		{}
	public:
		int GetId() const
		{
			return sd_;
		}
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

		proto::RequestPtr Recv()
		{
			char buf[1024];
			int len = 1024;
			len = recv(sd_, buf, 256, 0);


			return proto::ReadRequest(0, buf, len);
		}

		proto::ResponsePtr Process(proto::SigninRequest& request) override
		{
			char buf[1024];
			int len = 1024;

			len = proto::SigninRequest::serialize(request, buf, len); 
			if (len == -1) {
				std::cerr << "signin serialize failed" << std::endl;
				return nullptr;	
			}
			std::cout << "Send signin request for " << request.name << std::endl;

			send(sd_, buf, len, 0);

			std::cout << "Receive response" << std::endl;
			len = recv(sd_, buf, 256, 0);
		
			std::cout << "Signin response received" << std::endl;

			return proto::ReadResponse(request.GetId(), buf, len);
		}

		proto::ResponsePtr Process(proto::LoginRequest& request) override
		{
			char buf[1024];
			int len = 1024;

			len = proto::LoginRequest::serialize(request, buf, len); 
			if (len == -1) {
				std::cerr << "login serialize failed" << std::endl;
				return nullptr;	
			}
			std::cout << "Send login request for " << request.name << std::endl;
			send(sd_, buf, len, 0);

			std::cout << "Receive response" << std::endl;
			len = recv(sd_, buf, 256, 0);

			std::cout << "Login response received" << std::endl;

			return proto::ReadResponse(request.GetId(), buf, len);
		}

		proto::ResponsePtr Process(proto::MessageRequest& request) override 
		{
			char buf[1024];
			int len = 1024;

			len = request.serialize(buf, len); 
			if (len == -1) {
				std::cerr << "login serialize failed" << std::endl;
				return nullptr;	
			}
			std::cout << "Send message request" << std::endl;
			send(sd_, buf, len, 0);

			std::cout << "Receive response" << std::endl;
			len = recv(sd_, buf, 256, 0);

			std::cout << "Message response received" << std::endl;

			return proto::ReadResponse(request.GetId(), buf, len);
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

	class Client : public proto::Origin
	{
	public:
		void Process(proto::SigninSuccess& response) override
		{
			std::cout << "SignedIn success" << std::endl;
		}

		void Process(proto::SigninFailure& response) override
		{
			std::cout << "SignedIn failure" << std::endl;
		}
		void Process(proto::LoginSuccess& response) override
		{
			std::cout << "Login success" << std::endl;
		}
		void Process(proto::LoginFailure& response) override
		{
			std::cout << "Login failed" << std::endl;
		}
		void Process(proto::MessageSuccess& request) override
		{
			std::cout << "Message success" << std::endl;
		}
	public:
		/*
		proto::ResponsePtr Process(proto::SigninRequest& request) override
		{
			assert(false);
			return std::make_unique<proto::SigninSuccess>(0);
		}

		proto::ResponsePtr Process(proto::LoginRequest& request) override
		{
			assert(false);
			return std::make_unique<proto::LoginSuccess>(0);
		}
		*/
		

		proto::ResponsePtr ShowMessage(proto::MessageRequestPtr request)
		{
			std::cout << "Message rceived" << std::endl;
			for (const auto& name: request->names) {
				std::cout << "From: " << name << std::endl;
			}
			std::cout << "Text: " << request->text << std::endl;
			return std::make_unique<proto::MessageSuccess>(0);
		}
	};


}


