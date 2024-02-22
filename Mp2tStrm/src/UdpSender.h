#pragma once

#include "BoundedBuffer.h"
#include "UdpData.h"
#include "AccessUnit.h"

#include <queue>
#include <memory>

#ifdef PERFCNTR
#include "Mp2tPerfCntr/BaseIOInterface.h"
#endif 

#ifndef QSIZE
#define QSIZE 100
#endif


class UdpSender
#ifdef PERFCNTR
	: public BaseIOInterface
#endif
{
public:
	typedef AccessUnit DataType;
	typedef BoundedBuffer<DataType, QSIZE> QueueType;
	typedef std::queue<UdpData> UdpQueueType;

public:
	UdpSender(const char* ipaddr, uint32_t port, QueueType& queue, unsigned char ttl, const char* iface_addr);
	~UdpSender();

	void operator()();

	void stop();

	uint64_t count() noexcept;

	uint64_t bytes() noexcept;

	void address(char* addr, size_t len) noexcept;

	long position() noexcept;

private:
	class Impl;
	std::unique_ptr<Impl> _pimpl;
};

