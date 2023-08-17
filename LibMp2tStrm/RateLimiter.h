#pragma once

#include "UdpSender.h"

#ifdef PERFCNTR
#include "Mp2tPerfCntr/BaseIOInterface.h"
#endif

#include <chrono>
#include <queue>


class RateLimiter
#ifdef PERFCNTR
	: public BaseIOInterface
#endif
{
	typedef UdpSender::QueueType QueueType;

public:
	RateLimiter(QueueType& in, QueueType& out, double fps);

	void operator() ();

	void stop() noexcept;

	uint64_t count() noexcept;

	uint64_t bytes() noexcept;

	void address(char* addr, size_t len) noexcept;

	long position() noexcept;

private:
	void poll();

private:
	QueueType& _inQueue;
	QueueType& _outQueue;
	bool _run{ true };
	double _framePerSeconds{};
	std::chrono::steady_clock::time_point _startTime{};
	uint64_t _startPts{};
	std::queue<AccessUnit> _queue;
	long _window;
	uint64_t _framecount{};
	long _position{};
};
