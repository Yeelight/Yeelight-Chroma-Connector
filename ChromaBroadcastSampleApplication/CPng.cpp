// PngButton.cpp : implementation file

#include "stdafx.h"
#include "CPng.h"

CPng::CPng()
{
	m_bTracked	= FALSE;
	m_bMenuOn	= FALSE;
	m_nImgPart	= 0;
	m_pImage	= NULL;
	m_nState			= CTRL_NOFOCUS;
	m_nBtnType			= BTN_TYPE_NORMAL;
	m_bShowTextFrame	= FALSE;
	m_crTextColor		= DEF_TEXT_COLOR1;
	m_crTextFrameColor	= DEF_TEXT_FRAME_COLOR;
	m_cursor = ::LoadCursor(NULL, IDC_ARROW);
}

CPng::~CPng()
{
	if (m_pImage != NULL)
	{
		delete m_pImage;
		m_pImage = NULL;
	}
}

void CPng::Init(UINT nImg, int nPartNum, UINT nBtnType)
{
	m_pImage = ImageFromResource(AfxGetResourceHandle(), nImg, L"PNG");
	m_nBtnType = nBtnType;
	m_nImgPart = nPartNum;

	if (m_pImage == NULL)
		return;

	CRect rcButton;

	if (m_nImgPart == BTN_IMG_1)
		rcButton = CRect(0, 0, m_pImage->GetWidth(), m_pImage->GetHeight());
	else if (m_nImgPart == BTN_IMG_3)
		rcButton = CRect(0, 0, m_pImage->GetWidth() / 3, m_pImage->GetHeight());
	else if (m_nImgPart == BTN_IMG_4)
		rcButton = CRect(0, 0, m_pImage->GetWidth() / 4, m_pImage->GetHeight());
	else
		return;

	//SetWindowPos(NULL, 0, 0, rcButton.Width(), rcButton.Height(), SWP_NOACTIVATE | SWP_NOMOVE);
}

BEGIN_MESSAGE_MAP(CPng, CButton)
	//{{AFX_MSG_MAP(CPng)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPng message handlers

void CPng::OnPaint()
{
	CButton::OnPaint();
}

void CPng::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (!IsWindowEnabled())
		m_nState = CTRL_DISABLE;
	CDC dc;
	dc.Attach(lpDrawItemStruct->hDC);
	ShowImage(&dc, m_pImage, m_nState);
	dc.Detach();
}

bool CPng::ShowImage(CDC* pDC, Image* pImage, UINT nState)
{
	bool bSuc = false;

	CRect ClientRect;
	GetWindowRect(&ClientRect);

	if (pImage != NULL)
	{
		CRect rcButton;
		if (m_nImgPart == BTN_IMG_1)
			//rcButton = CRect(0, 0, m_pImage->GetWidth(), m_pImage->GetHeight());
			rcButton = CRect(0, 0, ClientRect.Width(), ClientRect.Height());
		else if (m_nImgPart == BTN_IMG_3)
		{
			if (nState == CTRL_NOFOCUS)
				rcButton = CRect(0, 0, m_pImage->GetWidth() / 3, m_pImage->GetHeight());
			else if (nState == CTRL_FOCUS)
				rcButton = CRect(m_pImage->GetWidth() / 3, 0, m_pImage->GetWidth() / 3 * 2, m_pImage->GetHeight());
			else if (nState == CTRL_SELECTED)
				rcButton = CRect(m_pImage->GetWidth() / 3 * 2, 0, m_pImage->GetWidth(), m_pImage->GetHeight());
			else
				return false;
		}
		else if (m_nImgPart == BTN_IMG_4)
		{
			if (nState == CTRL_NOFOCUS)
				rcButton = CRect(0, 0, m_pImage->GetWidth() / 4, m_pImage->GetHeight());
			else if (nState == CTRL_FOCUS)
				rcButton = CRect(m_pImage->GetWidth() / 4, 0, m_pImage->GetWidth() / 4 * 2, m_pImage->GetHeight());
			else if (nState == CTRL_SELECTED)
				rcButton = CRect(m_pImage->GetWidth() / 4 * 2, 0, m_pImage->GetWidth() / 4 * 3, m_pImage->GetHeight());
			else if (nState == CTRL_DISABLE)
				rcButton = CRect(m_pImage->GetWidth() / 4 * 3, 0, m_pImage->GetWidth(), m_pImage->GetHeight());
			else
				return false;
		}
		else
			return false;


		pDC->SetBkMode(TRANSPARENT);
		Graphics graph(pDC->GetSafeHdc());
		//graph.DrawImage(pImage, RectF(0,0, rcButton.Width(), rcButton.Height()), 0, 0, rcButton.Width(), rcButton.Height(), UnitPixel);
		graph.DrawImage(pImage, RectF(0, 0, rcButton.Width(), rcButton.Height()), 0, 0, m_pImage->GetWidth(), m_pImage->GetHeight(), UnitPixel);
		graph.ReleaseHDC(pDC->GetSafeHdc());


		//Painting font
		CString szText;
		GetWindowText(szText);
		CRect txtRc = { 0,0,rcButton.Width() ,rcButton.Height() };
		CFont* oldFont = pDC->SelectObject(m_font.GetSafeHandle() ? &m_font : GetFont());
		if (!m_bShowTextFrame)
		{
			pDC->SetTextColor(m_crTextColor);
			pDC->DrawText(szText, szText.GetLength(), &txtRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_TABSTOP);
		}
		else
		{
			//Artistic font
			DrawTextString(pDC, szText, m_crTextColor, m_crTextFrameColor, &txtRc);
		}
		pDC->SelectObject(oldFont);
		bSuc = true;
	}
	return bSuc;
}

