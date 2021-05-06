/*
 *Copyright (c) 2021, Yeelight
 *All rights reserved.
 *
 *Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are met:
 *
 *1. Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *2. Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 *3. Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 */

#pragma once
#include "Resource.h"


class CGuidanceDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CGuidanceDlg)

public:
	CGuidanceDlg(CWnd* pParent = nullptr);
	virtual ~CGuidanceDlg();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);

	//Set the parent dialog pointer
	void SetParentDialog(CDialogEx* dialog);
	afx_msg void OnCancel();
	//void PostNcDestroy();

	enum { IDD = IDD_DIALOG_GUIDE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//virtual void PostNcDestroy();

	DECLARE_MESSAGE_MAP()
private:
	CFont m_FontNormal;
	CGuidanceDlg* pInstance;
	CDC* pDC;
	CRect dialogRect;
	CDialogEx* m_ParentDialog;

protected:
	HICON m_hIcon;
};
