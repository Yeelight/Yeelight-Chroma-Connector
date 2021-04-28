// PngButton.cpp : implementation file

#include "stdafx.h"
#include "PngButton.h"

CPngButton::CPngButton()
{
	m_bTracked	= FALSE;
	m_bMenuOn	= FALSE;
	m_nImgPart	= 0;
	m_pImage	= NULL;
	m_pImageNoFocus = NULL;
	m_pImageFocus = NULL;
	m_pImageSelect = NULL;
	m_pImageDisable = NULL;
	m_nState			= CTRL_NOFOCUS;
	m_nBtnType			= BTN_TYPE_NORMAL;
	m_bShowTextFrame	= FALSE;
	m_crTextColor		= DEF_TEXT_COLOR2;
	m_crTextFrameColor	= DEF_TEXT_FRAME_COLOR;
	m_cursor = ::LoadCursor(NULL, IDC_ARROW);
}

CPngButton::~CPngButton()
{
	if (m_pImage != NULL)
	{
		delete m_pImage;
		m_pImage = NULL;
	}
	if (m_pImageNoFocus != NULL)
	{
		delete m_pImageNoFocus;
		m_pImageNoFocus = NULL;
	}

	if (m_pImageFocus != NULL)
	{
		delete m_pImageFocus;
		m_pImageFocus = NULL;
	}

	if (m_pImageSelect != NULL)
	{
		delete m_pImageSelect;
		m_pImageSelect = NULL;
	}

	if (m_pImageDisable != NULL)
	{
		delete m_pImageDisable;
		m_pImageDisable = NULL;
	}
}

void CPngButton::Init(UINT nBtnType, UINT nNoFocusImg, UINT nFocusImg, UINT nSelectImg, UINT nDisableImg)
{
	if (nNoFocusImg)
	{
		m_pImageNoFocus = ImageFromResource(AfxGetResourceHandle(), nNoFocusImg, L"PNG");
	}
	if (nFocusImg)
	{
		m_pImageFocus = ImageFromResource(AfxGetResourceHandle(), nFocusImg, L"PNG");
	}
	if (nSelectImg)
	{
		m_pImageSelect = ImageFromResource(AfxGetResourceHandle(), nSelectImg, L"PNG");
	}
	if (nDisableImg)
	{
		m_pImageDisable = ImageFromResource(AfxGetResourceHandle(), nDisableImg, L"PNG");
	}

	m_nBtnType = nBtnType;
	//m_nImgPart = nPartNum;

	if (m_pImageNoFocus == NULL)
		return;

	CRect rcButton;

	rcButton = CRect(0, 0, m_pImageNoFocus->GetWidth(), m_pImageNoFocus->GetHeight());

	GetWindowRect(&rcButton);
	//SetWindowPos(NULL, 0, 0, rcButton.Width(), rcButton.Height(), SWP_NOACTIVATE | SWP_NOMOVE);
}

BEGIN_MESSAGE_MAP(CPngButton, CButton)
	//{{AFX_MSG_MAP(CPngButton)
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_PAINT()
	ON_MESSAGE(WM_MOUSEHOVER, OnMouseHOver)
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	//}}AFX_MSG_MAP
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPngButton message handlers

void CPngButton::OnPaint()
{
	CButton::OnPaint();
}

void CPngButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	//if (!IsWindowEnabled())
	//	m_nState = CTRL_DISABLE;
	CDC dc;
	dc.Attach(lpDrawItemStruct->hDC);
	ShowImage(&dc, m_pImage, m_nState);
	dc.Detach();
}

bool CPngButton::ShowImage(CDC* pDC, Image* pImage, UINT nState)
{
	bool bSuc = false;

	CRect ClientRect;
	GetWindowRect(&ClientRect);

	CRect rcButton;

	if (nState == CTRL_NOFOCUS)
	{
		//rcButton = CRect(0, 0, m_pImageNoFocus->GetWidth(), m_pImageNoFocus->GetHeight());
		rcButton = CRect(0, 0, ClientRect.Width(), ClientRect.Height());
		pImage = m_pImageNoFocus;
	}

	else if (nState == CTRL_FOCUS)
	{
		//rcButton = CRect(0, 0, m_pImageFocus->GetWidth(), m_pImageFocus->GetHeight());
		rcButton = CRect(0, 0, ClientRect.Width(), ClientRect.Height());
		pImage = m_pImageFocus;
	}
	else if (nState == CTRL_SELECTED)
	{
		//rcButton = CRect(0, 0, m_pImageSelect->GetWidth(), m_pImageSelect->GetHeight());
		rcButton = CRect(0, 0, ClientRect.Width(), ClientRect.Height());
		pImage = m_pImageSelect;
	}
	else if (nState == CTRL_DISABLE)
	{
		rcButton = CRect(0, 0, ClientRect.Width(), ClientRect.Height());
		pImage = m_pImageDisable;
	}
	else
	{
		return false;
	}

	pDC->SetBkMode(TRANSPARENT);

	Graphics graph(pDC->GetSafeHdc());
	//graph.DrawImage(pImage, RectF(0, 0, rcButton.Width(), rcButton.Height()), rcButton.left, rcButton.top, rcButton.Width(), rcButton.Height(), UnitPixel);
	graph.DrawImage(pImage, RectF(0, 0, rcButton.Width(), rcButton.Height()), 0, 0, pImage->GetWidth(), pImage->GetHeight(), UnitPixel);
	graph.ReleaseHDC(pDC->GetSafeHdc());


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
		DrawTextString(pDC, szText, m_crTextColor, m_crTextFrameColor, &txtRc);
	}
	pDC->SelectObject(oldFont);
	bSuc = true;

	return bSuc;
}

