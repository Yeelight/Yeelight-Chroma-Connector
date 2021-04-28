#pragma once
#include "Public.h"
//////////////////////////////////////////////////////////////////////////

#define DEF_TEXT_FRAME_COLOR			RGB(255,255,255)
#define DEF_TEXT_COLOR2					RGB(10,10,10)
#define TOOLTIP_ID						100

//////////////////////////////////////////////////////////////////////////
class CPngButton : public CButton
{
public:
	CPngButton();
	virtual ~CPngButton();
public:
	//void Init(UINT nImg, int nPartNum, UINT nBtnType = BTN_TYPE_NORMAL);
	void Init(UINT nBtnType, UINT nNoFocusImg, UINT nFocusImg = NULL, UINT nSelectImg = NULL, UINT nDisableImg = NULL);
	bool SetTextColor(COLORREF crTextColor, COLORREF crTextFrameColor = DEF_TEXT_FRAME_COLOR, bool bShowFrame = false);
	void SetToolTips(LPCTSTR pszTips);
	void SetCursorType(HCURSOR hCursor);
	void SetFontType(int fontSize, CString fontType);
	void SetState(UINT new_state);
protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseHOver(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnPaint();
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
	Image*			m_pImageNoFocus;
	Image*			m_pImageFocus;
	Image*			m_pImageSelect;
	Image*			m_pImageDisable;

	CFont			m_font;
};
