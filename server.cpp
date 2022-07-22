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

	Server server;
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

				event.events = EPOLLIN | EPOLLET;
				event.data.fd = clientd;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, clientd, &event) == -1) {
					std::cerr <<  "can't add event" << std::endl;
				}

				std::cout << "Accepted" << std::endl;
			} else { // incoming request
				std::cout << "Incoming request" << std::endl;
				char buf[256];
				size_t nbytes = recv(revents[i].data.fd, buf, 256, 0);
				DumpBuffer(buf, nbytes);
				RequestPtr request = ReadRequest(buf, nbytes);
				assert(request != nullptr);
				std::cout << "Start to process" << std::endl;
				ResponsePtr response = server.Process(*request);
				nbytes = response->serialize(buf, 256);
				send(revents[i].data.fd, buf, nbytes, 0);

			}
		}
		



	}

	return 0;
}




