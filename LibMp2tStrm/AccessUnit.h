#pragma once

#include "tspckt.h"
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

	unsigned long long timestamp() const;
	void setTimestamp(unsigned long long ts);

	iterator begin();
	iterator end();

private:
	std::vector<lcss::TransportPacket> _units;
	unsigned long long _timestamp{};
};
