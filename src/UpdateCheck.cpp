#include "stdafx.h"

#include "resource.h"
#include "UpdateCheck.h"
#include "XMLReader.h"



void CUpdateCheck::checkForUpdates(HWND hwnd, bool inform_no_update)
{
	m_hwnd = hwnd;
	m_inform_no_update = inform_no_update;
	lastUpdateDate = "";
	latestVersion = "";
	userMessage = "";

	_beginthread(CUpdateCheck::bgUpdateCheck, 0, this);
}

void CUpdateCheck::bgUpdateCheck(void*pThis)
{
	CUpdateCheck* p = (CUpdateCheck*)pThis;

	CString strFileName;
	const HRESULT hr = ::URLDownloadToCacheFile(NULL,
		_T("https://www.tinycad.net/Update/TinyCAD"),
		strFileName.GetBuffer(MAX_PATH),
		URLOSTRM_GETNEWESTVERSION,
		0,
		NULL);
	strFileName.ReleaseBuffer();

	// Did this work?
	if (hr == S_OK)
	{
		// Yes, so read in the file
		{
			CFile theFile;

			// Open the file for reading
			const BOOL r = theFile.Open(strFileName, CFile::modeRead);

			if (r)
			{
				CString name;

				// Create the XML stream writer
				CStreamFile stream(&theFile, CArchive::load);
				CXMLReader xml(&stream);

				// Now parse the XML to get the update information
				// Get the update tag
				xml.nextTag(name);
				if (name == "Update")
				{
					xml.intoTag();
					while (xml.nextTag(name))
					{
						// Is this a symbol?
						if (name == "DATE")
						{
							xml.getChildData(p->lastUpdateDate);
						}
						else if (name == "VERSION")
						{
							xml.getChildData(p->latestVersion);
						}
						else if (name == "MESSAGE")
						{
							xml.getChildData(p->userMessage);
						}
					}
				}
			}
		}

		// Now we can delete the file
		DeleteFile(strFileName);

		// Now signal to the main app that we have new information
		SendMessage(p->m_hwnd, WM_COMMAND, ID_AUTOUPDATE, 0);
	}
}

int64_t CUpdateCheck::VersionAsNumber(CString str)
{
	// Decode the version in to 2 numbers for easy comparison
	CString n = str;
	n.Replace(_T("."), _T(""));
	return _wtoll(n);
}
