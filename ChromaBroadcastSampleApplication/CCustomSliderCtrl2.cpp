#include "stdafx.h"
#include "CCustomSliderCtrl2.h"


// CCustomSliderCtr2
BEGIN_MESSAGE_MAP(CCustomSliderCtr2, CSliderCtrl)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	//ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE(CCustomSliderCtr2, CSliderCtrl);

CCustomSliderCtr2::CCustomSliderCtr2(): m_hoverBrush(TRACK_HOVERTHUMB_COLOR),m_hoverPen(PS_SOLID, 1,       TRACK_HOVERTHUMB_COLOR), m_defaultBrush(TRACK_DEGAULTTHUMB_COLOR), m_defaultPen(PS_SOLID, 1, TRACK_DEGAULTTHUMB_COLOR){
m_bHoverThumb = FALSE;
}

CCustomSliderCtr2::~CCustomSliderCtr2(){}

void CCustomSliderCtr2::OnPaint()
{
    CClientDC dc(this);
    CSliderCtrl::OnPaint();

	//Draw the overall background color of the slider
	CRect client_rect;
	GetClientRect(client_rect);
	CBrush brush_bg(SLIDER_BK_COLOR);
	dc.FillRect(client_rect, &brush_bg);

    //Draw the left and right sides of the thumb respectively
    CRect rect,rectaimL,rectaimR;

    GetChannelRect(&rect);
    rectaimL = rect;
    rectaimR = rect;

    GetThumbRect(&rect);
	int offset = 2;
	//The left side of the thumb to the head position of the channel
	rectaimL.top = rect.top + offset;
	rectaimL.bottom = rect.bottom - offset;
	rectaimL.right = rect.left;

	//The right side of the thumb to the end of the channel
	rectaimR.top = rect.top + offset;
	rectaimR.bottom = rect.bottom - offset;
	rectaimR.left = rect.right;

	//Fill the left and right sides separately
    CBrush brush_left(CHANNEL_LEFT_COLOR);
    dc.FillRect(rectaimL,&brush_left);

	CBrush brush_right(CHANNEL_RIGHT_COLOR);
    dc.FillRect(rectaimR,&brush_right);

	//Fill the thumb
	GetThumbRect(&rect);
	CBrush brush_thumb(CHANNEL_LEFT_COLOR);
	dc.FillRect(rect, &brush_thumb);

	//Draw the channel background color (the channel horizontal line color is no longer drawn)
    //CBrush brushS(G_BASE_WHITE);
    //int rectaimH = rect.Height()/3;
    //rectaimL.top += rectaimH;
    //rectaimL.bottom -= rectaimH;
    //rectaimR.top += rectaimH;
    //rectaimR.bottom -= rectaimH;
    //dc.FillRect(rectaimL,&brushS);
    //dc.FillRect(rectaimR,&brushS);
}



void CCustomSliderCtr2::OnCustomDraw(NMHDR * pNMHDR, LRESULT *pResult)
{
    NMCUSTOMDRAW nmcd = *(LPNMCUSTOMDRAW)pNMHDR;
    if ( nmcd.dwDrawStage == CDDS_PREPAINT )
    {
         // return CDRF_NOTIFYITEMDRAW so that we will get subsequent
        // CDDS_ITEMPREPAINT notifications
        *pResult = CDRF_NOTIFYITEMDRAW ;
         return;
    }
   else if (nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
    {
        if ( nmcd.dwItemSpec == TBCD_THUMB )
        {
		    *pResult = CDRF_DODEFAULT;
		    CBrush* pBrush = NULL;
		    CPen* pPen = NULL;
		    switch(nmcd.uItemState)
		    {
		        case 0:
			        if(m_bHoverThumb && IsWindowEnabled())
			        {
				        pBrush = &m_hoverBrush;
				        pPen = &m_hoverPen;
			        }
			        else if(IsWindowEnabled())
			        {
				        pBrush = &m_defaultBrush;
				        pPen = &m_defaultPen;
			        }
			       else
			        {
				        pBrush = &m_hoverBrush;
				        pPen = &m_hoverPen;
			        }
			    break;
		        case CDIS_SELECTED:
			            pBrush = &m_hoverBrush;
			            pPen = &m_hoverPen;
			            break;
		        default:
			            pBrush = &m_defaultBrush;
			            pPen = &m_defaultPen;
			            break;
		  }

		    CDC *pdc = CDC::FromHandle(nmcd.hdc);
		    CBrush * pOldBrush = pdc->SelectObject(pBrush);
		    CPen* pOldPen = pdc->SelectObject(pPen);

		    CRect rcThumb,rcChannel,rcPaint;
		    GetThumbRect(rcThumb);
		    GetChannelRect(rcChannel);
		    rcPaint = rcThumb;
		    rcPaint.bottom = rcChannel.bottom + 1;

		    pdc->Rectangle(rcPaint);

		    pdc->SelectObject(pOldBrush);
		    pdc->SelectObject(pOldPen);
		    *pResult = CDRF_SKIPDEFAULT;
        }
    }
}


void CCustomSliderCtr2::OnMouseMove(UINT nFlags, CPoint point)
{
    CRect rc;
    GetThumbRect(&rc);
    if(rc.PtInRect(point))
	    m_bHoverThumb = TRUE;
    else
	    m_bHoverThumb = FALSE;
    CSliderCtrl::OnMouseMove(nFlags, point);
}

void CCustomSliderCtr2::OnMouseLeave()
{
    m_bHoverThumb = FALSE;
    CSliderCtrl::OnMouseLeave();
}

BOOL CCustomSliderCtr2::OnEraseBkgnd(CDC* pDC)
{
    CRect rect,rectaim;
    GetClientRect(&rect);
    rectaim =  rect;

    GetThumbRect(&rect);
    rectaim.top = rect.top;
    rectaim.bottom = rect.bottom;
    CBrush brush(G_EDIT_OPT_BK);
    pDC->FillRect(rectaim,&brush);
    return TRUE;
}
