#include "Pid2TypeMap.h"

#include <string.h>

#ifdef _WIN32
#define strncpy strncpy_s
#define stricmp _stricmp
#else
#define strncpy strncpy
#define stricmp strcasecmp
#endif

namespace
{
	char TAG_HDMV[] = { (char)0x48, (char)0x44, (char)0x4D, (char)0x56, (char)0xFF, (char)0x1B, (char)0x44, (char)0x3F, 0 };
	char TAG_HDPR[] = { (char)0x48, (char)0x44, (char)0x50, (char)0x52, (char)0xFF, (char)0x1B, (char)0x67, (char)0x3F, 0 };
}

void Pid2TypeMap::update(const lcss::ProgramMapTable& pmt)
{
	// Only update if the version is greater than the pervious version
	if (_version < pmt.version_number())
	{
		_version = (int)pmt.version_number();
		_pid2type.clear();
		for (const auto& pe : pmt)
		{
			switch (pe.stream_type())
			{
			case 0x06: // Rec. ITU-T H.222.0 | ISO/IEC 13818-1 PES packets containing private data
			case 0x15: // Metadata carried in PES packets
			{
				char value[255]{};
				char format_identifier[5]{};

				for (const auto& desc : pe)
				{
					// registration_descriptor
					if (desc.tag() == 0x05)
					{
						desc.value((BYTE*)value);
						strncpy(format_identifier, value, 4);
						break;
					}
					// metadata_descriptor
					else if (desc.tag() == 0x26)
					{
						desc.value((BYTE*)value);
						strncpy(format_identifier, value + 3, 4);
						break;
					}
				}

				if (strcmp(format_identifier, "KLVA") == 0)
				{
					_pid2type.insert({ pe.pid(), STREAM_TYPE::KLVA });
				}
				else if (strcmp(format_identifier, "$EXI") == 0)
				{
					_pid2type.insert({ pe.pid(), STREAM_TYPE::$EXI });
				}
			}
			break;
			case 0x1B: // H.264
			{
				char value[255]{};
				for (const auto& desc : pe)
				{
					// registration_descriptor
					if (desc.tag() == 0x05)
					{
						desc.value((BYTE*)value);
						break;
					}
				}

				if (strcmp(value, TAG_HDMV) == 0 || strcmp(value, TAG_HDPR) == 0)
				{
					_pid2type.insert({ pe.pid(), STREAM_TYPE::HDMV });
				}
				else
				{
					_pid2type.insert({ pe.pid(), STREAM_TYPE::H264 });
				}
			}
			break;
			case 0x24: // H.265
			{
				char value[255]{};
				for (const auto& desc : pe)
				{
					// registration_descriptor
					if (desc.tag() == 0x05)
					{
						desc.value((BYTE*)value);
						break;
					}
				}

				if (stricmp(value, "HEVC") == 0)
				{
					_pid2type.insert({ pe.pid(), STREAM_TYPE::H265 });
				}
			}
			break;
			case 0x02: // MPEG-2 video
			{
				_pid2type.insert({ pe.pid(), STREAM_TYPE::VIDEO });
			}
			break;
			case 0x03: // ISO / IEC 11172 - 3 Audio
			case 0x04: // ISO / IEC 13818 - 3 Audio
			case 0x0F: // ISO/IEC 13818-7 Audio with ADTS transport syntax
			{
				_pid2type.insert({ pe.pid(), STREAM_TYPE::AUDIO });
			}
			break;
			}
		}
	} // if
}

Pid2TypeMap::STREAM_TYPE Pid2TypeMap::packetType(unsigned short pid)
{
	STREAM_TYPE type = STREAM_TYPE::UNKNOWN;

	map_type::iterator it = _pid2type.find(pid);
	if (it != _pid2type.end())
	{
		type = it->second;
	}

	return type;
}
