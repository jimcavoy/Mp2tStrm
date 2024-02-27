#pragma once

#include <mp2tp/tspckt.h>
#include <vector>

class AccessUnit
{
public:
	typedef std::vector<lcss::TransportPacket>::iterator iterator;

public:
	AccessUnit();
	~AccessUnit();
	AccessUnit(const AccessUnit& orig);
	AccessUnit& operator=(const AccessUnit& rhs);
	AccessUnit(AccessUnit&& src) noexcept;
	AccessUnit& operator=(AccessUnit&& rhs) noexcept;

	void swap(AccessUnit& src);

	void add(lcss::TransportPacket& unit);
	void add(lcss::TransportPacket&& unit);

	size_t size() const;

	uint64_t timestamp() const;
	void setTimestamp(uint64_t ts);

	iterator begin();
	iterator end();

private:
	std::vector<lcss::TransportPacket> _units;
	uint64_t _timestamp{};
};
