#pragma once  
 
#include "stdafx.h"  
#include <atlimage.h>  
#include "gdiplus.h"
#pragma comment( lib, "gdiplus.lib" )  
using namespace Gdiplus;
//button state
enum
{
	CTRL_NOFOCUS = 0x01,            //normal
	CTRL_FOCUS,                     //mousemove  
	CTRL_SELECTED,                  //buttondown  
	CTRL_DISABLE,                   //
};
 
//picture type
enum
{
	BTN_IMG_1 = 1,                  //  
	BTN_IMG_3 = 3,                  //
	BTN_IMG_4 = 4,                  //
};
 
//button type
enum
{
	BTN_TYPE_NORMAL = 0x10,         //
	BTN_TYPE_MENU,                  //
	BTN_TYPE_STATIC,                //
};
 

bool LoadImageFromResourse(CImage* pImg, UINT nImgID, LPCTSTR lpImgType);
bool LoadPicture(CImage& bmp, UINT nImgID, LPCTSTR lpImgType = _T("PNG"));