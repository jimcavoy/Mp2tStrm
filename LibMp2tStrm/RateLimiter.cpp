#include "RateLimiter.h"


RateLimiter::RateLimiter(QueueType& in, QueueType& out)
	:_inQueue(in)
	, _outQueue(out)
{
}

void RateLimiter::operator() ()
{
	while (_run)
	{
		AccessUnit au;
		const bool isFull = _inQueue.Get(std::move(au), 100);
		if (isFull)
		{
			_outQueue.Put(std::move(au));
		}
		else
		{
			stop();
		}
	}
}

void RateLimiter::stop()
{
	_run = false;
}
