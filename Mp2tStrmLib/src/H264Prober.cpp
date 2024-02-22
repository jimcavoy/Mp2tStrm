#include "H264Prober.h"

#include <h264p/nalu.h>
#include <h264p/naluimpl.h>
#include <loki/Visitor.h>

class H264Prober::Impl
{
public:
	Impl()
	{

	}

public:
	void onNALUnit(ThetaStream::NALUnit& nalu);

public:
	double		fps{};
	int			width{};
	int			height{};
	std::string profile;
	std::string level;
	int			num_of_frames{};
	bool        interlaced{ false };
	int         aspect_ratio_x{};
	int         aspect_ratio_y{};
};

class NALUnitProbeVisitor
	: public Loki::BaseVisitor,
	public Loki::Visitor<ThetaStream::NALUnitSPS>,
	public Loki::Visitor<ThetaStream::NALUnitAUD>
{
public:
	NALUnitProbeVisitor(H264Prober::Impl& prober)
		:parser_(prober)
	{

	}

	void Visit(ThetaStream::NALUnitSPS& unit)
	{
		if (parser_.fps > 0)
			return;

		switch (unit.profile_idc)
		{
		case 66:  parser_.profile = "baseline"; break;
		case 77:  parser_.profile = "main";     break;
		case 88:  parser_.profile = "extended"; break;
		case 100: parser_.profile = "high";     break;
		}

		char buf[255];
		_itoa_s(unit.level_idc, buf, 10);
		char lvl[8];
		memset(lvl, 0, 8);
		lvl[0] = buf[0];
		lvl[1] = '.';
		lvl[2] = buf[1];
		parser_.level = lvl;
		parser_.width = (unit.pic_width_in_mbs_minus1 + 1) * 16;
		int PicSizeInMapUnits = (unit.pic_height_in_map_units_minus1 + 1) * 16;
		parser_.interlaced = !unit.frame_mbs_only_flag;
		if (unit.frame_mbs_only_flag)
			parser_.height = PicSizeInMapUnits;
		else
			parser_.height = PicSizeInMapUnits * 2;

		if (unit.vui_parameters_present_flag)
		{
			if (unit.vui_seq_parameters.aspect_ratio_info_present_flag)
			{
				switch (unit.vui_seq_parameters.aspect_ratio_idc)
				{
				case 1: parser_.aspect_ratio_x = 1; parser_.aspect_ratio_y = 1;break;
				case 2: parser_.aspect_ratio_x = 12; parser_.aspect_ratio_y = 11;break;
				case 3: parser_.aspect_ratio_x = 10; parser_.aspect_ratio_y = 11;break;
				case 4: parser_.aspect_ratio_x = 16; parser_.aspect_ratio_y = 11;break;
				case 5: parser_.aspect_ratio_x = 40; parser_.aspect_ratio_y = 33;break;
				case 6: parser_.aspect_ratio_x = 24; parser_.aspect_ratio_y = 11;break;
				case 7: parser_.aspect_ratio_x = 20; parser_.aspect_ratio_y = 11;break;
				case 8: parser_.aspect_ratio_x = 32; parser_.aspect_ratio_y = 11;break;
				case 9: parser_.aspect_ratio_x = 80; parser_.aspect_ratio_y = 33;break;
				case 10: parser_.aspect_ratio_x = 18; parser_.aspect_ratio_y = 11;break;
				case 11: parser_.aspect_ratio_x = 15; parser_.aspect_ratio_y = 11;break;
				case 12: parser_.aspect_ratio_x = 64; parser_.aspect_ratio_y = 33;break;
				case 13: parser_.aspect_ratio_x = 160; parser_.aspect_ratio_y = 99;break;
				case 14: parser_.aspect_ratio_x = 4; parser_.aspect_ratio_y = 3;break;
				case 15: parser_.aspect_ratio_x = 3; parser_.aspect_ratio_y = 2;break;
				case 16: parser_.aspect_ratio_x = 2; parser_.aspect_ratio_y = 1;break;
				case 255: {
					parser_.aspect_ratio_x = unit.vui_seq_parameters.sar_width;
					parser_.aspect_ratio_y = unit.vui_seq_parameters.sar_height;
				} break;
				}
			}

			if (unit.vui_seq_parameters.timing_info_present_flag)
			{
				double quot = (double)unit.vui_seq_parameters.time_scale / (double)unit.vui_seq_parameters.num_units_in_tick;
				parser_.fps = quot / 2.0;
				//cout << ",\n\t\"fixed_frame_rate_flag\": " << unit.vui_seq_parameters.fixed_frame_rate_flag;
			}
		}
	}

	void Visit(ThetaStream::NALUnitAUD& unit)
	{
		parser_.num_of_frames++;
	}

private:
	H264Prober::Impl& parser_;
};


void H264Prober::Impl::onNALUnit(ThetaStream::NALUnit& nalu)
{
	NALUnitProbeVisitor vis(*this);
	nalu.Accept(vis);
}


H264Prober::H264Prober()
	:_pimpl(std::make_unique<H264Prober::Impl>())
{
}

H264Prober::~H264Prober()
{
}

void H264Prober::onNALUnit(ThetaStream::NALUnit& nalu)
{
	_pimpl->onNALUnit(nalu);
}

double H264Prober::framesPerSecond() const
{
	return _pimpl->fps;
}

int H264Prober::width() const
{
	return _pimpl->width;
}

int H264Prober::height() const
{
	return _pimpl->height;
}

std::string H264Prober::profile() const
{
	return _pimpl->profile;
}

std::string H264Prober::level() const
{
	return _pimpl->level;
}

int H264Prober::numberOfFrames() const
{
	return _pimpl->num_of_frames;
}

bool H264Prober::isInterlaced() const
{
	return _pimpl->interlaced;
}

int H264Prober::aspectRatioX() const
{
	return _pimpl->aspect_ratio_x;
}

int H264Prober::aspectRatioY() const
{
	return _pimpl->aspect_ratio_y;
}


