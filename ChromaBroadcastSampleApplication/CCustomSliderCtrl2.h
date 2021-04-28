#include "stdafx.h"

#pragma once

#define      G_BASE_CCCCCC				        RGB(0,0,0)//black
#define      G_BASE_0078D6				        RGB(0, 120, 214)//blue
#define      TRACK_HOVERTHUMB_COLOR		        RGB(255, 255, 255)//white
#define      TRACK_DEGAULTTHUMB_COLOR	        RGB(255, 255, 255)//white
#define      G_EDIT_OPT_BK                      RGB(0xff, 0, 0) //red
#define      G_BASE_WHITE                       RGB(255, 255, 255)//white


#define      CHANNEL_LEFT_COLOR                 RGB(255, 255, 255)//white
#define      CHANNEL_RIGHT_COLOR                RGB(0x12,0x12,0x2B)//dark blue
#define      SLIDER_BK_COLOR                    RGB(0x29,0x2f,0x45)//grey blue
#define      THUMB_HOVER_COLOR                  RGB(255, 255, 255)
#define      THUMB_DEFAULT_COLOR                RGB(255, 255, 255)//white


class CCustomSliderCtr2: public CSliderCtrl
{
    DECLARE_DYNCREATE(CCustomSliderCtr2)
public:
    CCustomSliderCtr2();
    virtual ~CCustomSliderCtr2();
    virtual void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);

protected:
    afx_msg void OnPaint();
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnMouseLeave();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    DECLARE_MESSAGE_MAP()

private:
    bool        m_bHoverThumb;
    CBrush     	m_hoverBrush;
    CPen        m_hoverPen;
    CBrush     	m_defaultBrush;
    CPen        m_defaultPen;
};
