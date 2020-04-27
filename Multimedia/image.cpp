#include "image.h"

Image::Image(HINSTANCE hInstance)
{
	hbmp = (HBITMAP)LoadImage(hInstance, L"bmp1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	GetObject(hbmp, sizeof(BITMAP), &bmp);
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = bmp.bmWidth;
	bi.biHeight = bmp.bmHeight;
	bi.biPlanes = bmp.bmPlanes;
	bi.biBitCount = bmp.bmBitsPixel;
	bi.biCompression = bmp.bmType;
	bi.biSizeImage = bmp.bmWidth * bmp.bmHeight * bmp.bmBitsPixel / 8;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrImportant = 0;
	hDib = GlobalAlloc(GHND, bi.biSizeImage);
	lpbitmap = (BYTE*)GlobalLock(hDib);

	hbmp2 = (HBITMAP)LoadImage(hInstance, L"bmp2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	GetObject(hbmp2, sizeof(BITMAP), &bmp2);
	bi2.biSize = sizeof(BITMAPINFOHEADER);
	bi2.biWidth = bmp2.bmWidth;
	bi2.biHeight = bmp2.bmHeight;
	bi2.biPlanes = bmp2.bmPlanes;
	bi2.biBitCount = bmp2.bmBitsPixel;
	bi2.biCompression = bmp2.bmType;
	bi2.biSizeImage = bmp2.bmWidth * bmp2.bmHeight * bmp2.bmBitsPixel / 8;
	bi2.biXPelsPerMeter = 0;
	bi2.biYPelsPerMeter = 0;
	bi2.biClrImportant = 0;
	hDib2 = GlobalAlloc(GHND, bi2.biSizeImage);
	lpbitmap2 = (BYTE*)GlobalLock(hDib2);

	fopen_s(&ifp, filename, "r");
	fread(mybuf, 1155072, 1, ifp);
	pbmfh = (BITMAPFILEHEADER*)mybuf;
	pbmi = (BITMAPINFO*)(pbmfh + 1);
	pBits = (BYTE*)pbmfh + pbmfh->bfOffBits;
	if (pbmi->bmiHeader.biSize == sizeof(BITMAPCOREHEADER))
	{
		cxDib = ((BITMAPCOREHEADER*)pbmi)->bcWidth;
		cyDib = ((BITMAPCOREHEADER*)pbmi)->bcHeight;
	}
	else
	{
		cxDib = pbmi->bmiHeader.biWidth;
		cyDib = abs(pbmi->bmiHeader.biHeight);
	}

	// 高斯模板
	gaosi[0][0] = 1; gaosi[0][1] = 5; gaosi[0][2] = 8; gaosi[0][3] = 5; gaosi[0][4] = 1;
	gaosi[1][0] = 5; gaosi[1][1] = 21; gaosi[1][2] = 34; gaosi[1][3] = 21; gaosi[1][4] = 5;
	gaosi[2][0] = 8; gaosi[2][1] = 34; gaosi[2][2] = 56; gaosi[2][3] = 34; gaosi[2][4] = 8;
	gaosi[3][0] = 5; gaosi[3][1] = 21; gaosi[3][2] = 34; gaosi[3][3] = 21; gaosi[3][4] = 5;
	gaosi[4][0] = 1; gaosi[4][1] = 5; gaosi[4][2] = 8; gaosi[4][3] = 5; gaosi[4][4] = 1;

	// 波能初始化
	for (int y = 0; y < IMAGE_HIGHT; y++)
		for (int x = 0; x < IMAGE_WIDTH; x++)
		{
			buffer_1[y][x] = 0;
			buffer_2[y][x] = 0;
		}

}

void Image::UpDisplay(HDC hdc)
{
	GetDIBits(hdc, hbmp, 0, (UINT)bmp.bmHeight, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

	for (int i = 0; i < bi.biHeight; i++)
		for (int j = 0; j < bi.biWidth; j++) {
			BYTE r = *(lpbitmap + 2 + j * 4 + (bi.biHeight - i - 1) * bi.biWidth * 4);
			BYTE g = *(lpbitmap + 1 + j * 4 + (bi.biHeight - i - 1) * bi.biWidth * 4);
			BYTE b = *(lpbitmap + 0 + j * 4 + (bi.biHeight - i - 1) * bi.biWidth * 4);
			SetPixel(hdc, j + 50, i + 50, RGB(r, g, b));
		}
};

void Image::DownDisplay(HDC hdc)
{
	GetDIBits(hdc, hbmp, 0, (UINT)bmp.bmHeight, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
	for (int i = 0; i < bi.biHeight; i++)
		for (int j = 0; j < bi.biWidth; j++) {
			BYTE r = *(lpbitmap + 2 + j * 4 + i * bi.biWidth * 4);
			BYTE g = *(lpbitmap + 1 + j * 4 + i * bi.biWidth * 4);
			BYTE b = *(lpbitmap + 0 + j * 4 + i * bi.biWidth * 4);
			SetPixel(hdc, j + 350, i + 50, RGB(r, g, b));
		}
}

void Image::GrayDisplay(HDC hdc)
{
	GetDIBits(hdc, hbmp, 0, (UINT)bmp.bmHeight, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
	for (int i = 0; i < bi.biHeight; i++)
		for (int j = 0; j < bi.biWidth; j++) {
			BYTE r = *(lpbitmap + 2 + j * 4 + (bi.biHeight - i - 1) * bi.biWidth * 4);
			BYTE g = *(lpbitmap + 1 + j * 4 + (bi.biHeight - i - 1) * bi.biWidth * 4);
			BYTE b = *(lpbitmap + 0 + j * 4 + (bi.biHeight - i - 1) * bi.biWidth * 4);
			SetPixel(hdc, j + 650, i + 50, RGB(r, r, r));
		}
}

void Image::FudiaoDisPlay(HDC hdc)
{
	GetDIBits(hdc, hbmp, 0, (UINT)bmp.bmHeight, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
	for (int y = 0; y < bi.biHeight - 1; y++)
		for (int x = 0; x < bi.biWidth - 1; x++)
		{
			int r, g, b;
			int r_rightSide, g_rightSide, b_rightSide;
			int r_underSide, g_underSide, b_underSide;
			r = *(lpbitmap + 2 + x * 4 + (bi.biHeight - y - 1) * bi.biWidth * 4);
			g = *(lpbitmap + 1 + x * 4 + (bi.biHeight - y - 1) * bi.biWidth * 4);
			b = *(lpbitmap + 0 + x * 4 + (bi.biHeight - y - 1) * bi.biWidth * 4);
			r_rightSide = *(lpbitmap + 2 + (x + 1) * 4 + (bi.biHeight - y - 1) * bi.biWidth * 4);
			g_rightSide = *(lpbitmap + 1 + (x + 1) * 4 + (bi.biHeight - y - 1) * bi.biWidth * 4);
			b_rightSide = *(lpbitmap + 0 + (x + 1) * 4 + (bi.biHeight - y - 1) * bi.biWidth * 4);
			r_underSide = *(lpbitmap + 2 + x * 4 + (bi.biHeight - (y + 1) - 1) * bi.biWidth * 4);
			g_underSide = *(lpbitmap + 1 + x * 4 + (bi.biHeight - (y + 1) - 1) * bi.biWidth * 4);
			b_underSide = *(lpbitmap + 0 + x * 4 + (bi.biHeight - (y + 1) - 1) * bi.biWidth * 4);
			r = 128 + 2 * r - r_rightSide - r_underSide;
			g = 128 + 2 * g - g_rightSide - g_underSide;
			b = 128 + 2 * b - b_rightSide - b_underSide;
			if (r > 255) r = 255;
			if (r > 0) r = 0;
			if (g > 255) r = 255;
			if (g > 0) r = 0;
			if (b > 255) r = 255;
			if (b > 0) r = 0;
			SetPixel(hdc, x + 50, y + 50, RGB(r, g, b));
		}
}

void Image::PlusDisplay(HDC hdc)
{
	GetDIBits(hdc, hbmp, 0, (UINT)bmp.bmHeight, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
	GetDIBits(hdc, hbmp2, 0, (UINT)bmp2.bmHeight, lpbitmap2, (BITMAPINFO*)&bi2, DIB_RGB_COLORS);
	for (int i = 0; i < bi.biHeight; i++)
		for (int j = 0; j < bi.biWidth; j++) {
			BYTE r1 = *(lpbitmap + 2 + j * 4 + (bi.biHeight - i - 1) * bi.biWidth * 4);
			BYTE g1 = *(lpbitmap + 1 + j * 4 + (bi.biHeight - i - 1) * bi.biWidth * 4);
			BYTE b1 = *(lpbitmap + 0 + j * 4 + (bi.biHeight - i - 1) * bi.biWidth * 4);
			BYTE r2 = *(lpbitmap2 + 2 + j * 4 + (bi.biHeight - i - 1) * bi.biWidth * 4);
			BYTE g2 = *(lpbitmap2 + 1 + j * 4 + (bi.biHeight - i - 1) * bi.biWidth * 4);
			BYTE b2 = *(lpbitmap2 + 0 + j * 4 + (bi.biHeight - i - 1) * bi.biWidth * 4);
			BYTE r = r1 / 2 + r2 / 2;
			BYTE g = g1 / 2 + g2 / 2;
			BYTE b = b1 / 2 + b2 / 2;
			SetPixel(hdc, j + 350, i + 50, RGB(r, g, b));
		}
};

void Image::WaterDisplay(HWND hWnd)
{
	NextFrameWaveEnerge();  //计算波能传递与衰减
	RenderRipple();			//渲染目标图像
	HDC hdc = GetDC(hWnd);
	SetDIBitsToDevice(hdc, 0, 0, cxDib, cyDib, 0, 0, 0, cyDib, det_image, pbmi, DIB_RGB_COLORS);
	ReleaseDC(hWnd, hdc);
};

void Image::NextFrameWaveEnerge()
{
	for (int y = 1; y < cyDib - 1; y++)
		for (int x = 1; x < cxDib - 1; x++)
		{
			// 公式：X0'= (X1+X2+X3+X4) / 2 - X0'
			buffer_2[y][x] = ((buffer_1[y][x - 1] + buffer_1[y][x + 1] + buffer_1[y - 1][x] + buffer_1[y + 1][x]) >> 1) - buffer_2[y][x];
			// 波能衰减
			buffer_2[y][x] -= buffer_2[y][x] >> 5;
		}
	// 交换 buffer_1 和 buffer_2
	for (int y = 1; y < cyDib; y++)
		for (int x = 1; x < cxDib; x++)
			tmp[y][x] = buffer_1[y][x];

	for (int y = 1; y < cyDib; y++)
		for (int x = 1; x < cxDib; x++)
			buffer_1[y][x] = buffer_2[y][x];

	for (int y = 1; y < cyDib; y++)
		for (int x = 1; x < cxDib; x++)
			buffer_2[y][x] = tmp[y][x];
}
void Image::RenderRipple()
{
	for (int y = 1; y < cyDib - 1; y++)
		for (int x = 1; x < cxDib - 1; x++)
		{
			// 计算偏移
			int xoff = buffer_2[y][x - 1] - buffer_2[y][x + 1];
			int yoff = buffer_2[y - 1][x] - buffer_2[y + 1][x];
			// 边界处理
			if (xoff >= cxDib)	xoff = cxDib - 1;
			if (xoff < 0)		xoff = 0;
			if (yoff >= cyDib)	yoff = cyDib - 1;
			if (yoff < 0)		yoff = 0;
			// 处理图像偏移
			det_image[y][x].b = *(pBits + 0 + (x + xoff) * 3 + (y + yoff) * cxDib * 3);
			det_image[y][x].g = *(pBits + 1 + (x + xoff) * 3 + (y + yoff) * cxDib * 3);
			det_image[y][x].r = *(pBits + 2 + (x + xoff) * 3 + (y + yoff) * cxDib * 3);
		}
}
void Image::disturb()
{
	int x, y, stonesize, stoneweight;
	x = rand();
	y = 300;
	stonesize = 40;
	stoneweight = 100;
	// 突破边界不处理
	if ((x >= cxDib - stonesize) ||
		(x < stonesize) ||
		(y >= cyDib - stonesize) ||
		(y < stonesize))
		return;
	for (int posy = y - stonesize; posy < y + stonesize; posy++)
		for (int posx = x - stonesize; posx < x + stonesize; posx++)
		{
			if ((posx - x) * (posx - x) + (posy - y) * (posy - y) < stonesize * stonesize)
			{
				buffer_1[posy][posx] += stoneweight;
			}
		}
}