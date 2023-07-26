#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <shobjidl.h>
#include <shlguid.h>
#include <strsafe.h>

using namespace std;
//LPTSTR child = "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"; 
BOOL findRegKey(HKEY rootHkey , LPTSTR regPath , LPTSTR lpkeyname , LPTSTR findkokeyname ,LPBYTE key_value);
BOOL GetRegValue(HKEY findregkey , LPTSTR regkeyname ,LPBYTE key_vle);
//打开目录搜索是否有OBS Studio 程序
BOOL Open_dir(LPTSTR lpdirPath , LPTSTR exename , LPSTR allpath);
HRESULT ResolveIt(HWND hwnd , LPCSTR lpszLinkFile , LPSTR lpszPath , int iPathBufferSize);
int main() {
	HKEY rootHkey = HKEY_LOCAL_MACHINE;
	LPTSTR child = "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"; 	
	TCHAR findkeyname[MAX_PATH] = {0};
	BYTE  key_vle[MAX_PATH] = {0};
	CHAR openfilenamebuffer[MAX_PATH];
	CHAR openlinkpath[MAX_PATH];
	BOOL find = findRegKey(rootHkey , child , "Desktop" , findkeyname, key_vle);
	if (find) {
		printf("取到的子项为: %s\n" , findkeyname);
		if (Open_dir((LPTSTR)key_vle , "OBS Studio.lnk" , openfilenamebuffer)) {
			printf("main openfilenambuffer ---- %s \n" , openfilenamebuffer);
			if (SUCCEEDED(ResolveIt(NULL , openfilenamebuffer , openlinkpath , MAX_PATH))) {
				printf("找到真实地址为  %s \n" , openlinkpath);
			}
		}
	}
	return 0; 
}
HRESULT ResolveIt(HWND hwnd , LPCSTR lpszLinkFile , LPSTR lpszPath , int iPathBufferSize) {
	CoInitialize(0);
	HRESULT hres;
	IShellLink* psl;
	TCHAR szGotPath[MAX_PATH];
	TCHAR szDescription[MAX_PATH];
	WIN32_FIND_DATA wfd = { 0 };

	//*lpszPath = 0; // Assume failure 

	// Get a pointer to the IShellLink interface. It is assumed that CoInitialize
	// has already been called. 
	hres = CoCreateInstance(CLSID_ShellLink , NULL , CLSCTX_INPROC_SERVER , IID_IShellLink , (LPVOID*)&psl);
	if (SUCCEEDED(hres))
	{
		IPersistFile* ppf;

		// Get a pointer to the IPersistFile interface. 
		hres = psl->QueryInterface(IID_IPersistFile , (void**)&ppf);

		if (SUCCEEDED(hres))
		{
			WCHAR wsz[MAX_PATH];

			// Ensure that the string is Unicode. 
			MultiByteToWideChar(CP_ACP , 0 , lpszLinkFile , -1 , wsz , MAX_PATH);

			// Add code here to check return value from MultiByteWideChar 
			// for success.

			// Load the shortcut. 
			hres = ppf->Load(wsz , STGM_READ);

			if (SUCCEEDED(hres))
			{
				// Resolve the link. 
				hres = psl->Resolve(hwnd , 0);

				if (SUCCEEDED(hres))
				{
					// Get the path to the link target. 
					hres = psl->GetPath(szGotPath , MAX_PATH , (WIN32_FIND_DATA*)&wfd , SLGP_SHORTPATH);
					//wprintf(L"%s" , szGotPath);
					if (SUCCEEDED(hres))
					{
						// Get the description of the target. 
						hres = psl->GetDescription(szDescription , MAX_PATH);

						if (SUCCEEDED(hres))
						{
							hres = StringCbCopy(lpszPath , iPathBufferSize , szGotPath);
							if (SUCCEEDED(hres))
							{
								// Handle success
							}
							else
							{
								// Handle the error
							}
						}
					}
				}
			}

			// Release the pointer to the IPersistFile interface. 
			ppf->Release();
		}

		// Release the pointer to the IShellLink interface. 
		psl->Release();
	}
	else {
		printf("%d \n" , GetLastError());
	}
	return hres;
}
BOOL Open_dir(LPTSTR lpdirPath , LPTSTR exename , LPSTR ALLpath) {
	printf("%s \n" , lpdirPath);
	WIN32_FIND_DATA		findFiledata;
	HANDLE hListFile;
	CHAR szFilePath[MAX_PATH];
	lstrcpy(szFilePath , lpdirPath);
	lstrcat(szFilePath , "\\");
	lstrcat(szFilePath , exename);
	hListFile = FindFirstFile(szFilePath, &findFiledata);
	if (hListFile == INVALID_HANDLE_VALUE) {
		printf("错误 %d ! \n" , GetLastError());
		return FALSE;
	}
	else {
		do {
			if (lstrcmp(findFiledata.cFileName , ".") == 0 || lstrcmp(findFiledata.cFileName , "..") == 0) {
				continue;
			}
			
			if (lstrcmp(exename , findFiledata.cFileName) == 0) {
				strcpy_s(ALLpath , strlen(szFilePath) + 1  , szFilePath);
				break;
			}
		} while (FindNextFile(hListFile , &findFiledata));
	}
	return true;
}
BOOL GetRegValue(HKEY findregkey , LPTSTR regkeyname, LPBYTE keyvle) {

	DWORD dwtype;
	DWORD key_value = MAX_PATH;
	LONG  status;
	BYTE key_vle[MAX_PATH];
	status  = RegQueryValueEx(findregkey , regkeyname , 0 , &dwtype , key_vle , &key_value);
	if (status == ERROR_SUCCESS) {
		strcpy_s((LPTSTR)keyvle , MAX_PATH , (LPTSTR)key_vle);	
		return true;
	}
	//
	return false;
}
BOOL findRegKey(HKEY rootHkey,LPTSTR regPath , LPTSTR lpkeyname , LPTSTR findkeyname, LPBYTE key_value){

	HKEY dekstop;
	LPCTSTR	 lpsubkey = regPath;
	DWORD result = RegOpenKeyEx(rootHkey, lpsubkey , 0 , KEY_READ | KEY_WOW64_64KEY , &dekstop);
	if (result != ERROR_SUCCESS) {
		printf("打开需要枚举的KEY错误! \n");
	}

	TCHAR subName[MAX_PATH] = { 0 };
	DWORD subsize = MAX_PATH;

	BYTE  subuffer[MAX_PATH] = { 0 };
	DWORD subufsize = MAX_PATH;

	LONG eumeresult;
	DWORD dindex = 0;

	DWORD dwType;

	do {
		eumeresult = RegEnumValue(dekstop , dindex++ , subName , &subsize , NULL , &dwType , subuffer , &subufsize);		
		if (strstr(subName , lpkeyname) != 0) {
			strcpy_s(findkeyname , lstrlen(subName) + 1 , subName);
			BYTE key_vle[MAX_PATH];
			if (!GetRegValue(dekstop , findkeyname,key_vle)) {
					printf("函数调用错误!\n");
				}
			strcpy_s((LPTSTR)key_value , MAX_PATH , (LPTSTR)key_vle);
				RegCloseKey(dekstop);
				return true;
		}
		subsize = MAX_PATH;
		subufsize = MAX_PATH;
	} while (eumeresult != ERROR_NO_MORE_ITEMS);
	RegCloseKey(dekstop);
	printf("没找到 %s 子项 \n" , lpkeyname);
	return false; 
}
