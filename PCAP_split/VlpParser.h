
#pragma once

#include <stdint.h>
#include <fstream>
#include "buffer.h"

struct Point
{
	uint32_t timeStamp;	// us
	double Angle;
	double x;			// meter
	double y;
	double z;
	uint16_t i;
};

typedef void (*pfnProgress)(float);
typedef void (*pfnPoint)(const Point&);

class VlpParser
{
public:
	VlpParser();
	~VlpParser();

	void parse(std::string raw, std::string out, int flag);
	void setChannel(int ch);
	void setOnPoint(pfnPoint pfn);
	void setOnProgress(pfnProgress pfn);
	void setMinDist(double min);
	void setMaxDist(double max);

	int getChannel() const;
	double getMinDist() const;
	double getMaxDist() const;

private:
	void init();
	void beforeParsing();
	void afterParsing();
	bool readPacket();
	void onPoint(const Point& p,int Point_num);

private:
	int			m_channel;	// 0表示所有通道; 指定通道(16或者32)
	std::string	m_fileIn;
	std::string	m_fileOut;
	pfnProgress	m_onProgress;
	pfnPoint	m_onPoint;
	double		m_minDist;
	double		m_maxDist;

	FILE*		m_fin;
	FILE*		m_fout;
	int64_t		m_rawFileSize;
	int64_t		m_bytesRead;
	float		m_progress;
	buffer		m_buf;
	
private:
	static bool			B_INIT;
	static const int	CH_COUNT = 32;
	static double		V_ANGLES[CH_COUNT];
	static double		V_COSs[CH_COUNT];
	static double		V_SINs[CH_COUNT];
};

