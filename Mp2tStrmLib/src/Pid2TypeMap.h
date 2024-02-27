#pragma once

#include <map>

#include <mp2tp/tspmt.h>

/// <summary>
/// Represents a Program Map Table that maps PID to 
/// stream type
/// ISO/IEC 13818-1 Table 2-24 - Stream type assignments
/// </summary>
class Pid2TypeMap
{
public:
	enum class STREAM_TYPE
	{
		UNKNOWN,
		H264,
		H265,
		HDMV,
		VIDEO,
		AUDIO,
		KLVA,
		$EXI
	};

private:
	typedef std::map<unsigned short, Pid2TypeMap::STREAM_TYPE> map_type;

public:
	void update(const lcss::ProgramMapTable& pmt);
	STREAM_TYPE packetType(unsigned short pid);
	bool isEmpty() const noexcept
	{
		return _pid2type.empty();
	}

private:
	map_type	_pid2type;
	int			_version{-1};
};

