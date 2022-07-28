#include "proto.h"
#include "server.h"

#include <cassert>
#include <iostream>
#include <memory>

namespace proto
{

static MessageRequestPtr ReadMessage(const char* buf, size_t len)
{
	MessageRequestPtr message = std::make_unique<MessageRequest>(5);
	std::string word;
	for (size_t i = 0; i < len; ++i) {
		char c = buf[i];
		if (c =='\0') {
			if (!word.empty()) {
				if (i + 1 < len) {
					message->names.push_back(word);
				} else {
					message->text = word;
				}
				word.clear();
			}
		} else {
			word += c;
		}
	}
	/*
	if (!word.empty()) {
		message->text = word;
	}
	*/

	return message;
}

/*
int WriteMessage(const MessageRequest& message, char* buf, size_t maxlen)
{
	size_t pos = 0;
	for (const auto& name: message.names) {
		for (const char c: name) {
			buf[pos++] = c;
		}
		buf[pos++] = '\0';
	}

	for (const char c: message.text) {
		buf[pos++] = c;
	}
	buf[pos] = '\0';

	return pos;
}
*/

RequestPtr ReadRequest(int id, const char* buf, size_t nbytes)
{
	if (nbytes < sizeof(Header)) {
		throw std::domain_error(std::to_string(nbytes) + " less than header " + std::to_string(sizeof(Header)));
	}

	Header* header = (Header*)buf;

	switch(static_cast<RequestType>(header->type)) {
		case RequestType::Singin: {
			return std::make_unique<SigninRequest>(id, std::string(buf + sizeof(Header), header->len));
		} 
		case RequestType::Login: {
			return std::make_unique<LoginRequest>(id, std::string(buf + sizeof(Header), header->len));
		}
		case RequestType::Message: {
			std::cout << "ReadRequest message" << std::endl;
			return ReadMessage(buf + sizeof(Header), header->len);
		} 
		
		default:
			assert(false);
	}
	return nullptr;
}


ResponsePtr ReadResponse(int id, const char* buf, size_t nbytes)
{
	if (nbytes < sizeof(Header)) {
		throw std::domain_error(std::to_string(nbytes) + " less than header " + std::to_string(sizeof(Header)));
	}

	Header* header = (Header*)buf;

	switch(static_cast<RequestType>(header->type)) {
		case RequestType::Singin: {
			if (static_cast<SigninResponse::Result>(buf[sizeof(Header)]) != SigninResponse::Result::Success) {
				return std::make_unique<SigninFailure>(id); 
			} 
			return std::make_unique<SigninSuccess>(id);
		} 
		case RequestType::Login: {
			if (static_cast<LoginResponse::Result>(buf[sizeof(Header)]) != LoginResponse::Result::Success) {
				return std::make_unique<LoginFailure>(id); 
			} 
			return std::make_unique<LoginSuccess>(id);
		}
	 	/*	
		case RequestType::Message: {
			MessageRequest message = ReadMessage(buf + sizeof(Header), header->len);
			// return std::move(std::make_unique<MessageRequest>(message));
			break;
		} 
		*/
		default:
			assert(false);
	}
	return nullptr;
}



void DumpBuffer(const char* buf, size_t len)
{
	Header* header = (Header*)buf;
	std::cout 	<< "Type: " << static_cast<int>(header->type) << std::endl
				<< "Length: " << static_cast<size_t>(header->len) << std::endl;
	std::cout << "Data[";
	for (size_t i = sizeof(Header); i < len; ++i) {
		std::cout << buf[i];	
	}
	std::cout << "]" << std::endl;

}

ResponsePtr SigninRequest::Process(Target& server) 
{
	return server.Process(*this);
}

ResponsePtr LoginRequest::Process(Target& server) 
{
	return server.Process(*this);
}

ResponsePtr MessageRequest::Process(Target& server) 
{
	return server.Process(*this);
}


void SigninSuccess::Process(Origin& origin) 
{
	origin.Process(*this);
}

void SigninFailure::Process(Origin& origin) 
{
	origin.Process(*this);
}

void LoginSuccess::Process(Origin& origin) 
{
	origin.Process(*this);
}

void LoginFailure::Process(Origin& origin) 
{
	origin.Process(*this);
}

void MessageSuccess::Process(Origin& origin) 
{
	origin.Process(*this);
}


}

