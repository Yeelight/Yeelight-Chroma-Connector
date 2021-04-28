#pragma once
#include "afxsock.h"
class CMulticast :
    public CAsyncSocket
{
public:
	CMulticast();
	virtual ~CMulticast();
	virtual void OnAccept(int nErrorCode);
	virtual void OnClose(int nErrorCode);
	virtual void OnConnect(int nErrorCode);
	virtual void OnOutOfBandData(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
	virtual void OnSend(int nErrorCode);

	void set_dialog(CDialogEx* dialog);

private:
	CDialogEx* m_pDlg;
};

