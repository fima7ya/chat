#pragma once

#include "proto.h"

#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>

class Server
{
public:
	ResponsePtr Process(Request& request)
	{
		return request.Process(*this);
	}
public:
	ResponsePtr Process(SigninRequest& request)
	{
		std::cout << "SigninRequest" << std::endl;
		return std::make_unique<SigninSuccess>();
	}
	ResponsePtr Process(LoginRequest& request)
	{
		std::cout << "LoginRequest" << std::endl;
		return std::make_unique<LoginSuccess>();
	}
	ResponsePtr Process(MessageRequest& request)
	{
		std::cout << "MessagenRequest" << std::endl;
		return std::make_unique<MessageSuccess>();
	}
private:
	std::unordered_map<std::string, int> clients_;	


};



