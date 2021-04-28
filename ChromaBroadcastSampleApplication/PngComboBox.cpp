#include "stdafx.h"
#include "PngComboBox.h"
#include "resource.h"

IMPLEMENT_DYNAMIC(CPngComboBox, CComboBox)

BEGIN_MESSAGE_MAP(CPngComboBox, CComboBox)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	//ON_CONTROL_REFLECT(CBN_SELCHANGE, OnCbnSelchange)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
CPngComboBox::CPngComboBox()
{
	m_bOver = FALSE;
	m_bArtFont = FALSE;
	m_pBrsh = CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH));
	m_bgPen.CreatePen(PS_SOLID, 1, RGB(0x12, 0x12, 0x2b));
	m_font.CreateFont(20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, _T("arial"));
	m_bDown = FALSE;
	m_selItem = 666666666;
	m_preSelItem = 666666666;
	m_bFous = FALSE;
	m_endIcon = NULL;
	m_crFrame = DEF_FRAME_COLOR;
	m_crBgTitle = DEF_BG_COLOR;
	m_crBgSelected = DEF_SELECT_COLOR;
	m_crBgNoSelected = DEF_BG_COLOR;
	m_crArtFont = DEF_FRAME_COLOR;
	m_norImg = NULL;
	m_clickImg = NULL;
}

CPngComboBox::~CPngComboBox()
{
	if (m_bgPen.m_hObject != NULL)
		m_bgPen.DeleteObject();
	if (m_pBrsh->m_hObject != NULL)
		m_pBrsh->DeleteObject();
	if (m_font.m_hObject != NULL)
		m_font.DeleteObject();
	for (auto* e : m_vecItemList)
	{
		if (e != NULL) {
			delete e;
		}
	}
	if (m_norImg != NULL)
	{
		delete m_norImg;
		m_norImg = NULL;
	}
	if (m_clickImg != NULL)
	{
		delete m_clickImg;
		m_clickImg = NULL;
	}
}
void CPngComboBox::AddNewString(CString leftText, UINT icon, COLORREF leftColor)
{
	ItemList* pItem		= new ItemList;
	pItem->clrLeft		= leftColor;
	pItem->hIcon		= icon ? AfxGetApp()->LoadIcon(icon) : NULL;
	pItem->strUrl		= leftText;
	pItem->iItem		= this->AddString(leftText);
	m_vecItemList.push_back(pItem);
}
void CPngComboBox::SetEndIcon(UINT endIcon) {
	m_endIcon = endIcon;
}
void CPngComboBox::SetRightButton(UINT norImg, UINT clickImg) {
	m_norImg = ImageFromResource(AfxGetInstanceHandle(), norImg,_T("PNG"));
	m_clickImg = ImageFromResource(AfxGetInstanceHandle(), clickImg, _T("PNG"));
	if (!m_norImg || !m_clickImg) {
		m_norImg = m_clickImg = NULL;
	}
}
Image *CPngComboBox::ImageFromResource(HINSTANCE hInstance, UINT uImgID, LPCTSTR lpType)
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

