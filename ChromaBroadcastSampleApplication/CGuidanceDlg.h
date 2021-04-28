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
