
#include "stdafx.h"
#include <iomanip>
#include <math.h>
#include <exception>
#include <io.h>
#include <assert.h>
#include <string>

#include "VlpParser.h"

#ifndef PI
#define PI 3.141592653589793
#endif

#define DEG2RAD(d)	((d)*PI/180.0)


static int64_t getFileSize(const char* file)
{
	int64_t size = 0;
	//FILE* fd = fopen(file, "rb");
	FILE *fd = NULL;
	fopen_s(&fd, file, "rb");

	if (fd != NULL){
		size = _filelengthi64(_fileno(fd));
		fclose(fd);
	}
	return size;
}
static void write(FILE* of, const Point& p)
{
	fprintf(of, "%u\t%lf\t%lf\t%lf\t%lf\t%d\n", 
		p.timeStamp, p.Angle, p.x, p.y, p.z, p.i);
}

bool VlpParser::B_INIT = false;
double VlpParser::V_COSs[VlpParser::CH_COUNT];
double VlpParser::V_SINs[VlpParser::CH_COUNT];
double VlpParser::V_ANGLES[VlpParser::CH_COUNT];

VlpParser::VlpParser()
	: m_buf(2048)
{
	if (!B_INIT){
		init();
	}

	m_channel = 0;
	m_onProgress = NULL;
	m_onPoint = NULL;
	m_minDist = 0.5;
	m_maxDist = 120.0;

	m_fin = NULL;
	m_fout = NULL;
	m_rawFileSize = 0;
	m_progress = 0;
	m_bytesRead = 0;
}
VlpParser::~VlpParser()
{
	if (m_fin != NULL){
		fclose(m_fin);
		m_fin = NULL;
	}
	if (m_fout){
		fclose(m_fout);
		m_fout = NULL;
	}
}

void VlpParser::setChannel(int ch)
{
	m_channel = ch;
}
void VlpParser::setOnPoint(pfnPoint pfn)
{
	m_onPoint = pfn;
}
void VlpParser::setOnProgress(pfnProgress pfn)
{
	m_onProgress = pfn;
}
void VlpParser::setMinDist(double min)
{
	m_minDist = min;
}
void VlpParser::setMaxDist(double max)
{
	m_maxDist = max;
}

int VlpParser::getChannel() const
{
	return m_channel;
}
double VlpParser::getMinDist() const
{
	return m_minDist;
}
double VlpParser::getMaxDist() const
{
	return m_maxDist;
}

void VlpParser::parse(std::string raw, std::string out, int flag)
{
	m_fileIn = raw;
	m_fileOut = out;
	beforeParsing();
	
	long long Point_num = 0;
	while(true)
	{		// ��ȡ���ݰ�
		if (!readPacket()){
			break;
		}

		// ��ȡʱ���ǩ
		ubyte* packetStart = (ubyte*)(m_buf.buf());
		uint32_t timeStamp = (*(uint32_t*)(packetStart+1200));	// ���ݰ��е�һ�����ʱ���ǩ, microsecond

		// ����Packet����
		double hAngleDelta = 0.0;
		for (int i = 0; i < 12; i++)
		{
			ubyte* pBlock = packetStart + i*100;
			double hAngle = (*(uint16_t*)(pBlock+2))/100.0;		// degree

			// ��ȡˮƽ�Ƕ�����
			if (i < 11){
				ubyte* pNextBlock = pBlock+100;
				double hNextAngle = (*(uint16_t*)(pNextBlock+2))/100.0;
				hAngleDelta = hNextAngle - hAngle;
				//ͳ�Ƽ��
			}

			for (int j = 0; j < 32; j++)
			{
				/*if (flag != 0 && m_channel != 0 && (m_channel - 17 + flag) != j%CH_COUNT){
					continue;
				}*/
				Point_num++;
				ubyte* pPoint = pBlock + 3*j + 4;
				double dist = (*(uint16_t*)(pPoint))*0.002;				// meter
				if (dist < m_minDist || dist > m_maxDist){				// �����
					continue;
					//�쳣��Ĵ���
				}

				const double BLOCK_TIME = 46.080;						// ����BLOCK��ʱ�� us
				//double time = j < 16 ? j*2.304 : j*2.304 + 18.43;		// ��BLOCK�е�һ�����ʱ��Ϊʱ����� us
				double time = j*2.304;
				double angle = hAngle + (time/BLOCK_TIME)*hAngleDelta;	// ��ΪBLOCK_TIMEʱ����ת�پ���
				angle = DEG2RAD(angle);

				int vIdx = j%CH_COUNT;
				Point p = { 
					timeStamp + (uint32_t)(i*BLOCK_TIME + time),	/*time us*/
					angle,                                          /*angle ����*/
					dist*V_COSs[vIdx]*sin(angle),					/*x, meter*/ 
					dist*V_COSs[vIdx]*cos(angle),					/*y, meter*/
					dist*V_SINs[vIdx],								/*z, meter*/
					(uint16_t)(*(pPoint+2))							/*intensity*/
				};
				onPoint(p, Point_num);
			}
		}

		// ���buf
		m_buf.clear();
	}
	//��β��ܽ�����㰴��ʱ��ֿ�
	afterParsing();
}