void CPngComboBox::SetComboColor(COLORREF crFrame, COLORREF crBgTitle, COLORREF crBgSelected, COLORREF crBgNoSelected)
{
	m_crFrame = crFrame;
	m_crBgTitle = crBgTitle;
	m_crBgSelected = crBgSelected;
	m_crBgNoSelected = crBgNoSelected;
}
void CPngComboBox::SetFontType(int fontSize, CString fontType,BOOL bArtFont, COLORREF crArtFont) {
	m_font.Detach();
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfHeight = fontSize;
	_tcsncpy_s(lf.lfFaceName, LF_FACESIZE, fontType, fontType.GetLength());
	VERIFY(m_font.CreateFontIndirect(&lf));
	m_bArtFont = bArtFont;
	m_crArtFont = crArtFont;
}
void CPngComboBox::OnNcPaint(CDC* pDC)
{
	CDC dMemDC;
	dMemDC.CreateCompatibleDC(pDC);
	dMemDC.SetMapMode(pDC->GetMapMode());

	CBitmap mNewBmp;
	CRect rc;
	GetClientRect(&rc);

	mNewBmp.CreateCompatibleBitmap(pDC, rc.right - rc.left, rc.bottom - rc.top);
	CBitmap* pOldBmp = dMemDC.SelectObject(&mNewBmp);

	CPen* pOldPen = dMemDC.SelectObject(&m_bgPen);
	CBrush bgBs(m_crBgNoSelected);
	CBrush* pOldBrsh = dMemDC.SelectObject(&bgBs);
	dMemDC.Rectangle(&rc);
	dMemDC.SelectObject(pOldPen);
	dMemDC.SelectObject(pOldBrsh);

	if ((GetCurSel() > 5) || (GetCurSel() < 0))
	{
		return;
	}

	PItemList pItem = m_vecItemList[GetCurSel()];

	CPen	m_penLeft;
	CPen	m_penRight;
	CRect rcLeft, rcMid, rcRight;
	rcLeft = rcMid = rcRight = rc;
	dMemDC.SetBkMode(TRANSPARENT);

	if (pItem->hIcon != NULL) {
		rcLeft.left = 5;
		rcLeft.right = rcLeft.left + 16;
		rcLeft.top += 4;
		::DrawIconEx(dMemDC.m_hDC, rcLeft.left, rcLeft.top, pItem->hIcon, 16, 16, NULL, NULL, DI_NORMAL);
	}
	else {
		rcLeft.right = 0;
	}
	rcMid.left = rcLeft.right + 5;
	CFont* pOldFont = dMemDC.SelectObject(&m_font);
	DrawTextString(&dMemDC, pItem->strUrl, pItem->clrLeft,m_crArtFont, &rcMid);
	dMemDC.SelectObject(pOldFont);

	CRect rcEnd(rc);
	rcEnd.left = rc.right - 22;
	if (m_norImg == NULL) {
		if (m_bDown)
		{
			dMemDC.DrawFrameControl(&rcEnd, DFC_SCROLL, DFCS_SCROLLDOWN | DFCS_FLAT | DFCS_MONO | DFCS_PUSHED);
		}
		else
		{
			dMemDC.DrawFrameControl(&rcEnd, DFC_SCROLL, DFCS_SCROLLDOWN | DFCS_FLAT | DFCS_MONO);
		}
	}
	else {
		Graphics graphics(dMemDC.m_hDC);
		if (m_bDown) {
			graphics.DrawImage(m_norImg, rcEnd.left, (rcEnd.bottom - m_norImg->GetHeight()) / 2, m_norImg->GetWidth(), m_norImg->GetHeight());

		}
		else {
			graphics.DrawImage(m_clickImg, rcEnd.left, (rcEnd.bottom - m_clickImg->GetHeight())/2, m_clickImg->GetWidth(), m_clickImg->GetHeight());
		}
		graphics.ReleaseHDC(dMemDC.m_hDC);
	}


	pDC->BitBlt(rc.left, rc.top, rc.Width(), rc.Height(), &dMemDC,
		rc.left, rc.top, SRCCOPY);

	dMemDC.SelectObject(pOldBmp);
	pOldBmp->DeleteObject();
	dMemDC.DeleteDC();
}
void CPngComboBox::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	OnNcPaint(&dc);
}
void CPngComboBox::DrawContent(UINT iState, CRect rcClient, CDC* pDC, int itemID)
{
	PItemList pItem = m_vecItemList[itemID];

	CPen	m_penLeft;
	CPen	m_penRight;
	CRect rcLeft, rcMid;
	rcLeft = rcMid = rcClient;
	pDC->SetBkMode(TRANSPARENT);

	if (iState & ODS_SELECTED)
	{
		UpdateRect();
	}

	if (itemID == m_selItem)
	{
		CPen bgPen;
		bgPen.CreatePen(PS_SOLID, 1, DEF_SELECT_COLOR);
		CPen* pOldPen = pDC->SelectObject(&bgPen);
		CBrush bgBs(m_crBgSelected);
		CBrush* pOldBrush = pDC->SelectObject(&bgBs);
		pDC->RoundRect(&rcClient, CPoint(0, 0));
		pDC->SelectObject(pOldBrush);
		pDC->SelectObject(pOldPen);
		bgPen.DeleteObject();
	}
	else {
		CBrush bgBs(m_crBgNoSelected);
		pDC->FillRect(&rcClient,&bgBs);
	}
	if (pItem->hIcon != NULL) {
		rcLeft.left = 5;
		rcLeft.right = rcLeft.left + 16;
		rcLeft.top += 4;
		::DrawIconEx(pDC->m_hDC, rcLeft.left, rcLeft.top, pItem->hIcon, 16, 16, NULL, NULL, DI_NORMAL);
	}else{
		rcLeft.right = 0;
	}

	rcMid.left = rcLeft.right + 5;
	if(m_endIcon != NULL)
		rcMid.right -= 22;
	pDC->SetTextColor(pItem->clrLeft);
	CFont* pOldFont = pDC->SelectObject(&m_font);
	DrawTextString(pDC, pItem->strUrl, pItem->clrLeft, m_crArtFont, &rcMid);
	pDC->SelectObject(pOldFont);

	CRect rcEnd(rcClient);
	rcEnd.left = rcEnd.right - 18;
	rcEnd.top += 3;
	if (itemID == m_selItem && m_endIcon != NULL)
		DrawIconEx(pDC->m_hDC, rcEnd.left, rcEnd.top, AfxGetApp()->LoadIcon(m_endIcon), 16, 16, NULL, NULL, DI_NORMAL);
}

