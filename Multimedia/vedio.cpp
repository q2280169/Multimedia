#include "vedio.h"
#include <stdio.h>

Vedio::Vedio()
{
	filename = (char*)"foreman.yuv";
	filename2 = (char*)"mother.yuv";
    filenameback = (char*)"background.bmp";

    fopen_s(&ifp, filename, "r");
    fread(mybuf, 45621248, 1, ifp);
    pBity = mybuf;
    pBitu = mybuf + 352 * 288;
    pBitv = mybuf + 352 * 288 + 176 * 144;

    fopen_s(&ifp2, filename2, "r");
    fread(mybuf2, 45621248, 1, ifp2);
    pBity2 = mybuf2;
    pBitu2 = mybuf2 + 352 * 288;
    pBitv2 = mybuf2 + 352 * 288 + 176 * 144;

    //打开图像文件
    fopen_s(&ifpback, filenameback, "r");
    fread(mybufback, 307200, 1, ifpback);
    pbmfh = (BITMAPFILEHEADER*)mybufback;
    pbmi = (BITMAPINFO*)(pbmfh + 1);
    pbits = (BYTE*)pbmfh + pbmfh->bfOffBits;
    cxDib = pbmi->bmiHeader.biWidth;
    cyDib = pbmi->bmiHeader.biHeight;

    // 高斯模板
    gaosi[0][0] = 1;gaosi[0][1] = 5;gaosi[0][2] = 8;gaosi[0][3] = 5;gaosi[0][4] = 1;
    gaosi[1][0] = 5;gaosi[1][1] = 21;gaosi[1][2] = 34;gaosi[1][3] = 21;gaosi[1][4] = 5;
    gaosi[2][0] = 8;gaosi[2][1] = 34;gaosi[2][2] = 56;gaosi[2][3] = 34;gaosi[2][4] = 8;
    gaosi[3][0] = 5;gaosi[3][1] = 21;gaosi[3][2] = 34;gaosi[3][3] = 21;gaosi[3][4] = 5;
    gaosi[4][0] = 1;gaosi[4][1] = 5;gaosi[4][2] = 8;gaosi[4][3] = 5;gaosi[4][4] = 1;
   
    // 波能初始化
    for (int y = 0; y < IMAGE_HIGHT; y++)
        for (int x = 0; x < IMAGE_WIDTH; x++)
        {
            buffer_1[y][x] = 0;
            buffer_2[y][x] = 0;
        }
}

void Vedio::bofang(HWND hWnd)
{
    n = n + 1;
    if (n > 299) n = 0;
    pBity = pBity + (352 * 288 + 2 * (176 * 144)) * n;
    pBitu = pBity + 352 * 288;
    pBitv = pBitu + 176 * 144;
    for (int i = 0; i < 144; i++)
        for (int j = 0; j < 176; j++)
        {
            u[i][j] = *(pBitu + j + 176 * (i));
            v[i][j] = *(pBitv + j + 176 * (i));
        }
    for (int i = 0; i < 288; i++)
        for (int j = 0; j < 352; j++)
        {
            y[i][j] = *(pBity + j + (i) * 352);
            int r = (298 * (y[i][j] - 16) + 409 * (v[i / 2][j / 2] - 128) + 128) >> 8;
            if (r < 0) r = 0;
            if (r > 255) r = 255;
            int g = (298 * (y[i][j] - 16) - 100 * (u[i / 2][j / 2] - 128) - 208 * (v[i / 2][j / 2] - 128) + 128) >> 8;
            if (g < 0) g = 0;
            if (g > 255) g = 255;
            int b = (298 * (y[i][j] - 16) + 516 * (u[i / 2][j / 2] - 128) + 128) >> 8;
            if (b < 0) b = 0;
            if (b > 255) b = 255;
            det_image[288 - i - 1][j].r = r;
            det_image[288 - i - 1][j].g = g;
            det_image[288 - i - 1][j].b = b;

        }
    // 显示当前帧
    HDC hdc = GetDC(hWnd);
    SetDIBitsToDevice(hdc, 30, 20, 352, 288, 0, 0, 0, 288, det_image, pbmi, DIB_RGB_COLORS);
    ReleaseDC(hWnd, hdc);
    // 恢复指向图像数据首字节
    pBity = mybuf;
}

