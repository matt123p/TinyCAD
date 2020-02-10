#pragma once
class CUpdateCheck
{
private:

	// Window to signal back on
	HWND m_hwnd;

	// When was the last time this update information was generated?
	CString lastUpdateDate;

	// What is the latest version of TinyCAD?
	CString latestVersion;

	// Is there a special message for the user?
	CString userMessage;

	// Do we notify even if there is no update available?
	bool m_inform_no_update;


	static void bgUpdateCheck(void*);
public:
	void checkForUpdates(HWND hwnd, bool inform_no_update);

	CString getLatestVersion() { return latestVersion; }
	CString getLastUpdateDate() { return lastUpdateDate; }
	CString getUserMessage() { return userMessage; }
	bool getInformNoUpdate() { return m_inform_no_update;  }

	int64_t VersionAsNumber(CString str);
};

