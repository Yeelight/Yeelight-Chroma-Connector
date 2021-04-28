// CGuidanceDlg.cpp: Implementation file
//

#include "stdafx.h"
#include "ChromaYeelightApp.h"
#include "CGuidanceDlg.h"
#include "afxdialogex.h"
#include "ChromaBroadcastSampleApplicationDlg.h"


IMPLEMENT_DYNAMIC(CGuidanceDlg, CDialogEx)

CGuidanceDlg::CGuidanceDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CGuidanceDlg::IDD, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON2);
    //CGuidanceDlg* pInstance;
    CDC* pDC = nullptr;
    //CRect dialogRect;
}

CGuidanceDlg::~CGuidanceDlg()
{
    //if (pDC)
    //{
    //    ReleaseDC(pDC);
    //    pDC = nullptr;
    //}
}

void CGuidanceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BOOL CGuidanceDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // Set the icon for this dialog.  The framework does this automatically
//  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon

    SetBackgroundImage(IDB_BITMAP7);

    return TRUE;  // return TRUE  unless you set the focus to a control
}


BEGIN_MESSAGE_MAP(CGuidanceDlg, CDialogEx)
END_MESSAGE_MAP()

void CGuidanceDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    CDialogEx::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CGuidanceDlg::SetParentDialog(CDialogEx* dialog)
{
    this->m_ParentDialog = dialog;
}

void CGuidanceDlg::OnCancel()
{
    DestroyWindow();
    ((CChromaBroadcastSampleApplicationDlg*)m_ParentDialog)->pGuidanceDlg = NULL;
    //CDialogEx::OnCancel();
}

//void CGuidanceDlg::PostNcDestroy()
//{
//    CDialogEx::PostNcDestroy();
//    if (this != NULL)
//    {
//        delete this;
//    }
//}
