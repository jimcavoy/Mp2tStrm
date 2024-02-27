#include "AccessUnit.h"

#include <utility>

AccessUnit::AccessUnit()
{
}

AccessUnit::~AccessUnit()
{
}

AccessUnit::AccessUnit(const AccessUnit& orig)
	:_timestamp(orig._timestamp)
{
}

AccessUnit& AccessUnit::operator=(const AccessUnit& rhs)
{
	AccessUnit temp(rhs);
	swap(temp);

	return *this;
}

AccessUnit::AccessUnit(AccessUnit&& src) noexcept
{
	*this = std::move(src);
}

AccessUnit& AccessUnit::operator=(AccessUnit&& rhs) noexcept
{
	if (this != &rhs)
	{
		_units = std::move(rhs._units);
		std::swap(_timestamp, rhs._timestamp);
	}

	return *this;
}

void AccessUnit::swap(AccessUnit& src)
{
	_units.swap(src._units);
	std::swap(_timestamp, src._timestamp);
}

void AccessUnit::add(lcss::TransportPacket& unit)
{
	_units.push_back(unit);
}

void AccessUnit::add(lcss::TransportPacket&& unit)
{
	_units.push_back(unit);
}

size_t AccessUnit::size() const
{
	return _units.size();
}

uint64_t AccessUnit::timestamp() const
{
	return _timestamp;
}

void AccessUnit::setTimestamp(uint64_t ts)
{
	_timestamp = ts;
}

AccessUnit::iterator AccessUnit::begin()
{
	return _units.begin();
}

AccessUnit::iterator AccessUnit::end()
{
	return _units.end();
}