void CPngComboBox::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bOver == FALSE)
	{
		m_bOver = TRUE;
		UpdateRect();

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE;
		tme.dwHoverTime = 0;
		tme.hwndTrack = m_hWnd;

		_TrackMouseEvent(&tme);
	}
	CComboBox::OnMouseMove(nFlags, point);
}

LRESULT CPngComboBox::OnMouseLeave(WPARAM wparam, LPARAM lparam)
{
	if (m_bOver)
	{
		m_bOver = FALSE;
	}
	UpdateRect();

	return 1;
}

void CPngComboBox::UpdateRect(void)
{
	CRect rcClient;
	GetWindowRect(&rcClient);
	rcClient.DeflateRect(-2, -2);
	GetParent()->ScreenToClient(&rcClient);
	GetParent()->InvalidateRect(&rcClient, FALSE);
	return;
}

BOOL CPngComboBox::OnEraseBkgnd(CDC * pDC)
{
	return TRUE;
}

void CPngComboBox::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bDown = TRUE;
	UpdateRect();
	CComboBox::OnLButtonDown(nFlags, point);
}

void CPngComboBox::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bDown = FALSE;
	UpdateRect();
	CComboBox::OnLButtonUp(nFlags, point);
}
void CPngComboBox::OnCbnSelchange()
{
	UpdateRect();
}

void CPngComboBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (lpDrawItemStruct->CtlType != ODT_COMBOBOX)
	{
		return;
	}
	UINT itemID = lpDrawItemStruct->itemID;//the index of the item in the combo box
	CRect rcClient = lpDrawItemStruct->rcItem;
	UINT  iState = lpDrawItemStruct->itemState;

	CBitmap MemBit;

	//CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	//DrawContent(iState, rcClient, pDC, itemID);

	CDC dc;
	dc.Attach(lpDrawItemStruct->hDC);
	DrawContent(iState, rcClient, &dc, itemID);
	dc.Detach();
}

void CPngComboBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	CRect rc;
	GetClientRect(&rc);
	lpMeasureItemStruct->itemHeight = rc.Height();
	lpMeasureItemStruct->itemWidth = rc.Width();
}

LRESULT CPngComboBox::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (WM_CTLCOLORLISTBOX == message)
	{
		HWND hListBox = (HWND)lParam;

		CListBox* pListBox = (CListBox*)FromHandle(hListBox);
		ASSERT(pListBox);
		int nCount = pListBox->GetCount();

		if (CB_ERR != nCount)
		{
			CPoint pt;
			GetCursorPos(&pt);
			pListBox->ScreenToClient(&pt);

			CRect rc;
			for (int i = 0; i < nCount; i++)
			{
				pListBox->GetItemRect(i, &rc);
				if (rc.PtInRect(pt))
				{
					m_preSelItem = m_selItem;

					m_selItem = i;
					if (m_selItem != m_preSelItem)
					{
						CRect preRc;
						CRect rcFous(rc);
						rcFous.left = rcFous.right - 18;

						pListBox->GetItemRect(m_preSelItem, &preRc);
						pListBox->InvalidateRect(&preRc);

						if (rcFous.PtInRect(pt))
						{
							m_bFous = TRUE;
							pListBox->InvalidateRect(&rc);
						}
						else
						{
							m_bFous = FALSE;
							pListBox->InvalidateRect(&rc);
						}

					}

					break;
				}
			}
		}

	}
	return CComboBox::WindowProc(message, wParam, lParam);
}


void CPngComboBox::DrawTextString(CDC * pDC, LPCTSTR pszString, COLORREF crText, COLORREF crFrame, LPRECT lpRect)
{
	int nStringLength = lstrlen(pszString);
	int nXExcursion[8] = { 1,1,1,0,-1,-1,-1,0 };
	int nYExcursion[8] = { -1,0,1,1,1,0,-1,-1 };

	pDC->SetTextColor(crFrame);
	CRect rcDraw;
	for (int i = 0; i < sizeof(nXExcursion) / sizeof(nXExcursion[0]); ++i)
	{
		rcDraw.CopyRect(lpRect);
		rcDraw.OffsetRect(nXExcursion[i], nYExcursion[i]);
		pDC->DrawText(pszString, nStringLength, &rcDraw, DT_VCENTER | DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);
	}
	rcDraw.CopyRect(lpRect);
	pDC->SetTextColor(crText);
	pDC->DrawText(pszString, nStringLength, &rcDraw, DT_VCENTER | DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);
	return;
}

void CPngComboBox::DrawTextString(CDC * pDC, LPCTSTR pszString, COLORREF crText, COLORREF crFrame, int nXPos, int nYPos)
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
