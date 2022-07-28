#pragma once

#include <memory>
#include <vector>
#include <iostream>


namespace proto 
{

class Target;
class Origin;

#pragma pack(push, 1)
struct Header
{
	uint8_t type;
	uint16_t len;
};
#pragma pack(pop)

enum class RequestType
{
	Singin,
	Login,
	Message
};


class Response
{
public:
	explicit Response(int id)
		: id_(id)
	{}
public:
	virtual void Process(Origin& origin) = 0;
	virtual int serialize(char* buf, size_t maxlen) = 0;
public:
	int GetId() const 
	{
		return id_;
	}
private:
	int id_;
};

using ResponsePtr = std::unique_ptr<Response>;
		
	
class SigninResponse : public Response
{
public:
	explicit SigninResponse(int id)
		: Response(id)
	{}
public:
	enum class Result
	{
		Success,
		Failure
	};
public:
	int serialize(char* buf, size_t maxlen) override
	{
		Header* header = (Header*)buf;
		header->type = static_cast<uint8_t>(RequestType::Singin);
		return sizeof(Header);
	}
};

class SigninSuccess : public SigninResponse
{
public:
	explicit SigninSuccess(int id)
		: SigninResponse(id)
	{}
public:
	virtual void Process(Origin& origin) override; 

	int serialize(char* buf, size_t maxlen) override
	{
		int len = SigninResponse::serialize(buf, maxlen);
		buf[len++] = static_cast<uint8_t>(Result::Success);
		return len;
	}
};

class SigninFailure: public SigninResponse
{
public:
	explicit SigninFailure(int id)
		: SigninResponse(id)
	{}
public:
	virtual void Process(Origin& origin) override; 

	int serialize(char* buf, size_t maxlen) override
	{
		int len = SigninResponse::serialize(buf, maxlen);
		buf[len++] = static_cast<uint8_t>(Result::Failure);
		return len;
	}
};

class LoginResponse : public Response
{
public:
	explicit LoginResponse(int id)
		: Response(id)
	{}
public:
	enum class Result
	{
		Success,
		Failure
	};
public:
	int serialize(char* buf, size_t maxlen) override
	{
		Header* header = (Header*)buf;
		header->type = static_cast<uint8_t>(RequestType::Login);
		return sizeof(Header);
	}
};

class LoginSuccess : public LoginResponse
{
public:
	explicit LoginSuccess(int id)
		: LoginResponse(id)
	{}
public:
	virtual void Process(Origin& origin) override; 
	int serialize(char* buf, size_t maxlen) override
	{
		int len = LoginResponse::serialize(buf, maxlen);
		buf[len++] = static_cast<uint8_t>(Result::Success);
		return len;
	}
};

class LoginFailure: public LoginResponse
{
public:
	explicit LoginFailure(int id)
		: LoginResponse(id)
	{}
public:
	virtual void Process(Origin& origin) override; 
	int serialize(char* buf, size_t maxlen) override
	{
		int len = LoginResponse::serialize(buf, maxlen);
		buf[len++] = static_cast<uint8_t>(Result::Failure);
		return len;
	}
};

class MessageResponse : public Response
{
public:
	explicit MessageResponse(int id)
		: Response(id)
	{}
public:
	int serialize(char* buf, size_t maxlen) override
	{
		return 0;
	}
};

class MessageSuccess : public MessageResponse
{
public:
	explicit MessageSuccess(int id)
		: MessageResponse(id)
	{}
public:
	virtual void Process(Origin& origin) override; 

	int serialize(char* buf, size_t maxlen) override
	{
		return 0;
	}
};

class MessageFailure: public MessageResponse
{
public:
	explicit MessageFailure(int id)
		: MessageResponse(id)
	{}
public:
	virtual void Process(Origin& origin) override; 
	int serialize(char* buf, size_t maxlen) override
	{
		return 0;
	}
};


class Request
{
public:
	explicit Request(int id)
		: id_(id)
	{}
public:
	virtual int serialize(char* buf, size_t maxlen) = 0;
public:
	virtual ResponsePtr Process(Target& target) = 0;
public:
	int GetId() const 
	{
		return id_;
	}
private:
	int id_;
};

using RequestPtr = std::unique_ptr<Request>;

class SigninRequest: public Request
{
public:
	explicit SigninRequest(int id)
		: Request(id)
	{}
		
