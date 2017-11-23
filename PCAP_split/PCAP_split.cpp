// PCAP_split.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

#include <iostream>
#include <vector>
#include <string.h>
#include <io.h>
#include "VlpParser.h"

int g_progress = 0;
void createProgressBar()
{
	for (int i = 1; i <= 50; ++i) {
		std::cout << "=";
	}
	for (int i = 1; i <= 50; ++i) {
		std::cout << "\b";
	}
	g_progress = 0;
}
void progress(float p)
{
	int iProgress = (int)(p * 50);
	if (iProgress < g_progress || g_progress >= 50) {
		return;
	}
	for (int i = g_progress; i < iProgress; ++i) {
		std::cout << ">";
	}
	g_progress = iProgress;
	if (g_progress == 50) {
		std::cout << std::endl;
		return;
	}
}

void onPoint(const Point& p)
{
	//....�����������������������
}

void getFiles(std::string path, std::vector<std::string>& files)
{
	//�ļ����
	long hFile = 0;
	//�ļ���Ϣ
	struct _finddata_t fileinfo;
	std::string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//�����Ŀ¼������
			//������ǣ������б�
			if ((fileinfo.attrib & _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
				{
					getFiles(p.assign(path).append("\\").append(fileinfo.name), files);
				}
			}
			else
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}

		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

int main(int argc, char* argv[])
{
	int ch = 0;

	std::string raw;   //�����ļ���
	std::string out;   //����ļ���

	const char* filePath = "E:/20171025���¼�У����/20171025-1/Laser/RawData/32";
	std::vector<std::string>files;
	std::vector<std::string>filesout;

	getFiles(filePath, files);   //��ȡ�ļ����������ļ�

	if (files.size() == 0)
	{
		std::cout << "No Find Files!" << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "ALL Files: " << files.size() << std::endl;

	int flag = 0;

	std::cout << "������߻���32�ߣ�0-16����";
	std::cin >> flag;
	if (flag<0 || flag>32)
	{
		flag = flag % 32;
	}

	if (flag == 0)
	{
		std::cout << "���32��" << std::endl;
	}
	else
	{
		std::cout << "������ߣ�" << flag << "��" << std::endl;
	}

	//���·��
	filesout = files;
	std::string str;
	std::string str1("txt");

	if (flag == 0)
	{
		str.assign("../result1/");
	}
	else
	{
		str.assign("../result32/");
	}

	for (int i = 0; i < filesout.size(); i++)
	{
		filesout[i].replace(filesout[i].length() - 4, 4, str1);//��ʼλ�ã��滻�ַ���
		filesout[i].replace(0, 52, str);
	}
	//ch��������channel
	ch = 32;
	for (int i = 0; i < files.size(); i++)
	{
		raw = files[i];
		out = filesout[i];

		std::cout << "Start to parse raw data: " << raw.c_str() << std::endl;
		createProgressBar();

		try {
			VlpParser parser;
			parser.setChannel(ch);
			parser.setOnProgress(progress);
			parser.setOnPoint(onPoint);
			parser.parse(raw, out, flag);
		}
		catch (std::exception& e) {
			std::cout << e.what() << std::endl;
		}

		raw.clear();
		out.clear();
	}


	files.clear();
	filesout.clear();

	return EXIT_SUCCESS;
}

