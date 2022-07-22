#pragma once

#include <memory>
#include <vector>

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

class Server;

struct Response
{
	virtual int serialize(char* buf, size_t maxlen) = 0;
};

using ResponsePtr = std::unique_ptr<Response>;
		
	
class SigninResponse : public Response
{
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
	int serialize(char* buf, size_t maxlen) override
	{
		int len = SigninResponse::serialize(buf, maxlen);
		buf[len++] = static_cast<uint8_t>(Result::Success);
		return len;
	}
};

class SigninFailure: public SigninResponse
{
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
	int serialize(char* buf, size_t maxlen) override
	{
		int len = LoginResponse::serialize(buf, maxlen);
		buf[len++] = static_cast<uint8_t>(Result::Success);
		return len;
	}
};

class LoginFailure: public LoginResponse
{
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
	int serialize(char* buf, size_t maxlen) override
	{
		return 0;
	}
};

class MessageSuccess : public MessageResponse
{
	int serialize(char* buf, size_t maxlen) override
	{
		return 0;
	}
};

class MessageFailure: public MessageResponse
{
	int serialize(char* buf, size_t maxlen) override
	{
		return 0;
	}
};


struct Request
{
	virtual ResponsePtr Process(Server& server) = 0;
};

using RequestPtr = std::unique_ptr<Request>;

class SigninRequest: public Request
{
public:
	SigninRequest() = default;
	explicit SigninRequest(const std::string& str) 
		: name(str)
	{}
public:
	ResponsePtr Process(Server& server) override;

public:
	std::string name;

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
	LoginRequest() = default;
	explicit LoginRequest(const std::string& str) 
		: name(str)
	{}
public:
	ResponsePtr Process(Server& server) override;
public:
	std::string name;
public:
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
	std::vector<std::string> names;
	std::string text;

	ResponsePtr Process(Server& server) override;
};
using MessageRequestPtr = std::unique_ptr<MessageRequest>;

MessageRequest ReadMessage(const char* buf, size_t len);
int WriteMessage(const MessageRequest& message, char* buf, size_t maxlen);




RequestPtr ReadRequest(const char* buf, size_t nbytes);
ResponsePtr ReadResponse(const char* buf, size_t nbytes);


void DumpBuffer(const char* buf, size_t len);