	SigninRequest(int id, const std::string& str) 
		: Request(id)
		, name(str)
	{}
public:
	ResponsePtr Process(Target& target) override;

public:
	std::string name;

	int serialize(char* buf, size_t maxlen) override
	{
		Header* header = (Header*)buf;		
		header->type = static_cast<uint8_t>(RequestType::Singin);
		header->len = name.size(); 

		int pos = sizeof(Header); 
		for (const char c: name) {
			buf[pos++] = c;	
		}
		buf[pos++] = '\0';

		return pos;
	}
	static int serialize(const SigninRequest& request, char* buf, size_t maxlen) {
		Header* header = (Header*)buf;		
		header->type = static_cast<uint8_t>(RequestType::Singin);
		header->len = request.name.size(); 

		int pos = sizeof(Header); 
		for (const char c: request.name) {
			buf[pos++] = c;	
		}
		buf[pos++] = '\0';

		return pos;
	}

};

using SigninRequestPtr = std::unique_ptr<SigninRequest>;

class LoginRequest: public Request
{
public:
	explicit LoginRequest(int id)
		: Request(id)
	{}
	LoginRequest(int id, const std::string& str) 
		: Request(id)
		, name(str)
	{}
public:
	ResponsePtr Process(Target& target) override;
public:
	std::string name;
public:
	int serialize(char* buf, size_t maxlen) override 
	{
		Header* header = (Header*)buf;		
		header->type = static_cast<uint8_t>(RequestType::Login);
		header->len = name.size(); 

		int pos = sizeof(Header); 
		for (const char c: name) {
			buf[pos++] = c;	
		}
		buf[pos++] = '\0';

		return pos;
	}

	static int serialize(const LoginRequest& request, char* buf, size_t maxlen) {
		Header* header = (Header*)buf;		
		header->type = static_cast<uint8_t>(RequestType::Login);
		header->len = request.name.size(); 

		int pos = sizeof(Header); 
		for (const char c: request.name) {
			buf[pos++] = c;	
		}
		buf[pos++] = '\0';

		return pos;
	}
};

using LoginRequestPtr = std::unique_ptr<LoginRequest>;

class MessageRequest: public Request
{
public:
	explicit MessageRequest(int id)
		: Request(id)
	{}
public:
	ResponsePtr Process(Target& target) override;
public:
	std::vector<std::string> names;
	std::string text;

public:
	int serialize(char* buf, size_t maxlen) override 
	{
		Header* header = (Header*)buf;		
		header->type = static_cast<uint8_t>(RequestType::Message);

		int pos = sizeof(Header); 

		for (const auto& name: names) {
			for (const char c: name) {
				buf[pos++] = c;	
			}
			buf[pos++] = '\0';		
		}

		for (const char c: text) {
			buf[pos++] = c;	
		}
		buf[pos++] = '\0';

		header->len = pos - sizeof(Header); 

		return pos;
	}

};
using MessageRequestPtr = std::unique_ptr<MessageRequest>;

/*
MessageRequest ReadMessage(const char* buf, size_t len);
int WriteMessage(const MessageRequest& message, char* buf, size_t maxlen);
*/


RequestPtr ReadRequest(int id, const char* buf, size_t nbytes);
ResponsePtr ReadResponse(int id, const char* buf, size_t nbytes);


void DumpBuffer(const char* buf, size_t len);


class Target 
{
public:
	ResponsePtr Accept(Request& request)
	{
		return request.Process(*this);
	};
	
public:
	virtual ResponsePtr Process(SigninRequest& request) = 0;
	virtual ResponsePtr Process(LoginRequest& request) = 0;
	virtual ResponsePtr Process(MessageRequest& request) = 0;
};


class Origin 
{
public:
	void Accept(Response& response)
	{
		return response.Process(*this);
	};
	
public:
	virtual void Process(SigninSuccess& response) = 0;
	virtual void Process(SigninFailure& response) = 0;
	virtual void Process(LoginSuccess& response) = 0;
	virtual void Process(LoginFailure& response) = 0;
	virtual void Process(MessageSuccess& response) = 0;
	// virtual void Process(MessageFailed& request) = 0;
};

}

