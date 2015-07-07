// TestLocale.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <ostream>
#include "TestLocale.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print an error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	else
	{
		std::cout << "Locale test begins" << std::endl;
		std::cout << "\tdefault code page is " << GetACP() << std::endl;
		std::cout << "\t_AtlGetConversionACP code page is " << ATL::_AtlGetConversionACP() << std::endl;

		CPINFOEX cpinfo = {};
		GetCPInfoEx(ATL::_AtlGetConversionACP(), 0, &cpinfo);

		std::cout << "\tThread code page is " << cpinfo.CodePage << std::endl;

		std::cout << std::endl;


		std::cout << "Setting locale to default returns " << SetThreadLocale(LOCALE_SYSTEM_DEFAULT) << std::endl;


		std::cout << "\tnew default code page is " << GetACP() << std::endl;
		std::cout << "\tnew _AtlGetConversionACP code page is " << ATL::_AtlGetConversionACP() << std::endl;

		GetCPInfoEx(ATL::_AtlGetConversionACP(), 0, &cpinfo);

		std::cout << "\tnew Thread code page is " << cpinfo.CodePage << std::endl;
		std::cout << "Locale test ends." << std::endl;

		nRetCode = 0;
	}

	return nRetCode;
}
