#pragma once
#include "Public.h"

#define DEF_TEXT_FRAME_COLOR			RGB(255,255,255)
#define DEF_TEXT_COLOR					RGB(10,10,10)
#define TOOLTIP_ID						100

//
class CPngCheckBox : public CButton
{
public:
	CPngCheckBox();
	virtual ~CPngCheckBox();

public:
	BOOL GetChecked() { return m_bChecked; }
	void SetChecked(BOOL bChecked = TRUE) { m_bChecked = bChecked; Invalidate(FALSE); }

public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

protected:
	virtual void PreSubclassWindow();

public:
	void Init(UINT nBkImg, UINT nCkImg);
	void SetCheckBoxTextColor(COLORREF crTextColor);
	void SetBackColor(COLORREF crBack) { m_crBack = crBack; }
	bool SetTextColor(COLORREF crTextColor, COLORREF crTextFrameColor = DEF_TEXT_FRAME_COLOR, bool bShowFrame = false);
	void SetFontType(int fontSize, CString fontType);
	Image *ImageFromResource(HINSTANCE hInstance, UINT uImgID, LPCTSTR lpType);
	void DrawTextString(CDC * pDC, LPCTSTR pszString, COLORREF crText, COLORREF crFrame, LPRECT lpRect);
	void DrawTextString(CDC * pDC, LPCTSTR pszString, COLORREF crText, COLORREF crFrame, int nXPos, int nYPos);
	void PaintParent();

protected:
	BOOL								m_bChecked;
	Image*								m_ImageCheck;
	Image*								m_ImageBack;
	COLORREF							m_crBack;
	COLORREF							m_crTextColor;
	CFont								m_font;
	BOOL								m_bShowTextFrame;
	COLORREF							m_crTextFrameColor;
	BOOL								m_bSetBack;

	afx_msg BOOL OnClicked();
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	DECLARE_MESSAGE_MAP()
	DECLARE_DYNAMIC(CPngCheckBox)
};
