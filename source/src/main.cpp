////by fanxiushu 2018-03-08

#include <Windows.h>
#include "stdio.h"
#include "vcamera.h"

HINSTANCE g_hInstance;
////
BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	g_hInstance = hInstance;
	if (dwReason == DLL_PROCESS_ATTACH) {
		////
		DisableThreadLibraryCalls(hInstance);
	}
	
	////
	return TRUE;
}

//////
//������ҵ� ����USB����ͷ�����ӹ���copy

//// RGB -> YUV �������ѯ���㷨
void rgb24_yuy2(void* rgb, void* yuy2, int width, int height)
{
	int R1, G1, B1, R2, G2, B2, Y1, U1, Y2, V1;
	unsigned char* pRGBData = (unsigned char *)rgb;
	unsigned char* pYUVData = (unsigned char *)yuy2;

	for (int i = 0; i<height; ++i)
	{
		for (int j = 0; j<width / 2; ++j)
		{
			B1 = *(pRGBData + (height - i - 1)*width * 3 + j * 6);
			G1 = *(pRGBData + (height - i - 1)*width * 3 + j * 6 + 1);
			R1 = *(pRGBData + (height - i - 1)*width * 3 + j * 6 + 2);
			B2 = *(pRGBData + (height - i - 1)*width * 3 + j * 6 + 3);
			G2 = *(pRGBData + (height - i - 1)*width * 3 + j * 6 + 4);
			R2 = *(pRGBData + (height - i - 1)*width * 3 + j * 6 + 5);

			Y1 = ((66 * R1 + 129 * G1 + 25 * B1 + 128) >> 8) + 16;
			U1 = (((-38 * R1 - 74 * G1 + 112 * B1 + 128) >> 8) + ((-38 * R2 - 74 * G2 + 112 * B2 + 128) >> 8)) / 2 + 128;
			Y2 = ((66 * R2 + 129 * G2 + 25 * B2 + 128) >> 8) + 16;
			V1 = (((112 * R1 - 94 * G1 - 18 * B1 + 128) >> 8) + ((112 * R2 - 94 * G2 - 18 * B2 + 128) >> 8)) / 2 + 128;

			*(pYUVData + i*width * 2 + j * 4) = max(min(Y1, 255), 0);
			*(pYUVData + i*width * 2 + j * 4 + 1) = max(min(U1, 255), 0);
			*(pYUVData + i*width * 2 + j * 4 + 2) = max(min(Y2, 255), 0);
			*(pYUVData + i*width * 2 + j * 4 + 3) = max(min(V1, 255), 0);
		}
	}
}

////////////////////
struct vcam_param
{
	HBITMAP hbmp;
	HDC hdc;
	void* rgb_data;

	int width;
	int height;
	const char* text;
	int i_color;
	int           clr_flip;
	int           i_size;
	int           sz_flip;
};
int create_dib(vcam_param* p, int w, int h)
{
	if (p->width == w && p->height == h && p->hbmp) return 0;
	////
	if (p->hbmp)DeleteObject(p->hbmp);
	if (p->hdc)DeleteDC(p->hdc);
	p->hbmp = 0; p->hdc = 0;
	p->hdc = CreateCompatibleDC(NULL);

	BITMAPINFOHEADER bi; memset(&bi, 0, sizeof(bi));
	bi.biSize = sizeof(bi);
	bi.biWidth = w;
	bi.biHeight = h;
	bi.biPlanes = 1;
	bi.biBitCount = 24; //RGB
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;


	p->hbmp = CreateDIBSection(p->hdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, &p->rgb_data, NULL, 0);
	if (!p->hbmp) {
		p->rgb_data = 0;
		printf("CreateDIBSection err=%d\n", GetLastError());
		return -1;
	}
	SelectObject(p->hdc, p->hbmp); ///
								   ////
	p->width = w;
	p->height = h;

	p->clr_flip = 1;
	p->i_color = 0;

	p->i_size = 20;
	p->sz_flip = 2; ///

	return 0;
}
void draw_text(vcam_param* p)
{
	if (!p->hbmp)return;
	////
	int len = p->width*p->height * 3; //
	memset(p->rgb_data, p->i_color, len); /// ����ɫ����
	p->i_color += p->clr_flip;
	if (p->i_color <= 0 || p->i_color >= 245) p->clr_flip = -p->clr_flip; ///

																		  ////
	p->i_size += p->sz_flip;
	if (p->i_size <= 10 || p->i_size >= 60) p->sz_flip = -p->sz_flip;

	HFONT font = CreateFont(p->i_size, p->i_size * 3 / 4, 1, 0, 800, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, 0, 0, NULL);
	HFONT old = (HFONT)SelectObject(p->hdc, font);
	RECT rc = { 0,0,p->width, p->height };
	SetBkMode(p->hdc, TRANSPARENT);
	SetTextColor(p->hdc, RGB(255, 255 - p->i_color % 255, (p->i_color / 3 + 20) % 255));
	DrawText(p->hdc, p->text, strlen(p->text), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	SelectObject(p->hdc, old);
	DeleteObject(font);
}

static vcam_param param;
static bool p_init = false;
///�ڴ˺�������дÿ֡ͼ��
int VCam_Frame_Callback(frame_t* frame)
{
//	memset(frame->buffer, 'd', frame->length);
	vcam_param* p = (vcam_param*)&param;
	///
	if (!p_init) {
		p_init = true;
		memset(&param, 0, sizeof(param));
		param.text = "Fanxiushu DirectShow VCamera";
	}

	create_dib(p, frame->width, frame->height); ///

	draw_text(p);

	if (p->rgb_data) rgb24_yuy2(p->rgb_data, frame->buffer, frame->width, frame->height);

	frame->delay_msec = 33; ///ÿ֡��ͣ��ʱ�䣬 ���룬 

	return 0;
}

