#pragma once
#include "BoundedBuffer.h"
#include "UdpData.h"

#include <fstream>
#include <memory>
#include <vector>

#ifdef PERFCNTR
#include "Mp2tPerfCntr/BaseIOInterface.h"
#endif

#ifndef QSIZE
#define QSIZE 100
#endif

class FileReader
#ifdef PERFCNTR
	: public BaseIOInterface
#endif
{
public:
	typedef BoundedBuffer<UdpData, QSIZE> QueueType;

public:
	FileReader(const char* filename, QueueType& q, size_t bufsiz);
	~FileReader();

	void stop() noexcept;

	uint64_t count() noexcept;

	uint64_t bytes() noexcept;

	void address(char* addr, size_t len) noexcept;

	void operator () ();

private:
	bool _run;
	QueueType& _queue;
	uint64_t _count;
	uint64_t _bytes;
	std::shared_ptr<std::istream> _ifile;
	std::vector<uint8_t> _buffer;
	size_t _bufsiz;
	std::string _address;
};

