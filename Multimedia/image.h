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

		// ͼ���˲�
		Color image[1000][1000];
		int sumr, sumg, sumb;
		
		// ˮ����ģ��
		FILE* ifp;								//�ļ�ָ��
		char* filename = (char*)"stone.bmp";    //Դͼ���ļ�����������޴���24λBMPͼ��
		unsigned char mybuf[IMAGE_WIDTH * IMAGE_HIGHT * 3 + 100];   //�ڴ棬���ڴ�Դͼ���ļ�
		BITMAPFILEHEADER* pbmfh;      //λͼ�ļ�ͷ�ṹ
		BITMAPINFO* pbmi;			 //λͼ��Ϣͷ�ṹ
		BYTE* pBits;			     //Դλͼ����ָ��
		int cxDib, cyDib;			 //ͼ��Ŀ�Ⱥ͸߶�
		int gaosi[5][5];  //��˹ģ��
		int buffer_1[IMAGE_HIGHT][IMAGE_WIDTH], buffer_2[IMAGE_HIGHT][IMAGE_WIDTH];//���ڼ��㲨��
		int tmp[IMAGE_HIGHT][IMAGE_WIDTH];          //���ڽ������ܾ���
		COLOR det_image[IMAGE_HIGHT][IMAGE_WIDTH];  //Ҫ��ʾ��Ŀ��ͼ��

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

