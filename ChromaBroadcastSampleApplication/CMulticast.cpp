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