void Vedio::zimu(HWND hWnd)
{
    n = n + 1;
    if (n > 299) n = 0;
    m = m + 1;
    if (m > 60) m = 0;

    pBity = pBity + (352 * 288 + 2 * (176 * 144)) * n;
    pBitu = pBity + 352 * 288;
    pBitv = pBitu + 176 * 144;
    // 读取一帧的 uv 值
    for (int i = 0; i < 144; i++)
        for (int j = 0; j < 176; j++)
        {
            u[i][j] = *(pBitu + j + 176 * (i));
            v[i][j] = *(pBitv + j + 176 * (i));
        }
    // 读取y值，并立即将yuv转成rgb值
    for (int i = 0; i < 288; i++)
        for (int j = 0; j < 352; j++)
        {
            // 读取y
            y[i][j] = *(pBity + j + (i) * 352);
            // 转码
            int r = (298 * (y[i][j] - 16) + 409 * (v[i / 2][j / 2] - 128) + 128) >> 8;
            if (r < 0) r = 0;
            if (r > 255) r = 255;
            int g = (298 * (y[i][j] - 16) - 100 * (u[i / 2][j / 2] - 128) - 208 * (v[i / 2][j / 2] - 128) + 128) >> 8;
            if (g < 0) g = 0;
            if (g > 255) g = 255;
            int b = (298 * (y[i][j] - 16) + 516 * (u[i / 2][j / 2] - 128) + 128) >> 8;
            if (b < 0) b = 0;
            if (b > 255) b = 255;
            // 取字幕图标图像的像素值
            int rback = *(pbits + 2 + j * 3 + (cyDib - i - 1) * cxDib * 3);
            int gback = *(pbits + 1 + j * 3 + (cyDib - i - 1) * cxDib * 3);
            int bback = *(pbits + 0 + j * 3 + (cyDib - i - 1) * cxDib * 3);
            if (rback == 0 && gback == 0 && bback == 0)
            {
                det_image[288 - i - 1][j].r = r;
                det_image[288 - i - 1][j].g = g;
                det_image[288 - i - 1][j].b = b;
            }
            else
            {
                // 存储到目标图像矩阵
                if (j < (m / 60.0 * 352))
                {
                    det_image[288 - i - 1][j].r = rback;
                    det_image[288 - i - 1][j].g = gback;
                    det_image[288 - i - 1][j].b = bback;
                }
                else
                {
                    det_image[288 - i - 1][j].r = 255;
                    det_image[288 - i - 1][j].g = 255;
                    det_image[288 - i - 1][j].b = 255;
                }
            }
        }
    // 显示当前帧
    HDC hdc = GetDC(hWnd);
    SetDIBitsToDevice(hdc, 300, 20, 352, 288, 0, 0, 0, 288, det_image, pbmi, DIB_RGB_COLORS);
    ReleaseDC(hWnd, hdc);
    // 恢复指向图像数据首字节
    pBity = mybuf;
}