Image *CPngButton::ImageFromResource(HINSTANCE hInstance, UINT uImgID, LPCTSTR lpType)
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

void CPngButton::PreSubclassWindow()
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




BOOL CPngButton::PreTranslateMessage(MSG * pMsg)
{
	if (m_ToolTip.m_hWnd != NULL)
		m_ToolTip.RelayEvent(pMsg);
	return CButton::PreTranslateMessage(pMsg);
}

void CPngButton::SetState(UINT new_state)
{
	m_nState = new_state;
}

void CPngButton::SetToolTips(LPCTSTR pszTips)
{
	m_strTips = pszTips;
	UpdateToolTip();
}

void CPngButton::UpdateToolTip()
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

bool CPngButton::SetTextColor(COLORREF crTextColor, COLORREF crTextFrameColor, bool bShowFrame)
{
	m_crTextColor = crTextColor;
	m_bShowTextFrame = bShowFrame;
	m_crTextFrameColor = crTextFrameColor;

	if (GetSafeHwnd()) Invalidate(FALSE);
	return true;
}
void CPngButton::SetCursorType(HCURSOR hCursor) {
	m_cursor = hCursor;
}

BOOL CPngButton::OnSetCursor(CWnd * pWnd, UINT nHitTest, UINT message)
{
	::SetCursor(m_cursor);
	return TRUE;
}

void CPngButton::SetFontType(int fontSize, CString fontType) {
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfHeight = fontSize;
	_tcsncpy_s(lf.lfFaceName, LF_FACESIZE,fontType, fontType.GetLength());
	VERIFY(m_font.CreateFontIndirect(&lf));
}
BOOL CPngButton::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CPngButton::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (!m_bTracked) {
		TRACKMOUSEEVENT tme;
		ZeroMemory(&tme, sizeof(TRACKMOUSEEVENT));
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_HOVER | TME_LEAVE;
		tme.dwHoverTime = 1;
		tme.hwndTrack = this->GetSafeHwnd();
		if (::_TrackMouseEvent(&tme))
			m_bTracked = TRUE;
	}

	CButton::OnMouseMove(nFlags, point);
}

void CPngButton::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_nState != CTRL_SELECTED)
	{
		if (m_nState != CTRL_DISABLE)
		{
			m_nState = CTRL_SELECTED;

			if (!m_bMenuOn)
				m_bMenuOn = TRUE;

			PaintParent();
		}

	}

	CButton::OnLButtonDown(nFlags, point);
}

void CPngButton::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_nState != CTRL_FOCUS)
	{
		if (m_nState != CTRL_DISABLE)
		{
			m_nState = CTRL_FOCUS;
			PaintParent();
		}

	}

	CButton::OnLButtonUp(nFlags, point);
}

LRESULT CPngButton::OnMouseHOver(WPARAM wParam, LPARAM lParam)
{
	if (m_nState == CTRL_DISABLE)
	{
		return 0;
	}
	if (m_nState != CTRL_FOCUS)
	{
		m_nState = CTRL_FOCUS;
		PaintParent();
	}

	return 0;
}
LRESULT CPngButton::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	m_bTracked = FALSE;

	if (m_nBtnType == BTN_TYPE_NORMAL)
	{
		if (m_nState != CTRL_DISABLE)
		{
			m_nState = CTRL_NOFOCUS;
		}
	}
	else if (m_nBtnType == BTN_TYPE_MENU)
	{
		if (m_bMenuOn)
			m_nState = CTRL_SELECTED;
		else
			m_nState = CTRL_NOFOCUS;
	}

	PaintParent();
	return 0;
}

void CPngButton::PaintParent()
{
	CRect   rect;
	GetWindowRect(&rect);
	GetParent()->ScreenToClient(&rect);
	GetParent()->InvalidateRect(&rect);
}

void CPngButton::DrawTextString(CDC * pDC, LPCTSTR pszString, COLORREF crText, COLORREF crFrame, LPRECT lpRect)
{
	int nStringLength = lstrlen(pszString);
	int nXExcursion[8] = { 1,1,1,0,-1,-1,-1,0 };
	int nYExcursion[8] = { -1,0,1,1,1,0,-1,-1 };

	pDC->SetTextColor(crFrame);
	CRect rcDraw;
	for (int i = 0; i < sizeof(nXExcursion)/sizeof(nXExcursion[0]); ++i)
	{
		rcDraw.CopyRect(lpRect);
		rcDraw.OffsetRect(nXExcursion[i], nYExcursion[i]);
		pDC->DrawText(pszString, nStringLength, &rcDraw, DT_VCENTER | DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
	}

	rcDraw.CopyRect(lpRect);
	pDC->SetTextColor(crText);
	pDC->DrawText(pszString, nStringLength, &rcDraw, DT_VCENTER | DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
	return;
}

void CPngButton::DrawTextString(CDC * pDC, LPCTSTR pszString, COLORREF crText, COLORREF crFrame, int nXPos, int nYPos)
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
