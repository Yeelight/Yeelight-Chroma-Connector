#include "stdafx.h"
#include "CPngCheckBox.h"


IMPLEMENT_DYNAMIC(CPngCheckBox, CButton)

BEGIN_MESSAGE_MAP(CPngCheckBox, CButton)
	ON_WM_CREATE()
	ON_CONTROL_REFLECT_EX(BN_CLICKED, OnClicked)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()


CPngCheckBox::CPngCheckBox()
{
	m_bChecked		= FALSE;
	m_crTextColor	= RGB(0, 0, 0);
	m_crBack		= RGB(255, 255, 255);
	m_bSetBack		= FALSE;
	m_bShowTextFrame = FALSE;
	m_crTextColor		= DEF_TEXT_COLOR;
	m_crTextFrameColor	= DEF_TEXT_FRAME_COLOR;
	m_ImageCheck = NULL;
	m_ImageBack = NULL;
}

CPngCheckBox::~CPngCheckBox()
{
	if (m_ImageCheck != NULL)
	{
		delete m_ImageCheck;
		m_ImageCheck = NULL;
	}
	if (m_ImageBack != NULL)
	{
		delete m_ImageBack;
		m_ImageBack = NULL;
	}
}


void CPngCheckBox::PreSubclassWindow()
{
	ModifyStyle(0, BS_OWNERDRAW);
	CButton::PreSubclassWindow();
}

void CPngCheckBox::Init(UINT nBkImg, UINT nCkImg) {
	m_ImageCheck = ImageFromResource(AfxGetResourceHandle(), nCkImg, L"PNG");
	m_ImageBack	 = ImageFromResource(AfxGetResourceHandle(), nBkImg, L"PNG");

	if (m_ImageBack != NULL) {
		m_bSetBack = TRUE;
	}

	CRect rcButton;
	rcButton = CRect(0, 0, m_ImageCheck->GetWidth(), m_ImageCheck->GetHeight());

	//GetWindowRect(&rcButton);

	//SetWindowPos(NULL, 0, 0, rcButton.Width(), rcButton.Height(), SWP_NOACTIVATE | SWP_NOMOVE);
}
HBRUSH CPngCheckBox::CtlColor(CDC* pDC, UINT nCtlColor)
{
	pDC->SetBkMode(TRANSPARENT);
	return (HBRUSH)GetStockObject(NULL_BRUSH);
}
void CPngCheckBox::SetFontType(int fontSize, CString fontType) {
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfHeight = fontSize;
	_tcsncpy_s(lf.lfFaceName, LF_FACESIZE, fontType, fontType.GetLength());
	VERIFY(m_font.CreateFontIndirect(&lf));
}
void CPngCheckBox::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	CDC dc;
	dc.Attach(lpDIS->hDC);
	CRect rcItem(lpDIS->rcItem);

	CRect ClientRect;
	GetWindowRect(&ClientRect);

	if (m_ImageCheck != NULL && m_ImageBack != NULL)
	{
		CRect rcButton;
		rcButton = CRect(0, 0, ClientRect.Width(), ClientRect.Height());
		if (m_bChecked && IsWindowEnabled())
		{
			//rcButton = CRect(0, 0, m_ImageCheck->GetWidth(), m_ImageCheck->GetHeight());
			dc.SetBkMode(TRANSPARENT);
			Graphics graph(dc.GetSafeHdc());
			//graph.DrawImage(m_ImageCheck, 0, 0, rcButton.left, rcButton.top, rcButton.Width(), rcButton.Height(), UnitPixel);
			graph.DrawImage(m_ImageCheck, RectF(0, 0, rcButton.Width(), rcButton.Height()), 0, 0, m_ImageCheck->GetWidth(), m_ImageCheck->GetHeight(), UnitPixel);
			graph.ReleaseHDC(dc.GetSafeHdc());
		}
		else if (!m_bChecked && IsWindowEnabled())
		{
			//rcButton = CRect(0, 0, m_ImageBack->GetWidth(), m_ImageBack->GetHeight());
			dc.SetBkMode(TRANSPARENT);
			Graphics graph(dc.GetSafeHdc());
			int W = m_ImageBack->GetWidth();
			int H = m_ImageBack->GetHeight();
			//graph.DrawImage(m_ImageBack, 0, 0, rcButton.left, rcButton.top, rcButton.Width(), rcButton.Height(), UnitPixel);
			graph.DrawImage(m_ImageBack, RectF(0, 0, rcButton.Width(), rcButton.Height()), 0, 0, m_ImageBack->GetWidth(), m_ImageBack->GetHeight(), UnitPixel);
			graph.ReleaseHDC(dc.GetSafeHdc());
		}

		CString szText;
		GetWindowText(szText);
		dc.SetBkMode(TRANSPARENT);
		CFont* oldFont = dc.SelectObject(m_font.GetSafeHandle() ? &m_font : GetFont());

		CRect txtRc = { 0,0,rcButton.Width() ,rcButton.Height() };

		DrawTextString(&dc, szText, m_crTextColor, m_crTextFrameColor, txtRc);
		dc.SelectObject(oldFont);
	}
	dc.Detach();
}

