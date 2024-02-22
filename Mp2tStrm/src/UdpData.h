#pragma once

#include <vector>

#ifndef _WIN32
typedef unsigned char uint8_t;
#endif

class UdpData
{
public:
	enum {
		DEFAULT_BUFLEN = 1500
	};

	UdpData();
	UdpData(uint8_t* buf, size_t len);
	UdpData(const UdpData& rhs);
	UdpData& operator=(const UdpData& rhs);
	UdpData(UdpData&& src) noexcept;
	UdpData& operator=(UdpData&& rhs) noexcept;
	~UdpData(void);

	void swap(UdpData& src);

	uint8_t* data();
	const uint8_t* data() const;
	size_t length() const;

	UdpData& write(const uint8_t* data, int len);

private:
	std::vector<uint8_t> _data;
};

