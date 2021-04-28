#pragma once
#include "Public.h"
//////////////////////////////////////////////////////////////////////////

#define DEF_TEXT_FRAME_COLOR			RGB(255,255,255)
#define DEF_TEXT_COLOR1					RGB(10,10,10)
#define TOOLTIP_ID						100

//////////////////////////////////////////////////////////////////////////
class CPng : public CButton
{
public:
	CPng();
	virtual ~CPng();
public:
	void Init(UINT nImg, int nPartNum, UINT nBtnType = BTN_TYPE_NORMAL);
	bool SetTextColor(COLORREF crTextColor, COLORREF crTextFrameColor = DEF_TEXT_FRAME_COLOR, bool bShowFrame = false);
	void SetToolTips(LPCTSTR pszTips);
	void SetCursorType(HCURSOR hCursor);
	//Set font size and type
	void SetFontType(int fontSize, CString fontType);
protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	DECLARE_MESSAGE_MAP()

protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void PreSubclassWindow();
	virtual BOOL PreTranslateMessage(MSG * pMsg);
	bool ShowImage(CDC* pDC, Image* pImage, UINT nState);
	void PaintParent();
	Image *ImageFromResource(HINSTANCE hInstance, UINT uImgID, LPCTSTR lpType);
	void UpdateToolTip();
	void DrawTextString(CDC * pDC, LPCTSTR pszString, COLORREF crText, COLORREF crFrame, LPRECT lpRect);
	void DrawTextString(CDC * pDC, LPCTSTR pszString, COLORREF crText, COLORREF crFrame, int nXPos, int nYPos);

private:
	int m_nImgPart;
	UINT			m_nState;
	UINT			m_nBtnType;
	BOOL			m_bMenuOn;
	BOOL			m_bTracked;
	BOOL			m_bShowTextFrame;
	COLORREF		m_crTextColor;
	COLORREF		m_crTextFrameColor;
	CString			m_strTips;
	HCURSOR			m_cursor;
	CToolTipCtrl	m_ToolTip;
	Image*			m_pImage;
	CFont			m_font;
};