void Vedio::danrudanchu(HWND hWnd)
{
    n = n + 1;
    if (n > 299) n = 0;
    pBity = pBity + (352 * 288 + 2 * (176 * 144)) * n;
    pBitu = pBity + 352 * 288;
    pBitv = pBitu + 176 * 144;
    pBity2 = pBity2 + (352 * 288 + 2 * (176 * 144)) * n;
    pBitu2 = pBity2 + 352 * 288;
    pBitv2 = pBitu2 + 176 * 144;
    // 读取一帧的 uv 值
    for (int i = 0; i < 144; i++)
        for (int j = 0; j < 176; j++)
        {
            u[i][j] = *(pBitu + j + 176 * (i));
            v[i][j] = *(pBitv + j + 176 * (i));
            u2[i][j] = *(pBitu2 + j + 176 * (i));
            v2[i][j] = *(pBitv2 + j + 176 * (i));
        }
    // 读取y值，并立即将yuv转成rgb值
    for (int i = 0; i < 288; i++)
        for (int j = 0; j < 352; j++)
        {
            // 读取y
            y[i][j] = *(pBity + j + (i) * 352);
            y2[i][j] = *(pBity2 + j + (i) * 352);
            // 转码
            int r = (298 * (y[i][j] - 16) + 409 * (v[i / 2][j / 2] - 128) + 128) >> 8;
            if (r < 0) r = 0;
            if (r > 255) r = 255;
            int g = (298 * (y[i][j] - 16) - 100 * (u[i / 2][j / 2] - 128) - 208 * (v[i / 2][j / 2] - 128) + 128) >> 8;
            if (g < 0) g = 0;
            if (g > 255) g = 255;
            int b = (298 * (y[i][j] - 16) + 516 * (u[i / 2][j / 2] - 128) + 128) >> 8;
            if (b < 0) b = 0;
            if (b > 255) b = 255;
            int r2 = (298 * (y2[i][j] - 16) + 409 * (v2[i / 2][j / 2] - 128) + 128) >> 8;
            if (r2 < 0) r2 = 0;
            if (r2 > 255) r2 = 255;
            int g2 = (298 * (y2[i][j] - 16) - 100 * (u2[i / 2][j / 2] - 128) - 208 * (v2[i / 2][j / 2] - 128) + 128) >> 8;
            if (g2 < 0) g2 = 0;
            if (g2 > 255) g2 = 255;
            int b2 = (298 * (y2[i][j] - 16) + 516 * (u2[i / 2][j / 2] - 128) + 128) >> 8;
            if (b2 < 0) b2 = 0;
            if (b2 > 255) b2 = 255;
            // 定义透明度参数
            double para = n / 300.0;
            // 存储到目标图像矩阵
            det_image[288 - i - 1][j].r = r * (1 - para) + r2 * para;
            det_image[288 - i - 1][j].g = g * (1 - para) + g2 * para;
            det_image[288 - i - 1][j].b = b * (1 - para) + b2 * para;
        }
    // 显示当前帧
    HDC hdc = GetDC(hWnd);
    SetDIBitsToDevice(hdc, 500, 20, 352, 288, 0, 0, 0, 288, det_image, pbmi, DIB_RGB_COLORS);
    ReleaseDC(hWnd, hdc);
    // 恢复指向图像数据首字节
    pBity = mybuf;
    pBity2 = mybuf2;
}

void Vedio::pinjie(HWND hWnd)
{
    n = n + 1;
    if (n > 299) n = 0;
    pBity = pBity + (352 * 288 + 2 * (176 * 144)) * n;
    pBitu = pBity + 352 * 288;
    pBitv = pBitu + 176 * 144;
    pBity2 = pBity2 + (352 * 288 + 2 * (176 * 144)) * n;
    pBitu2 = pBity2 + 352 * 288;
    pBitv2 = pBitu2 + 176 * 144;
    // 读取一帧的 uv 值
    for (int i = 0; i < 144; i++)
        for (int j = 0; j < 176; j++)
        {
            u[i][j] = *(pBitu + j + 176 * (i));
            v[i][j] = *(pBitv + j + 176 * (i));
            u2[i][j] = *(pBitu2 + j + 176 * (i));
            v2[i][j] = *(pBitv2 + j + 176 * (i));
        }
    // 读取y值，并立即将yuv转成rgb值
    for (int i = 0; i < 288; i++)
        for (int j = 0; j < 352; j++)
        {
            // 读取y
            y[i][j] = *(pBity + j + (i) * 352);
            y2[i][j] = *(pBity2 + j + (i) * 352);
            // 转码
            int r = (298 * (y[i][j] - 16) + 409 * (v[i / 2][j / 2] - 128) + 128) >> 8;
            if (r < 0) r = 0;
            if (r > 255) r = 255;
            int g = (298 * (y[i][j] - 16) - 100 * (u[i / 2][j / 2] - 128) - 208 * (v[i / 2][j / 2] - 128) + 128) >> 8;
            if (g < 0) g = 0;
            if (g > 255) g = 255;
            int b = (298 * (y[i][j] - 16) + 516 * (u[i / 2][j / 2] - 128) + 128) >> 8;
            if (b < 0) b = 0;
            if (b > 255) b = 255;
            int r2 = (298 * (y2[i][j] - 16) + 409 * (v2[i / 2][j / 2] - 128) + 128) >> 8;
            if (r2 < 0) r2 = 0;
            if (r2 > 255) r2 = 255;
            int g2 = (298 * (y2[i][j] - 16) - 100 * (u2[i / 2][j / 2] - 128) - 208 * (v2[i / 2][j / 2] - 128) + 128) >> 8;
            if (g2 < 0) g2 = 0;
            if (g2 > 255) g2 = 255;
            int b2 = (298 * (y2[i][j] - 16) + 516 * (u2[i / 2][j / 2] - 128) + 128) >> 8;
            if (b2 < 0) b2 = 0;
            if (b2 > 255) b2 = 255;
            // 定义透明度参数
            double para = n / 300.0;
            // 存储到目标图像矩阵
            det_image2[288 - i - 1][j].r = r;
            det_image3[288 - i - 1][j].r = r2;
            det_image2[288 - i - 1][j].g = g;
            det_image3[288 - i - 1][j].g = g2;
            det_image2[288 - i - 1][j].b = b;
            det_image3[288 - i - 1][j].b = b2;
        }
    // 显示当前帧
    HDC hdc = GetDC(hWnd);
    SetDIBitsToDevice(hdc, 400, 20, 352, 288, 0, 0, 0, 288, det_image2, pbmi, DIB_RGB_COLORS);
    SetDIBitsToDevice(hdc, 400 + 352, 20, 352, 288, 0, 0, 0, 288, det_image3, pbmi, DIB_RGB_COLORS);
    ReleaseDC(hWnd, hdc);
    // 恢复指向图像数据首字节
    pBity = mybuf;
    pBity2 = mybuf2;
}

