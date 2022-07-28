#include "proto.h"
#include "client.h"

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
#include <utility>

static std::string ReadLine()
{
	using namespace std;
	string s;
    getline(cin, s);
    return s;
}

std::vector<std::string> SplitIntoWords(const std::string& text) {
	using namespace std;
	vector<string> words;
	string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

int main()
{
	std::cout << "Chat client started" << std::endl;

	client::Server server("127.0.0.1", 5000);
	if (!server.Connect()) {
		return -1;
	}

	std::cout << "conneted" << std::endl;

	client::Client client;



	int epollfd = epoll_create1(0);
	if (epollfd == -1) {
		std::cerr << "can't create epoll descriptor" << std::endl;
		return -1;
	}

	struct epoll_event event;
	event.events = EPOLLIN | EPOLLET; 
	event.data.fd = fileno(stdin);

	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fileno(stdin), &event) == -1) {
		std::cerr <<  "can't add event" << std::endl;
	}

	event.events = EPOLLIN | EPOLLET;
	event.data.fd = server.GetId();
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, server.GetId(), &event) == -1) {
		std::cerr <<  "can't add event" << std::endl;
	}

	


	while(true) {

		struct epoll_event revents[80];
		int nfds = epoll_wait(epollfd, revents, 80, -1);

		for (int i = 0; i < nfds; ++i) {
			 
			if (revents[i].data.fd == fileno(stdin)) {
				std::string command;
				std::cin >> command;
				if (command == "signin") {
					proto::SigninRequest request(server.GetId());
					std::cin >> request.name;
					auto response = server.Accept(request);
					client.Accept(*response);
				} else if (command == "login") {
					proto::LoginRequest request(server.GetId());
					std::cin >> request.name;
					auto response = server.Accept(request);
					client.Accept(*response);
				} else if (command == "message") {
					proto::MessageRequest request(server.GetId());
					for (const auto& name: SplitIntoWords(ReadLine())) {
						request.names.push_back(name);		
					}
					request.text = ReadLine();

					std::cout << "Sending request" << std::endl;
					std::cout << "To: ";
					for (const auto& name: request.names) {
						std::cout << name << " ";	
					}
					std::cout << std::endl;
					std::cout << "Text: " << request.text << std::endl;
		
					auto response = server.Accept(request);
					client.Accept(*response);
				}
			} 

			if (revents[i].data.fd == server.GetId()) { 
				std::cout << "Incoming message" << std::endl;		
				auto request = server.Recv();
				// client.ShowMessage(std::move(request));


			}


		}

		 


	}

	return 0;
}





