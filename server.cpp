#include "proto.h"
#include "server.h"

#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/epoll.h>

#include <cstring>
#include <iostream>
#include <unordered_map>
#include <stdexcept>
#include <memory>
#include <cassert>
#include <vector>
#include <algorithm>

namespace srv 
{

Client::Client(Server& server, int sd)
	: server_(server)
	, sd_(sd)
{}


proto::RequestPtr Client::Recv()
{
	char buf[256];
	size_t nbytes = recv(sd_, buf, 256, 0);
	proto::DumpBuffer(buf, nbytes);
	return proto::ReadRequest(sd_, buf, nbytes);
}


proto::ResponsePtr Client::RecvResponse() {
	char buf[256];
	size_t nbytes = recv(sd_, buf, 256, 0);
	// proto::DumpBuffer(buf, nbytes);
	return proto::ReadResponse(sd_, buf, nbytes);
}


void Client::Send(proto::Response& response)
{
	char buf[1024];
	int len = 1024;

	len = response.serialize(buf, len); 
	if (len == -1) {
		std::cerr << "serialize failed" << std::endl;
		return;
	}
	std::cout << "Send response " << std::endl;
	send(sd_, buf, len, 0);
}

void Client::Send(proto::Request& request)
{
	char buf[1024];
	int len = 1024;

	len = request.serialize(buf, len); 
	if (len == -1) {
		std::cerr << "serialize failed" << std::endl;
		return;
	}
	std::cout << "Send request" << std::endl;
	send(sd_, buf, len, 0);
}

void Server::AddClient(ClientPtr client)
{
	if (socket_to_client_.count(client->GetId()) == 1) {
		return;
	}
	socket_to_client_[client->GetId()] = client;
}

ClientPtr Server::GetClient(int sd)
{
	return socket_to_client_.at(sd);
}

bool Server::HasUser(const std::string& name) const
{
	auto found = name_to_client_.find(name);
	if (found == name_to_client_.end()) {
		return false;
	}
	return true;
}


void Server::AddUser(const std::string& name)
{
	name_to_client_[name] = nullptr;
}


bool Server::IsOnline(const std::string& name) const
{
	return name_to_client_.at(name) != nullptr;
}

ClientPtr Server::GetClient(const std::string& name)
{
	return name_to_client_.at(name);
}


void Server::Login(const std::string& name, ClientPtr client)
{
	name_to_client_.at(name) = client;
}



void Server::PrintStatus() const
{
	for (const auto& [name, client]: name_to_client_) {
		std::cout << name << ": " << ((client != nullptr) ? "online" : "offline") << std::endl;
	}
}


proto::ResponsePtr Client::Process(proto::SigninRequest& request) 
{
	std::cout << "SigninRequest" << std::endl;
	if (server_.HasUser(request.name)) {
		return std::make_unique<proto::SigninFailure>(request.GetId());
	}

	server_.AddUser(request.name);

	return std::make_unique<proto::SigninSuccess>(request.GetId());
}

proto::ResponsePtr Client::Process(proto::LoginRequest& request) 
{
	std::cout << "LoginRequest" << std::endl;
	if (server_.HasUser(request.name) && !server_.IsOnline(request.name)) {
		auto client = server_.GetClient(sd_);
		server_.Login(request.name, client);
		name_ = request.name;
		return std::make_unique<proto::LoginSuccess>(request.GetId());

	}

	return std::make_unique<proto::LoginFailure>(request.GetId());

}

proto::ResponsePtr Client::Process(proto::MessageRequest& request) 
{
	std::cout << "MessagenRequest" << std::endl;

	std::cout << "To: ";
	for (const auto& name: request.names) {
		std::cout << "[" << name << "]";
	}
	std::cout << std::endl;

	std::cout << "Text: " << request.text << std::endl;

	proto::MessageRequest msg(5);
	msg.names.push_back(name_);
	msg.text = request.text;

	for (const auto& name: request.names) {
		if (server_.IsOnline(name)) {
			auto client = server_.GetClient(name);
			client->Send(msg);
		}
	}

	/*
	bool success = false;

	for (const auto& name: request.names) {
		if (server_.IsOnline(name)) {
			auto client = server_.GetClient(name);
			auto response = client->RecvResponse();
		}
	}
	*/

	return std::make_unique<proto::MessageSuccess>(request.GetId());
}


/*
void Client::Process(proto::SigninSuccess& response)
{
	char buf[1024];
	int len = 1024;

	len = response.serialize(buf, len); 
	if (len == -1) {
		std::cerr << "login serialize failed" << std::endl;
		return;
	}
	std::cout << "Send SigninSuccess " << std::endl;
	send(sd_, buf, len, 0);
}

void Client::Process(proto::SigninFailure& response)
{
	char buf[1024];
	int len = 1024;

	len = response.serialize(buf, len); 
	if (len == -1) {
		std::cerr << "login serialize failed" << std::endl;
		return;
	}
	std::cout << "Send SigninFailure " << std::endl;
	send(sd_, buf, len, 0);
}

void Client::Process(proto::LoginSuccess& response)
{
	char buf[1024];
	int len = 1024;

	len = response.serialize(buf, len); 
	if (len == -1) {
		std::cerr << "login serialize failed" << std::endl;
		return;
	}
	std::cout << "Send LoginSuccess " << std::endl;
	send(sd_, buf, len, 0);
}

void Client::Process(proto::LoginFailure& response)
{
	char buf[1024];
	int len = 1024;

	len = response.serialize(buf, len); 
	if (len == -1) {
		std::cerr << "login serialize failed" << std::endl;
		return;
	}
	std::cout << "Send LoginFailure" << std::endl;
	send(sd_, buf, len, 0);
}

void Client::Process(proto::MessageSuccess& response)
{
}
*/



}