void Vedio::water(HWND hWnd)
{
    n = n + 1;
    if (n > 299) n = 0;
    pBity = pBity + (352 * 288 + 2 * (176 * 144)) * n;
    pBitu = pBity + 352 * 288;
    pBitv = pBitu + 176 * 144;
    for (int i = 0; i < 144; i++)
        for (int j = 0; j < 176; j++)
        {
            u[i][j] = *(pBitu + j + 176 * (i));
            v[i][j] = *(pBitv + j + 176 * (i));
        }
    for (int i = 0; i < 288; i++)
        for (int j = 0; j < 352; j++)
        {
            y[i][j] = *(pBity + j + (i) * 352);
            int r = (298 * (y[i][j] - 16) + 409 * (v[i / 2][j / 2] - 128) + 128) >> 8;
            if (r < 0) r = 0;
            if (r > 255) r = 255;
            int g = (298 * (y[i][j] - 16) - 100 * (u[i / 2][j / 2] - 128) - 208 * (v[i / 2][j / 2] - 128) + 128) >> 8;
            if (g < 0) g = 0;
            if (g > 255) g = 255;
            int b = (298 * (y[i][j] - 16) + 516 * (u[i / 2][j / 2] - 128) + 128) >> 8;
            if (b < 0) b = 0;
            if (b > 255) b = 255;
            det_image[288 - i - 1][j].r = r;
            det_image[288 - i - 1][j].g = g;
            det_image[288 - i - 1][j].b = b;

        }
    // 恢复指向图像数据首字节
    pBity = mybuf;

    NextFrameWaveEnerge();
    RenderRipple();

    // 显示当前帧
    HDC hdc = GetDC(hWnd);
    SetDIBitsToDevice(hdc, 30, 20, 352, 288, 0, 0, 0, 288, det_image, pbmi, DIB_RGB_COLORS);
    ReleaseDC(hWnd, hdc);
}

void Vedio::NextFrameWaveEnerge()
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

void Vedio::RenderRipple()
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
            det_image[y][x].b = det_image[y + yoff][x + xoff].b;
            det_image[y][x].g = det_image[y + yoff][x + xoff].g;
            det_image[y][x].r = det_image[y + yoff][x + xoff].r;
        }
}

void Vedio::disturb()
{
    int x, y, stonesize, stoneweight;
    x = 100;
    y = 100;
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