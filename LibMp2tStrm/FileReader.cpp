#include "FileReader.h"

#include <io.h>
#include <fcntl.h>
#include <iostream>

using namespace std;

FileReader::FileReader(const char* filename, FileReader::QueueType& q, size_t bufsiz)
	: _queue(q)
	, _count(0)
	, _bytes(0)
	, _run(true)
	, _bufsiz(bufsiz)
{
	if (strcmp(filename, "-") == 0)
	{
		_setmode(_fileno(stdin), _O_BINARY);
		_ifile.reset(&cin, [](...) {});
	}
	else
	{
		ifstream* tsfile = new std::ifstream(filename, std::ios::binary);
		if (!tsfile->is_open())
		{
			char szErr[BUFSIZ];
			sprintf_s(szErr, "Failed to open input file %s", filename);
			std::exception exp(szErr);
			throw exp;
		}
		_ifile.reset(tsfile);
	}

	_buffer = std::vector<uint8_t>(_bufsiz);
	_address = filename;
}

FileReader::~FileReader(void)
{

}

void FileReader::stop() noexcept
{
	_run = false;
}

uint64_t FileReader::count() noexcept
{
	return _count;
}

uint64_t FileReader::bytes() noexcept
{
	const uint64_t ret = _bytes;
	_bytes = 0;
	return ret;
}

void FileReader::address(char* addr, size_t len) noexcept
{
	strcpy_s(addr, len, _address.c_str());
}

void FileReader::operator()()
{
	while (_run)
	{
		if (_ifile->good())
		{
			_ifile->read((char*)_buffer.data(), _bufsiz);
			const streamsize len = _ifile->gcount();
			_queue.Put(UdpData(_buffer.data(), len));
			_count += len / 188;
			_bytes += len;
		}
		else
		{
			stop();
			return;
		}
	}
}
