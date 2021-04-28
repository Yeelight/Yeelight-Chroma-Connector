#include "StdAfx.h"
#include "CGraphicsRoundRectPath.h"


CGraphicsRoundRectPath::CGraphicsRoundRectPath(void)
	:Gdiplus::GraphicsPath()
{

}

CGraphicsRoundRectPath::CGraphicsRoundRectPath(INT x, INT y, INT width, INT height, INT cornerX, INT cornerY)
	:Gdiplus::GraphicsPath()
{
	AddRoundRect(x,y,width,height,cornerX,cornerY);
}
void CGraphicsRoundRectPath::AddRoundRect(INT x, INT y, INT width, INT height, INT cornerX, INT cornerY)
{
	INT elWid = 2*cornerX;
	INT elHei = 2*cornerY;

	AddArc(x,y,elWid,elHei,180,90); // Upper left arc
	AddLine(x+cornerX,y,x+width-cornerX,y);// upper line

	AddArc(x+width-elWid,y, elWid,elHei,270,90); // Upper right arc
	AddLine(x+width,y+cornerY, x+width,y+height-cornerY);// right line

	AddArc(x+width-elWid,y+height-elHei, elWid,elHei,0,90); // Arc at bottom right corner
	AddLine(x+width-cornerX,y+height, x+cornerX,y+height); // bottom line

	AddArc(x,y+height-elHei, elWid,elHei,90,90);
	AddLine(x,y+cornerY, x, y+height-cornerY);
}
