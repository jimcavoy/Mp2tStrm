#pragma once

#include "UdpSender.h"

#include <chrono>
#include <queue>


class RateLimiter
{
	typedef UdpSender::QueueType QueueType;

public:
	RateLimiter(QueueType& in, QueueType& out, int fps);

	void operator() ();

	void stop();

private:
	void poll();

private:
	QueueType& _inQueue;
	QueueType& _outQueue;
	bool _run{ true };
	int _framePerSeconds{};
	std::chrono::steady_clock::time_point _startTime{};
	uint64_t _startPts{};
	std::queue<AccessUnit> _queue;
};