int main()
{
	// std::unordered_map<SocketDescriptor, HandlerPtr> handlers; 

	std::cout << "Chat server started" << std::endl;


	int listenerd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenerd == -1) {
		std::cerr << "can't create socket" << std::endl;
		return -1;
	}

	const int enable = 1;
	if (setsockopt(listenerd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		std::cerr << "SO_REUSEADDR failed" << std::endl;
		return -1;
	}

	struct sockaddr_in listener_addr;
	memset(&listener_addr, '0', sizeof(listener_addr));

	listener_addr.sin_family = AF_INET;
	listener_addr.sin_addr.s_addr = htonl(INADDR_ANY);	
	listener_addr.sin_port = htons(5000);

	if (bind(listenerd, (struct sockaddr*)&listener_addr, sizeof(listener_addr)) == -1) {
		std::cerr << "can't bind socket" << std::endl;
		return -1;
	}

	constexpr int BACKLOG = 5; // specifies the queue length for completely established sockets waiting to be accepted
	if (listen(listenerd, BACKLOG) == -1) {
		std::cerr << "can't use as listener" << std::endl;
	}


	int epollfd = epoll_create1(0);
	if (epollfd == -1) {
		std::cerr << "can't create epoll descriptor" << std::endl;
		return -1;
	}

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = listenerd;

	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenerd, &event) == -1) {
		std::cerr <<  "can't add event" << std::endl;
	}

	srv::Server server;

	while(true) {
		struct epoll_event revents[80];
		int nfds = epoll_wait(epollfd, revents, 80, -1);
		for (int i = 0; i < nfds; ++i) {
			if (revents[i].data.fd == listenerd) { // new connection
				struct sockaddr_in client_addr;
				socklen_t client_addr_size = sizeof(client_addr); 
				int clientd = accept(listenerd, (struct sockaddr*)&client_addr, &client_addr_size);
				if (clientd == -1) {
					std::cerr << "can't accept" << std::endl;
				}

				auto client = std::make_shared<srv::Client>(server, clientd);
				server.AddClient(client);

				event.events = EPOLLIN | EPOLLET;
				event.data.fd = clientd;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, clientd, &event) == -1) {
					std::cerr <<  "can't add event" << std::endl;
				}

				std::cout << "Accepted" << std::endl;
			} else { // incoming request
				std::cout << "Incoming request" << std::endl;
				// auto& client = clients.at(revents[i].data.fd);
				auto client = server.GetClient(revents[i].data.fd);
				auto request = client->Recv();
				auto response = client->Accept(*request);
				client->Send(*response);
				server.PrintStatus();


			}
		}
		



	}

	return 0;
}




