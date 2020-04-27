#pragma once
#include "framework.h"
#include <stdio.h>

#define IMAGE_WIDTH 640
#define IMAGE_HIGHT 480

typedef struct COLOR
{
	BYTE r;
	BYTE g;
	BYTE b;
}Color, RPGCOLOR;

class Image
{
	public:
		BITMAPINFOHEADER  bi;
		HBITMAP           hbmp;
		BITMAP            bmp;
		HANDLE            hDib;
		BYTE* lpbitmap = NULL;

		BITMAPINFOHEADER  bi2;
		HBITMAP           hbmp2;
		BITMAP            bmp2;
		HANDLE            hDib2;
		BYTE* lpbitmap2 = NULL;

		// 图像滤波
		Color image[1000][1000];
		int sumr, sumg, sumb;
		
		// 水波纹模拟
		FILE* ifp;								//文件指针
		char* filename = (char*)"stone.bmp";    //源图像文件，本程序仅限处理24位BMP图像
		unsigned char mybuf[IMAGE_WIDTH * IMAGE_HIGHT * 3 + 100];   //内存，用于储源图像文件
		BITMAPFILEHEADER* pbmfh;      //位图文件头结构
		BITMAPINFO* pbmi;			 //位图信息头结构
		BYTE* pBits;			     //源位图数据指针
		int cxDib, cyDib;			 //图像的宽度和高度
		int gaosi[5][5];  //高斯模板
		int buffer_1[IMAGE_HIGHT][IMAGE_WIDTH], buffer_2[IMAGE_HIGHT][IMAGE_WIDTH];//用于计算波能
		int tmp[IMAGE_HIGHT][IMAGE_WIDTH];          //用于交换波能矩阵
		COLOR det_image[IMAGE_HIGHT][IMAGE_WIDTH];  //要显示的目标图像

	public:
		Image(HINSTANCE hInstance);
		void UpDisplay(HDC hdc);
		void DownDisplay(HDC hdc);
		void GrayDisplay(HDC hdc);
		void FudiaoDisPlay(HDC hdc);
		void PlusDisplay(HDC hdc);
		void WaterDisplay(HWND hWnd);
		void NextFrameWaveEnerge();
		void RenderRipple();
		void disturb();
};

