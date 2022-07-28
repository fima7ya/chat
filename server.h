#pragma once

#include "proto.h"

#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>
#include <set>




namespace srv
{

class UsersRepository
{
public:
	bool Add(const std::string& name) 
	{
		if (users_.count(name) == 0) {
			users_.insert(name);
			return true;
		}
		return false;
	}

	bool Check(const std::string& name) const
	{
		return users_.count(name);
	}
private:
	std::set<std::string> users_;
};

/*
class Server : public proto::Target
{
public:
	proto::ResponsePtr Process(proto::SigninRequest& request) override;
	proto::ResponsePtr Process(proto::LoginRequest& request) override;
	proto::ResponsePtr Process(proto::MessageRequest& request) override;
private:

	std::unordered_map<std::string, int> online_users_;	
	UsersRepository users_;

};

class Client : public proto::Origin
{
public:
public:
	void Process(proto::SigninSuccess& response) override;
	void Process(proto::SigninFailure& response) override;
	void Process(proto::LoginSuccess& response) override;
	void Process(proto::LoginFailure& response) override;
	void Process(proto::MessageSuccess& response) override;
private:
	int sd_;
};
*/




class Server;

class Client : public proto::Target
{
public:	
	Client(Server& server, int sd);
public:
	int GetId() const
	{
		return sd_;
	}
public:
	proto::ResponsePtr Process(proto::SigninRequest& request) override;
	proto::ResponsePtr Process(proto::LoginRequest& request) override;
	proto::ResponsePtr Process(proto::MessageRequest& request) override;
public:
	proto::RequestPtr Recv();
	proto::ResponsePtr RecvResponse();
	void Send(proto::Response& response);
	void Send(proto::Request& request);
	
private:
	Server& server_;
	int sd_;
	std::string name_;
};
using ClientPtr = std::shared_ptr<Client>;

class Server
{
public:
	void AddClient(ClientPtr client);
	ClientPtr GetClient(int sd);
	bool HasUser(const std::string& name) const;
	void AddUser(const std::string& name);

	bool IsOnline(const std::string& name) const;
	void Login(const std::string& name, ClientPtr client);

	ClientPtr GetClient(const std::string& name);
public:
	void PrintStatus() const;

private:
	std::unordered_map<int, srv::ClientPtr> socket_to_client_;
	std::unordered_map<std::string, srv::ClientPtr> name_to_client_;
};


}




