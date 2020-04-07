


#if !defined(AFX_VISVIM_H__AC72670B_2977_11D1_B2F3_006008040780__INCLUDED_)
#define AFX_VISVIM_H__AC72670B_2977_11D1_B2F3_006008040780__INCLUDED_

#if !defined(__AFXWIN_H__)
#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h" 

#include <ObjModel\addguid.h>
#include <ObjModel\appguid.h>
#include <ObjModel\bldguid.h>
#include <ObjModel\textguid.h>
#include <ObjModel\dbgguid.h>





HKEY GetAppKey (char* AppName);
HKEY GetSectionKey (HKEY hAppKey, LPCTSTR Section);
int GetRegistryInt (HKEY hSectionKey, LPCTSTR Entry, int Default);
bool WriteRegistryInt (HKEY hSectionKey, char* Entry, int nValue);
void ReportLastError (HRESULT Err);
void ReportInternalError (char* Fct);




#endif 