void VlpParser::init()
{
	const static double g_vAngles[CH_COUNT] = {
		-30.67,-9.33,-29.33,-8,-28,-6.67,-26.67,-5.33,-25.33,-4,-24,-2.67,-22.67,
		-1.33,-21.33,0,-20,1.33,-18.67,2.67,-17.33,4,-16,5.33,-14.67,6.67,-13.33,
		8,-12,9.33,-10.67,10.67
	};
	memcpy(V_ANGLES, g_vAngles, sizeof(g_vAngles));

	for (int i = 0; i < CH_COUNT; i++){
		double angle = DEG2RAD(V_ANGLES[i]);
		V_SINs[i] = sin(angle);
		V_COSs[i] = cos(angle);
	}
	B_INIT = true;
}
void VlpParser::beforeParsing()
{
	// ���channel
	if (m_channel < 0 || m_channel > 32){
		throw std::exception("Invalid channel.");
	}

	// ��ԭʼ�����ļ�
	m_rawFileSize = getFileSize(m_fileIn.c_str());
	//m_fin = fopen(m_fileIn.c_str(), "rb");
	fopen_s(&m_fin, m_fileIn.c_str(), "rb");

	if (m_fin == NULL){
		throw std::exception("Cannot open raw file.");
	}

	// ��������ļ�
	if (!m_fileOut.empty()){
		//m_fout = fopen(m_fileOut.c_str(), "w");
		fopen_s(&m_fout, m_fileOut.c_str(), "w+");


		if (m_fout == NULL){
			throw std::exception("Cannot open out file.");
		}
	}

	m_bytesRead = 0;
	m_progress = 0.0f;
	m_buf.clear();
	if (m_onProgress != NULL){
		m_onProgress(0.0f);
	}
}
void VlpParser::afterParsing()
{
	m_buf.clear();

	if (m_fin != NULL){
		fclose(m_fin);
		m_fin = NULL;
	}
	if (m_fout){
		fclose(m_fout);
		m_fout = NULL;
	}

	if (m_onProgress != NULL){
		m_onProgress(1.0f);
	}
}
bool VlpParser::readPacket()
{
	while(!feof(m_fin))
	{
		// ������Ȼص�
		if (m_onProgress){
			float processed = (float)m_bytesRead/m_rawFileSize;
			if(processed-m_progress > 0.01f){
				m_progress = processed;
				m_onProgress(m_progress);
			}
		}

		// ��֤buf����1206���ֽ�
		size_t oldSize = m_buf.size();
		if(!read(m_fin, &m_buf, 1206)){
			return false;
		}
		m_bytesRead += 1206-oldSize;

		// ���� ffee
		bool bHeader = false;
		while(m_buf.size() > 2){
			ubyte* pStart = (ubyte*)(m_buf.buf());
			if (*pStart != 0xff || *(pStart+1) != 0xee){
				m_buf.pop_front();
				continue;
			}
			bHeader = true;
			break;
		}
		if (!bHeader){
			continue;
		}

		// ��֤buf����1206���ֽ�
		oldSize = m_buf.size();
		if(!read(m_fin, &m_buf, 1206)){
			return false;
		}
		m_bytesRead += 1206-oldSize;

		// ���packet
		bool bSucess = true;
		for (int i = 0; i < 12; ++i){
			ubyte* pStart = (ubyte*)(m_buf.buf() + i*100);
			if(*pStart != 0xff || *(pStart+1) != 0xee){
				m_buf.pop_front(2);
				bSucess = false;
				break;
			}
		}
		if (!bSucess){
			continue;
		}

		return true;
	}

	return false;
}
void VlpParser::onPoint(const Point& p,int Point_num)
{
	// �ص�
	if(m_onPoint != NULL){
		m_onPoint(p);
	}
	int  TXT_num = Point_num / 34560;
	std::string fileOut(m_fileOut);
	fileOut.replace(fileOut.length() - 4, 1, '_' + std::to_string(TXT_num) + '.');
	fopen_s(&m_fout, fileOut.c_str() , "at+");
	if (m_fout != NULL){
		// !! ����ƿ�� !!
		write(m_fout, p);
		fclose(m_fout);
	}
}

