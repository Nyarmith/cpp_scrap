#include "networking.h"
#include <cstdio>
#include <cctype>
#include <string>
#include <unordered_map>
#include <format>

class HttpResponse
{
public:
  HttpResponse(std::string resp) : resp_(resp){};
  int status();
  std::string body();
private:
  std::string resp_;
};

class HttpClient
{
public:
  HttpRequest(std::string server);
  ~HttpRequest();
  HttpResponse HEAD(std::string resource);
  HttpResponse GET(std::string resource);
  HttpResponse POST(std::string resource, std::string content_type, std::string content);
  HttpResponse PUT(std::string resource, std::string content_type, std::string content);
  HttpResponse DELETE(std::string resource);
  HttpResponse OPTIONS(std::string resource);

private:
  std::string server_;
  sock_t socket_;
};

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    fprintf("usage: <SERVER> <REQUEST> <Param1> [Param2] ... [ParamN]");
    return 1;
  }

  sockInit();

  std::string uri(argv[0]);
  HttpClient c(uri);

  std::string req(argv[1]);
  std::transform(req.begin(), req.end(), req.begin(), [](unsigned char c) { return std::tolower(c); });

  HttpResponse rsp;
  if (req == "head")          rsp = c.HEAD(argv[2]);
  else if (req == "get")      rsp = c.GET(argv[2]);
  else if (req == "post")     rsp = c.POST(argv[2], argv[3], argv[4]);
  else if (req == "put")      rsp = c.PUT(argv[2], argv[3], argv[4]);
  else if (req == "delete")   rsp = c.DELETE(argv[2]);
  else if (req == "options")  rsp = c.OPTIONS(argv[2]);
  else
  {
    fprintf(stderr, "unrecognized http operation: %s\n", argv[1]);
    return 2;
  }

  fprintf("response-code: %d\nresponse-contents: %s\n", rsp.status(), rsp.body().c_str());

  sockQuit();
  return 0;
}

HttpResponse::body()
{
  auto ind = resp_.find("\n\n");
  return resp_.substr(ind);
}

HttpResponse::status()
{
  auto ind1 = resp_.find(' ');
  auto ind2 = resp_.find(' ', ind1);
  std::string code = resp_.substr(ind1,ind2-ind1);
  return std::stoi(code);
}

HttpClient::HttpClient(std::string server)
{
  struct addrinfo hints, *serv;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  // find matching destinations
  int status = getaddrinfo(address, 80, &hints, &serv);

  // go through matches
  struct addrinfo *p;
  for (p = serv; p != NULL; p = p->ai_next)
  {
    if ((sock_ = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) continue;
    if ((connect(sock_,p->ai_addr,p->ai_addrlen)) != 0) { close(sock_); continue; }
    break;
  }
  if (p == NULL)
    throw std::runtime_error("Couldn't Connect to Server");
};

std::string rcv(const sock_t &sock)
{

  std::string rsp;
  constexpr size_t MaxBufLen = 4096;
  std::vector<char> buffer(MaxBufLen);
  int recvlen = 0;

  do {
    recvlen = recv(sock, &buffer[0], buffer.size(), 0);
    if (rlen == -1)
      return {""};
    else
      rsp.append(buffer.cbegin(), buffer.cend());
  } while (recvlen == MaxBufLen);

  return rsp;
}

HttpResponse HttpClient::HEAD(std::string resource)
{
  auto req = std::format("HEAD {} HTTP/1.1\nAccept: application/json\nHost: {}\n", resource, server_);
  send(sock_, req.c_str(), req.size(), 0);
  return {rcv(sock_)};
}

HttpResponse HttpClient::GET(std::string resource)
{
  auto req = std::format("GET {} HTTP/1.1\nHost: {}\n", resource, server_);
  send(sock_, req.c_str(), req.size(), 0);
  return {rcv(sock_)};
}

HttpResponse HttpClient::POST(std::string resource, std::string content_type, std::string content)
{
  auto req = std::format("POST {} HTTP/1.1\nHost: {}\nContent-Type: {}\nContent-Length: {}\n\n{}", resource, server_, content_type, content.size(),content);
  send(sock_, req.c_str(), req.size(), 0);
  return {rcv(sock_)};
}

HttpResponse HttpClient::PUT(std::string resource, std::string content_type, std::string content)
{
  auto req = std::format("PUT {} HTTP/1.1\nHost: {}\nContent-Type: {}\nContent-Length: {}\n\n{}", resource, server_, content_type, content.size(),content);
  send(sock_, req.c_str(), req.size(), 0);
  return {rcv(sock_)};
}

HttpResponse HttpClient::DELETE(std::string resource)
{
  auto req = std::format("DELETE {} HTTP/1.1\nHost: {}\n", resource, server_);
  send(sock_, req.c_str(), req.size(), 0);
  return {rcv(sock_)};
}

HttpResponse HttpClient::OPTIONS(std::string resource)
{
  auto req = std::format("OPTIONS {} HTTP/1.1\nHost: {}\n", resource, server_);
  send(sock_, req.c_str(), req.size(), 0);
  return {rcv(sock_)};
}
