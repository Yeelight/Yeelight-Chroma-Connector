#include "stdafx.h"  
#include "Public.h"  
 
 
bool LoadImageFromResourse(CImage* pImg, UINT nImgID, LPCTSTR lpImgType)
{
	if (pImg == NULL)
	{
		return FALSE;
	}
	pImg->Destroy();
 
	
	HRSRC hRsrc = ::FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(nImgID), lpImgType);
	if (hRsrc == NULL)
	{
		return false;
	}
 
	
	HGLOBAL hImgData = ::LoadResource(AfxGetResourceHandle(), hRsrc);
	if (hImgData == NULL)
	{
		::FreeResource(hImgData);
		return false;
	}
 
 
	LPVOID lpVoid = ::LockResource(hImgData);                           
	LPSTREAM pStream = NULL;
	DWORD dwSize = ::SizeofResource(AfxGetResourceHandle(), hRsrc);
	HGLOBAL hNew = ::GlobalAlloc(GHND, dwSize);
	LPBYTE lpByte = (LPBYTE)::GlobalLock(hNew);
	::memcpy(lpByte, lpVoid, dwSize);
	::GlobalUnlock(hNew);                                              
 
	HRESULT ht = ::CreateStreamOnHGlobal(hNew, TRUE, &pStream);
	if (ht != S_OK)
	{
		GlobalFree(hNew);
	}
	else
	{
		
		pImg->Load(pStream);
		GlobalFree(hNew);
	}
 
	
	::FreeResource(hImgData);
	return true;
}
 
bool LoadPicture(CImage& bmp, UINT nImgID, LPCTSTR lpImgType)           
{
	LoadImageFromResourse(&bmp, nImgID, lpImgType);                 
 
	if (bmp.IsNull())
	{
		return false;
	}
	if (bmp.GetBPP() == 32)                                              
	{
		for (int i = 0; i < bmp.GetWidth(); i++)
		{
			for (int j = 0; j < bmp.GetHeight(); j++)
			{
				byte* pByte = (byte*)bmp.GetPixelAddress(i, j);
				pByte[0] = pByte[0] * pByte[3] / 255;
				pByte[1] = pByte[1] * pByte[3] / 255;
				pByte[2] = pByte[2] * pByte[3] / 255;
			}
		}
	}
 
	return true;
}