void CPngCheckBox::SetCheckBoxTextColor(COLORREF crTextColor)
{
	m_crTextColor = crTextColor;
}

bool CPngCheckBox::SetTextColor(COLORREF crTextColor, COLORREF crTextFrameColor, bool bShowFrame)
{
	m_crTextColor = crTextColor;
	m_bShowTextFrame = bShowFrame;
	m_crTextFrameColor = crTextFrameColor;

	if (GetSafeHwnd()) Invalidate(FALSE);
	return true;
}
Image * CPngCheckBox::ImageFromResource(HINSTANCE hInstance, UINT uImgID, LPCTSTR lpType)
{
	HRSRC hResInfo = ::FindResource(hInstance, MAKEINTRESOURCE(uImgID), lpType);
	if (hResInfo == NULL)
		return NULL; //fail
	DWORD dwSize;
	dwSize = SizeofResource(hInstance, hResInfo); //get resource size(bytes)
	HGLOBAL hResData;
	hResData = ::LoadResource(hInstance, hResInfo);
	if (hResData == NULL)
		return NULL; //fail
	HGLOBAL hMem;
	hMem = ::GlobalAlloc(GMEM_MOVEABLE, dwSize);
	if (hMem == NULL) {
		::FreeResource(hResData);
		return NULL;
	}
	LPVOID lpResData, lpMem;
	lpResData = ::LockResource(hResData);
	lpMem = ::GlobalLock(hMem);
	::CopyMemory(lpMem, lpResData, dwSize); //copy memory
	::GlobalUnlock(hMem);
	::FreeResource(hResData); //free memory

	IStream *pStream;
	HRESULT hr;
	hr = ::CreateStreamOnHGlobal(hMem, TRUE, &pStream);//create stream object
	Image *pImage = NULL;
	if (SUCCEEDED(hr)) {
		pImage = Image::FromStream(pStream);//get GDI+ pointer
		//pStream->Release();
	}
	::GlobalFree(hMem);
	return pImage;
}

BOOL CPngCheckBox::OnClicked()
{
	if (!m_bChecked)
		m_bChecked = TRUE;
	else
		m_bChecked = FALSE;

	PaintParent();
	return FALSE;
}

BOOL CPngCheckBox::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}
void CPngCheckBox::PaintParent()
{
	CRect   rect;
	GetWindowRect(&rect);
	GetParent()->ScreenToClient(&rect);
	GetParent()->InvalidateRect(&rect);
}

void CPngCheckBox::DrawTextString(CDC * pDC, LPCTSTR pszString, COLORREF crText, COLORREF crFrame, LPRECT lpRect)
{
	int nStringLength = lstrlen(pszString);
	int nXExcursion[8] = { 1,1,1,0,-1,-1,-1,0 };
	int nYExcursion[8] = { -1,0,1,1,1,0,-1,-1 };

	CRect rcDraw;
	if (m_bShowTextFrame)
	{
		pDC->SetTextColor(crFrame);
		for (int i = 0; i < sizeof(nXExcursion) / sizeof(nXExcursion[0]); ++i)
		{
			rcDraw.CopyRect(lpRect);
			rcDraw.OffsetRect(nXExcursion[i], nYExcursion[i]);
			pDC->DrawText(pszString, nStringLength, &rcDraw, DT_VCENTER | DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
		}
	}

	rcDraw.CopyRect(lpRect);
	pDC->SetTextColor(crText);
	pDC->DrawText(pszString, nStringLength, &rcDraw, DT_VCENTER | DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
	return;
}

void CPngCheckBox::DrawTextString(CDC * pDC, LPCTSTR pszString, COLORREF crText, COLORREF crFrame, int nXPos, int nYPos)
{

	int nStringLength = lstrlen(pszString);
	int nXExcursion[8] = { 1,1,1,0,-1,-1,-1,0 };
	int nYExcursion[8] = { -1,0,1,1,1,0,-1,-1 };
	pDC->SetTextColor(crFrame);
	for (int i = 0; i < sizeof(nXExcursion) / sizeof(nXExcursion[0]); i++)
	{
		pDC->TextOut(nXPos + nXExcursion[i], nYPos + nYExcursion[i], pszString, nStringLength);
	}

	pDC->SetTextColor(crText);
	pDC->TextOut(nXPos, nYPos, pszString, nStringLength);
	return;
}
