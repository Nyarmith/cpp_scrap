#include "networking.h"

#include <algorithm>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdio>
#include <cctype>

#include <fmt/core.h>

class HttpResponse
{
public:
  HttpResponse(){};
  HttpResponse(std::string resp) : resp_(resp){ /*fprintf(stderr, "%s\n",resp_.c_str());*/ };
  int status();
  std::string body();
private:
  std::string resp_;
};

class HttpClient
{
public:
  HttpClient(std::string server);
  ~HttpClient();
  HttpResponse HEAD(std::string resource);
  HttpResponse GET(std::string resource);
  HttpResponse POST(std::string resource, std::string content_type, std::string content);
  HttpResponse PUT(std::string resource, std::string content_type, std::string content);
  HttpResponse DELETE(std::string resource);
  HttpResponse OPTIONS(std::string resource);

private:
  std::string server_;
  sock_t sock_;
};

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    fprintf(stderr,"usage: <SERVER> <REQUEST> <Param1> [Param2] ... [ParamN]\r\n");
    return 1;
  }

  sockInit();

  std::string uri(argv[1]);
  HttpClient c(uri);

  std::string req(argv[2]);
  std::transform(req.begin(), req.end(), req.begin(), [](unsigned char c) { return std::tolower(c); });

  HttpResponse rsp;
  if (req == "head")          rsp = c.HEAD(argv[3]);
  else if (req == "get")      rsp = c.GET(argv[3]);
  else if (req == "post")     rsp = c.POST(argv[3], argv[4], argv[5]);
  else if (req == "put")      rsp = c.PUT(argv[3], argv[4], argv[5]);
  else if (req == "delete")   rsp = c.DELETE(argv[3]);
  else if (req == "options")  rsp = c.OPTIONS(argv[3]);
  else
  {
    fprintf(stderr, "unrecognized http operation: %s\r\n", argv[2]);
    return 2;
  }

  fprintf(stderr,"response-code: %d\r\nresponse-contents: %s\r\n", rsp.status(), rsp.body().c_str());

  sockQuit();
  return 0;
}

std::string HttpResponse::body()
{
  auto ind = resp_.find("\r\n\r\n");
  return resp_.substr(ind+4);
}

int HttpResponse::status()
{
  auto ind1 = resp_.find(' ');
  auto ind2 = resp_.find(' ', ind1+1);
  std::string code = resp_.substr(ind1,ind2-ind1);
  //fprintf(stderr, "inds: %d %d code; %s\n",ind1, ind2, code);
  return std::stoi(code);
}

HttpClient::HttpClient(std::string server)
{
  struct addrinfo hints, *serv;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  // find matching destinations
  int status = getaddrinfo(server.c_str(), "80", &hints, &serv);

  // go through matches
  struct addrinfo *p;
  for (p = serv; p != NULL; p = p->ai_next)
  {
    if ((sock_ = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) continue;
    if ((connect(sock_,p->ai_addr,p->ai_addrlen)) == -1) { close(sock_); continue; }
    break;
  }
  if (p == NULL)
    throw std::runtime_error("Couldn't Connect to Server " + server);

  server_ = server;
};

std::string rcv(const sock_t &sock)
{

  std::string rsp;
  constexpr size_t MaxBufLen = 4096;
  std::vector<char> buffer(MaxBufLen);
  int recvlen = 0;

  do {
    recvlen = recv(sock, &buffer[0], buffer.size(), 0);
    if (recvlen == -1)
      return {""};
    else
    {
      rsp.append(buffer.cbegin(), buffer.cend());
    }
  } while (recvlen == MaxBufLen);

  return rsp;
}

HttpResponse HttpClient::HEAD(std::string resource)
{
  auto req = fmt::format("HEAD {} HTTP/1.1\r\nAccept: application/json\r\nHost: {}\r\n\r\n", resource, server_);
  send(sock_, req.c_str(), req.size(), 0);
  return {rcv(sock_)};
}

HttpResponse HttpClient::GET(std::string resource)
{
  auto req = fmt::format("GET {} HTTP/1.1\r\nHost: {}\r\n\r\n", resource, server_);
  send(sock_, req.c_str(), req.size(), 0);
  return {rcv(sock_)};
}

HttpResponse HttpClient::POST(std::string resource, std::string content_type, std::string content)
{
  auto req = fmt::format("POST {} HTTP/1.1\r\nHost: {}\r\nContent-Type: {}\r\nContent-Length: {}\r\n\r\n{}\r\n\r\n", resource, server_, content_type, content.size(),content);
  send(sock_, req.c_str(), req.size(), 0);
  return {rcv(sock_)};
}

HttpResponse HttpClient::PUT(std::string resource, std::string content_type, std::string content)
{
  auto req = fmt::format("PUT {} HTTP/1.1\r\nHost: {}\r\nContent-Type: {}\r\nContent-Length: {}\r\n\r\n{}\r\n\r\n", resource, server_, content_type, content.size(),content);
  send(sock_, req.c_str(), req.size(), 0);
  return {rcv(sock_)};
}

HttpResponse HttpClient::DELETE(std::string resource)
{
  auto req = fmt::format("DELETE {} HTTP/1.1\r\nHost: {}\r\n\r\n", resource, server_);
  send(sock_, req.c_str(), req.size(), 0);
  return {rcv(sock_)};
}

HttpResponse HttpClient::OPTIONS(std::string resource)
{
  auto req = fmt::format("OPTIONS {} HTTP/1.1\r\nHost: {}\r\n\r\n", resource, server_);
  send(sock_, req.c_str(), req.size(), 0);
  return {rcv(sock_)};
}

HttpClient::~HttpClient()
{
  close(sock_);
}
