#pragma once

class BaseIOInterface
{
public:
	BaseIOInterface(void);
	virtual ~BaseIOInterface(void);

	virtual void stop() = 0;

	virtual unsigned long long count() = 0;

	virtual unsigned long long bytes() = 0;

	virtual void address(char* addr, size_t len) = 0;

	virtual long position() = 0;
};

