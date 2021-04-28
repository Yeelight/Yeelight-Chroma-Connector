#include "stdafx.h"
#include "IniFile.h"

void CIniFile::Init()
{
    m_unMaxSection = 512;
    m_unSectionNameMaxSize = 33;
}

CIniFile::CIniFile()
{
    Init();
}

CIniFile::CIniFile(LPCTSTR szFileName)
{
    Init();

    wcscpy_s(m_szFileName, szFileName);


}

CIniFile::~CIniFile()
{

}

void CIniFile::SetFileName(LPCTSTR szFileName)
{
    wcscpy_s(m_szFileName, szFileName);
}

DWORD CIniFile::GetProfileSectionNames(CStringArray & arrSection)
{
    int i;
    int iPos = 0;
    int iMaxCount;
    TCHAR chSectionNames[2048] = { 0 };
    TCHAR chSection[2048] = { 0 };
    GetPrivateProfileSectionNamesW(chSectionNames, 2048, m_szFileName);

    for (i = 0; i < 2048; i++)
    {
        if (chSectionNames[i] == 0)
            if (chSectionNames[i] == chSectionNames[i + 1])
                break;
    }

    iMaxCount = i + 1;

    for (i = 0; i < iMaxCount; i++)
    {
        chSection[iPos++] = chSectionNames[i];
        if (chSectionNames[i] == 0 && 0 != i)
        {
            arrSection.Add(chSection);
            memset(chSection, 0, i);
            iPos = 0;
        }

    }

    return (int)arrSection.GetSize();
}

DWORD CIniFile::GetProfileString(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, CString& szKeyValue)
{
    DWORD dwCopied = 0;
    dwCopied = ::GetPrivateProfileString(lpszSectionName, lpszKeyName, _T(""),
        szKeyValue.GetBuffer(MAX_PATH), MAX_PATH, m_szFileName);
    szKeyValue.ReleaseBuffer();

    return dwCopied;
}

int CIniFile::GetProfileInt(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName)
{
    int nKeyValue = ::GetPrivateProfileInt(lpszSectionName, lpszKeyName, 0, m_szFileName);

    return nKeyValue;
}

BOOL CIniFile::SetProfileString(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, LPCTSTR lpszKeyValue)
{
    return ::WritePrivateProfileString(lpszSectionName, lpszKeyName, lpszKeyValue, m_szFileName);
}

BOOL CIniFile::SetProfileInt(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, int nKeyValue)
{
    CString szKeyValue;
    //szKeyValue.Format("%d", nKeyValue);

    return ::WritePrivateProfileString(lpszSectionName, lpszKeyName, szKeyValue, m_szFileName);
}

BOOL CIniFile::DeleteSection(LPCTSTR lpszSectionName)
{
    return ::WritePrivateProfileSection(lpszSectionName, NULL, m_szFileName);
}

BOOL CIniFile::DeleteKey(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName)
{
    return ::WritePrivateProfileString(lpszSectionName, lpszKeyName, NULL, m_szFileName);
}
