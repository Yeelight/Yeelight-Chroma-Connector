#pragma once
#include "Public.h"

class CGraphicsRoundRectPath: public GraphicsPath
{
		
public:
	CGraphicsRoundRectPath();
	CGraphicsRoundRectPath(INT x, INT y, INT width, INT height, INT cornerX, INT cornerY);
 
public:
	void AddRoundRect(INT x, INT y, INT width, INT height, INT cornerX, INT cornerY);
};