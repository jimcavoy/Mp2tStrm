#include "FileReader.h"

#ifdef _WIN32
#include <io.h>
#endif
#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <fstream>

using namespace std;

FileReader::FileReader(const char* filename, FileReader::QueueType& q, std::streamsize filesize)
	: _queue(q)
	, _count(0)
	, _bytes(0)
	, _run(true)
	, _filesize(filesize)
{
	if (strcmp(filename, "-") == 0)
	{
#ifdef _WIN32
		_setmode(_fileno(stdin), _O_BINARY);
#endif
		_ifile.reset(&cin, [](...) {});
	}
	else
	{
		ifstream* tsfile = new std::ifstream(filename, std::ios::binary);
		if (!tsfile->is_open())
		{
			char szErr[512]{};
#ifdef _WIN32
			sprintf_s(szErr, "Failed to open input file %s", filename);
#else
			sprintf(szErr, "Failed to open input file %s", filename);
#endif
			std::runtime_error exp(szErr);
			throw exp;
		}
		_ifile.reset(tsfile);
	}

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

long FileReader::position() noexcept
{
	return 0;
}

void FileReader::address(char* addr, size_t len) noexcept
{
#ifdef _WIN32
	strcpy_s(addr, len, _address.c_str());
#else
	strcpy(addr, _address.c_str());
#endif
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
			_readcount += len;
		}
		else
		{
			if (_readcount >= _filesize)
			{
				stop();
			}
			else // sometimes we get a bad I/O read so reset the state
			{
				_ifile->clear();
			}
		}
	}
}
