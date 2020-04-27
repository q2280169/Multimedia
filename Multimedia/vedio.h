#pragma once
#include "framework.h"

#define IMAGE_WIDTH    352
#define IMAGE_HIGHT    288

class Vedio
{
	public:
		class COLOR
		{
			public:
				BYTE b;
				BYTE g;
				BYTE r;
		};

	private:
		FILE* ifp;  //文件指针
		char* filename;
		BYTE mybuf[45621248]; //数组，用于存放视频文件
		BYTE* pBity, * pBitu, * pBitv;
		int y[288][352], u[144][176], v[144][176];

		FILE* ifp2;  //文件指针
		char* filename2;
		BYTE mybuf2[45621248]; //数组，用于存放视频文件 
		BYTE* pBity2, * pBitu2, * pBitv2;
		int y2[288][352], u2[144][176], v2[144][176];

		FILE* ifpback;
		char* filenameback;
		unsigned char mybufback[IMAGE_WIDTH * IMAGE_HIGHT * 3 + 100];
		BITMAPFILEHEADER* pbmfh;
		BITMAPINFO* pbmi;
		BYTE* pbits;
		int  cxDib, cyDib;
		COLOR det_image[IMAGE_HIGHT][IMAGE_WIDTH];
		COLOR det_image2[IMAGE_HIGHT][IMAGE_WIDTH];
		COLOR det_image3[IMAGE_HIGHT][IMAGE_WIDTH];
		int n = 0;
		int m = 0;

		int gaosi[5][5];  //高斯模板
		int buffer_1[IMAGE_HIGHT][IMAGE_WIDTH], buffer_2[IMAGE_HIGHT][IMAGE_WIDTH];//用于计算波能
		int tmp[IMAGE_HIGHT][IMAGE_WIDTH];          //用于交换波能矩阵
		
		void NextFrameWaveEnerge();
		void RenderRipple();

	public:
		Vedio();
		void CALLBACK bofang(HWND hWnd);
		void zimu(HWND hWnd);
		void danrudanchu(HWND hWnd);
		void pinjie(HWND hWnd);
		void water(HWND hWnd);
		void disturb();
};