Image *CPng::ImageFromResource(HINSTANCE hInstance, UINT uImgID, LPCTSTR lpType)
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

void CPng::PreSubclassWindow()
{
	ModifyStyle(0, BS_OWNERDRAW);

	if (NULL != GetSafeHwnd())
	{
		if (!(GetButtonStyle() & WS_CLIPSIBLINGS))
			SetWindowLong(GetSafeHwnd(), GWL_STYLE, GetWindowLong(GetSafeHwnd(),
				GWL_STYLE) | WS_CLIPSIBLINGS);
	}

	CButton::PreSubclassWindow();
}

BOOL CPng::PreTranslateMessage(MSG * pMsg)
{
	if (m_ToolTip.m_hWnd != NULL)
		m_ToolTip.RelayEvent(pMsg);
	return CButton::PreTranslateMessage(pMsg);
}

void CPng::SetToolTips(LPCTSTR pszTips)
{
	m_strTips = pszTips;
	UpdateToolTip();
}

void CPng::UpdateToolTip()
{
	if (GetSafeHwnd())
	{
		if (m_ToolTip.GetSafeHwnd() == NULL) m_ToolTip.Create(this);
		if (m_strTips.IsEmpty() == false)
		{
			CRect ClientRect;
			GetClientRect(&ClientRect);
			m_ToolTip.Activate(TRUE);
			m_ToolTip.AddTool(this, m_strTips, &ClientRect, TOOLTIP_ID);
		}
		else m_ToolTip.Activate(FALSE);
	}
	return;
}

bool CPng::SetTextColor(COLORREF crTextColor, COLORREF crTextFrameColor, bool bShowFrame)
{
	m_crTextColor = crTextColor;
	m_bShowTextFrame = bShowFrame;
	m_crTextFrameColor = crTextFrameColor;

	if (GetSafeHwnd()) Invalidate(FALSE);
	return true;
}

void CPng::SetCursorType(HCURSOR hCursor) {
	m_cursor = hCursor;
}

BOOL CPng::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	::SetCursor(m_cursor);
	return TRUE;
}

void CPng::SetFontType(int fontSize, CString fontType) {
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfHeight = fontSize;
	_tcsncpy_s(lf.lfFaceName, LF_FACESIZE,fontType, fontType.GetLength());
	VERIFY(m_font.CreateFontIndirect(&lf));
}
BOOL CPng::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CPng::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_nState != CTRL_SELECTED)
	{
		m_nState = CTRL_SELECTED;

		if (!m_bMenuOn)
			m_bMenuOn = TRUE;

		PaintParent();
	}

	CButton::OnLButtonDown(nFlags, point);
}

void CPng::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_nState != CTRL_FOCUS)
	{
		m_nState = CTRL_FOCUS;
		PaintParent();
	}

	CButton::OnLButtonUp(nFlags, point);
}


void CPng::PaintParent()
{
	CRect   rect;
	GetWindowRect(&rect);
	GetParent()->ScreenToClient(&rect);
	GetParent()->InvalidateRect(&rect);
}

//Artistic font
void CPng::DrawTextString(CDC * pDC, LPCTSTR pszString, COLORREF crText, COLORREF crFrame, LPRECT lpRect)
{
	int nStringLength = lstrlen(pszString);
	int nXExcursion[8] = { 1,1,1,0,-1,-1,-1,0 };
	int nYExcursion[8] = { -1,0,1,1,1,0,-1,-1 };
	//Painting border
	pDC->SetTextColor(crFrame);
	CRect rcDraw;
	for (int i = 0; i < sizeof(nXExcursion)/sizeof(nXExcursion[0]); ++i)
	{
		rcDraw.CopyRect(lpRect);
		rcDraw.OffsetRect(nXExcursion[i], nYExcursion[i]);
		pDC->DrawText(pszString, nStringLength, &rcDraw, DT_VCENTER | DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
	}
	//Painting font
	rcDraw.CopyRect(lpRect);
	pDC->SetTextColor(crText);
	pDC->DrawText(pszString, nStringLength, &rcDraw, DT_VCENTER | DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
	return;
}

//Artistic font
void CPng::DrawTextString(CDC * pDC, LPCTSTR pszString, COLORREF crText, COLORREF crFrame, int nXPos, int nYPos)
{
	int nStringLength = lstrlen(pszString);
	int nXExcursion[8] = { 1,1,1,0,-1,-1,-1,0 };
	int nYExcursion[8] = { -1,0,1,1,1,0,-1,-1 };
	//Painting border
	pDC->SetTextColor(crFrame);
	for (int i = 0; i < sizeof(nXExcursion) / sizeof(nXExcursion[0]); i++)
	{
		pDC->TextOut(nXPos + nXExcursion[i], nYPos + nYExcursion[i], pszString, nStringLength);
	}

	//Painting font
	pDC->SetTextColor(crText);
	pDC->TextOut(nXPos, nYPos, pszString, nStringLength);
	return;
}
