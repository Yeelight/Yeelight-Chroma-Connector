// IniFile.h
#pragma once

#include "stdafx.h"

class CIniFile
{
public:
    CIniFile();
    CIniFile(LPCTSTR szFileName);
    virtual ~CIniFile();

public:
    // Attributes
    void SetFileName(LPCTSTR szFileName);

public:
    // Operations
    BOOL SetProfileInt(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, int nKeyValue);
    BOOL SetProfileString(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, LPCTSTR lpszKeyValue);

    DWORD GetProfileSectionNames(CStringArray& arrSection);

    int GetProfileInt(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName);
    DWORD GetProfileString(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, CString& szKeyValue);

    BOOL DeleteSection(LPCTSTR lpszSectionName);
    BOOL DeleteKey(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName);

private:
    WCHAR m_szFileName[MAX_PATH];

    UINT m_unMaxSection; // max sections(256)
    UINT m_unSectionNameMaxSize; // max section name len,32(Null-terminated)

    void Init();
};
