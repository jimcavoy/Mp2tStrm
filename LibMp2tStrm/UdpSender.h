#pragma once

#include "BoundedBuffer.h"
#include "tspckt.h"
#include "UdpData.h"

#include <queue>
#include <memory>

class UdpSender
{
public:
	typedef lcss::TransportPacket DataType;
	typedef BoundedBuffer<DataType> QueueType;
	typedef std::queue<UdpData> UdpQueueType;

public:
	UdpSender(const char* ipaddr, uint32_t port, QueueType& queue, unsigned char ttl, const char* iface_addr);
	~UdpSender();

	void operator()();

	void stop();

	uint64_t count() noexcept;

	uint64_t bytes() noexcept;

	void address(char* addr, size_t len) noexcept;

private:
	void addToQueue(const DataType& data);
	void poll();
	void send(const UdpData& data);

private:
	class Impl;
	std::unique_ptr<Impl> _pimpl;
};

