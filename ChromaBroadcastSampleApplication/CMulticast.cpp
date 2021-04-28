#include "stdafx.h"
//#include "pch.h"
#include "CMulticast.h"

#include "ChromaYeelightApp.h"
#include "ChromaBroadcastSampleApplicationDlg.h"


// MySocket

CMulticast::CMulticast()
{
}

CMulticast::~CMulticast()
{
}



void CMulticast::OnAccept(int nErrorCode)
{

	//CAsyncSocket::OnAccept(nErrorCode);
}


void CMulticast::OnClose(int nErrorCode)
{
	if (nErrorCode == 0)
	{
		((CChromaBroadcastSampleApplicationDlg*)m_pDlg)->OnMulticastClose();
	}
	CAsyncSocket::OnClose(nErrorCode);
}


void CMulticast::OnConnect(int nErrorCode)
{
	if (nErrorCode == 0)
	{
		((CChromaBroadcastSampleApplicationDlg*)m_pDlg)->OnConnect();
	}
	CAsyncSocket::OnConnect(nErrorCode);
}


void CMulticast::OnOutOfBandData(int nErrorCode)
{
	CAsyncSocket::OnOutOfBandData(nErrorCode);
}


void CMulticast::OnReceive(int nErrorCode)
{
	if (nErrorCode == 0)
	{
		((CChromaBroadcastSampleApplicationDlg*)m_pDlg)->OnReceive();
	}
	CAsyncSocket::OnReceive(nErrorCode);
}


void CMulticast::OnSend(int nErrorCode)
{
	CAsyncSocket::OnSend(nErrorCode);
}

void CMulticast::set_dialog(CDialogEx* dialog)
{
	this->m_pDlg = dialog;
}
