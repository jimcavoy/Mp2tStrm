#include "RateLimiter.h"

#include <iostream>

RateLimiter::RateLimiter(QueueType& in, QueueType& out, double fps)
	:_inQueue(in)
	, _outQueue(out)
	, _framePerSeconds(fps)
{
	_window = (long) (90'000 / _framePerSeconds);
}

void RateLimiter::operator() ()
{
	std::chrono::steady_clock::time_point zero{};

	while (_run)
	{
		AccessUnit au;
		const bool isFull = _inQueue.Get(std::move(au), 10);
		if (isFull)
		{
			if (_startTime == zero && au.timestamp() != 0)
			{
				_startTime = std::chrono::steady_clock::now();
				_startPts = au.timestamp();
			}

			_queue.push(std::move(au));
		}
		else if (_queue.empty())
		{
			stop();
		}

		poll();
	}
}

void RateLimiter::stop() noexcept
{
	_run = false;
}

uint64_t RateLimiter::count() noexcept
{
	uint64_t ret = _framecount;
	_framecount = 0;
	return ret;
}

uint64_t RateLimiter::bytes() noexcept
{
	return 0;
}

void RateLimiter::address(char* addr, size_t len) noexcept
{
}

long RateLimiter::position() noexcept
{
	return _position;
}

void RateLimiter::poll()
{
	std::chrono::steady_clock::time_point zero{};

	if (_queue.empty())
	{
		return;
	}

	if (_startTime != zero)
	{
		AccessUnit& au = _queue.front();
		auto timeNow = std::chrono::steady_clock::now();
		std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(timeNow - _startTime);
		long clockTime = time_span.count() * 90'000;
		long auTime = (long) au.timestamp() - _startPts;
		long diff = abs(clockTime - auTime);

		if (diff < _window || au.timestamp() == 0 || au.timestamp() == _startPts)
		{
#ifndef NDEBUG
			std::cout << auTime << ", " << clockTime << ", " << diff << ", " << time_span.count() <<  std::endl;
#endif
			_position = auTime;
			_outQueue.Put(std::move(au));
			_queue.pop();
			_framecount++;
		}
	}
}
