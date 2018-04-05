/// by fanxiushu 2018-03-08

#pragma once

struct frame_t
{
	char* buffer;
	int   length;
	int   width;
	int   height;
	int   delay_msec; ///ͣ��ʱ��
					  ////
	void* param;      ///
};

typedef int(*FRAME_CALLBACK)(frame_t* frame);

//////
//
extern HINSTANCE g_hInstance;

int VCam_Frame_Callback(frame_t* frame); //�ڴ˺�������д��Ƶ����

