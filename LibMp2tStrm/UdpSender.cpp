#include "UdpSender.h"

#include <iostream>
#include <string>
#include <string.h>

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#endif

#ifdef _WIN32
#define sprintf sprintf_s
#else
#define sprintf sprintf
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define PVOID void*
#define SD_SEND SHUT_WR
#define SOCKADDR sockaddr
#define IN_ADDR in_addr
#endif

using namespace std;
const int BUFLEN = 1315;

class UdpSender::Impl
{
public:
	Impl(UdpSender::QueueType& q)
		:_queue(q)
	{}

public:
	bool _run{ true };
	SOCKET _socket{};
	sockaddr_in _recvAddr{};
	uint64_t _count{ 0 };
	uint64_t _bytes{ 0 };
	QueueType& _queue;
	std::string _address;
	UdpQueueType _udpQueue;
	UdpData _udpData;
};

UdpSender::UdpSender(const char* ipaddr, uint32_t port, UdpSender::QueueType& q, unsigned char ttl, const char* iface_addr)
{
	_pimpl = std::make_unique<UdpSender::Impl>(q);

	char szErr[BUFSIZ]{};
	IN_ADDR inaddr;
#ifdef _WIN32
	WORD winsock_version, err;
	WSADATA winsock_data;
	winsock_version = MAKEWORD(2, 2);

	err = WSAStartup(winsock_version, &winsock_data);
	if (err != 0)
	{
		std::exception exp("Failed to initialize WinSock");
		throw exp;
	}
#endif

	//----------------------
	// Create a SOCKET for connecting to server
	_pimpl->_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (_pimpl->_socket == INVALID_SOCKET) {
#ifdef _WIN32
		WSACleanup();
		sprintf(szErr, "Error at socket(): %d", WSAGetLastError());
#else
		sprintf(szErr, "Error at socket(): %d", errno);
#endif
		std::runtime_error exp(szErr);
		throw exp;
	}

	// Set time to live
	unsigned char optVal = ttl;
	int optLen = sizeof(optVal);

	if (setsockopt(_pimpl->_socket, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&optVal, optLen) == SOCKET_ERROR)
	{
#ifdef _WIN32
		WSACleanup();
		sprintf(szErr, "Error setting socket option IP_MULTICAST_TTL: %d", WSAGetLastError());
#else
		sprintf(szErr, "Error setting socket option IP_MULICAST_TTL: %d", errno);
#endif
		std::runtime_error exp(szErr);
		throw exp;
	}

	if (strlen(iface_addr) > 0)
	{
		inet_pton(AF_INET, iface_addr, (PVOID)&inaddr);
		if (setsockopt(_pimpl->_socket, IPPROTO_IP, IP_MULTICAST_IF, (char*)&inaddr.s_addr, sizeof(inaddr.s_addr)) == SOCKET_ERROR)
		{
#ifdef _WIN32
			WSACleanup();
			sprintf(szErr, "UdpSender Error setting socket option IP_MULTICAST_IF: %d", WSAGetLastError());
#else
			sprintf(szErr, "UdpSender Error setting socket option IP_MULTICAST_IF: %d", errno);
#endif
			std::runtime_error exp(szErr);
			throw exp;
		}
	}

	if (inet_pton(AF_INET, ipaddr, (PVOID)&inaddr) != 1)
	{
#ifdef _WIN32
		WSACleanup();
		sprintf_s(szErr, "Error when calling inet_pton: %d", WSAGetLastError());
#else
		sprintf(szErr, "Error when calling inet_pton: %d", errno);
#endif
		std::runtime_error exp(szErr);
		throw exp;
	}
	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port for the socket that is being send to
	_pimpl->_recvAddr.sin_family = AF_INET;
	_pimpl->_recvAddr.sin_addr.s_addr = inaddr.s_addr;
	_pimpl->_recvAddr.sin_port = htons(port);

	_pimpl->_address = ipaddr;
	_pimpl->_address += ":";
	_pimpl->_address += std::to_string(port);
}

UdpSender::~UdpSender(void)
{

}

void UdpSender::stop()
{
	shutdown(_pimpl->_socket, SD_SEND);
#ifdef _WIN32
	closesocket(_pimpl->_socket);
#else
	close(_pimpl->_socket);
#endif
	for (int i = 0; i < 10; i++)
	{
		DataType d;
		const bool isFull = _pimpl->_queue.Get(std::move(d), 0);
	}
	_pimpl->_run = false;
}

void UdpSender::operator()()
{
	while (_pimpl->_run)
	{
		DataType d;
		const bool hasData = _pimpl->_queue.Get(std::forward<DataType>(d), 200);
		if (hasData)
		{
			addToQueue(d);
			poll();
		}
		else
		{
			stop();
		}
	}
}

void UdpSender::send(const UdpData& data)
{
	const int status = sendto(_pimpl->_socket,
		(char*)data.data(),
		(int)data.length(),
		0,
		(SOCKADDR*)&_pimpl->_recvAddr,
		sizeof(_pimpl->_recvAddr));

	if (status == SOCKET_ERROR)
	{
#ifdef _WIN32
		const UINT32 errCode = WSAGetLastError();
#else
		const int errCode = errno;
#endif
	}
	else
	{
		_pimpl->_count++;
		_pimpl->_bytes += data.length();
	}
}

uint64_t UdpSender::count() noexcept
{
	return _pimpl->_count;
}

uint64_t UdpSender::bytes() noexcept
{
	const uint64_t ret = _pimpl->_bytes;
	_pimpl->_bytes = 0;
	return ret;
}

void UdpSender::address(char* addr, size_t len) noexcept
{
#ifdef _WIN32
	strcpy_s(addr, len, _pimpl->_address.c_str());
#else
	strcpy(addr, _pimpl->_address.c_str());
#endif
}

void UdpSender::addToQueue(const DataType& tsData)
{

	if (_pimpl->_udpData.length() > BUFLEN)
	{
		_pimpl->_udpQueue.push(std::forward<UdpData>(_pimpl->_udpData));
	}
	_pimpl->_udpData.write(tsData.data(), lcss::TransportPacket::TS_SIZE);
}

void UdpSender::poll()
{
	if (_pimpl->_run && !_pimpl->_udpQueue.empty())
	{
		send(_pimpl->_udpQueue.front());
		_pimpl->_udpQueue.pop();
	}
}