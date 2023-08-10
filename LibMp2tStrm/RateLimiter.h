#pragma once

#include "UdpSender.h"


class RateLimiter
{
	typedef UdpSender::QueueType QueueType;

public:
	RateLimiter(QueueType& in, QueueType& out);

	void operator() ();

	void stop();

private:
	QueueType& _inQueue;
	QueueType& _outQueue;
	bool _run{ true };
};
