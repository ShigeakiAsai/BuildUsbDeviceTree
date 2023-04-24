#define UNICODE 1
#define _UNICODE 1

#define WIN32_LEAN_AND_MEAN		// Windows ヘッダーから使用されていない部分を除外します。
// Windows ヘッダー ファイル:
#include <windows.h>

#include "atlstr.h"
#include "atlcoll.h"

#include <locale.h>

#include "BuildUsbDeviceTree.h"


int main(int argc, wchar_t* argv[])
{
	TARGET_USB_DEVICE_INFO  sTargetUsbDeviceInfo;
	CAtlString              strParentProperty;

	setlocale(LC_CTYPE, "");

	//  strParentProperty = L"USB\\VID_174C&PID_1351\\MSFT30HBSA21311000413_____";
	//	strParentProperty = L"USB\\VID_174B&PID_55BB\\MSFT3000000000000020191024";
	strParentProperty = L"USB\\VID_174C&PID_1351\\MSFT30________19032205B4B5";

	GetTargetUsbDeviceInfo(strParentProperty , &sTargetUsbDeviceInfo);


#ifdef _DEBUG
	// デバッグコード
	if (sTargetUsbDeviceInfo.nFindTargetNumber == 0)
	{
		wprintf(L"Not Found Device!!!\n");
		wprintf(L"  %s\n", (LPCTSTR)strParentProperty);
	}
	else if (sTargetUsbDeviceInfo.nFindTargetNumber > 1)
	{
		wprintf(L"Too Found Device!!!\n");
		wprintf(L"  %s\n", (LPCTSTR)strParentProperty);
	}
	else
	{
		wprintf(L"Found Target Device!\n");
		wprintf(L"  %s\n", (LPCTSTR)strParentProperty);
		wprintf(L"  DeviceIsOperatingAtSuperSpeedOrHigher   :%d\n", sTargetUsbDeviceInfo.nTargetDeviceDeviceIsOperatingAtSuperSpeedOrHigher);
		wprintf(L"  DeviceDeviceIsSuperSpeedCapableOrHigher :%d\n", sTargetUsbDeviceInfo.nTargetDeviceDeviceIsSuperSpeedCapableOrHigher);
	}
#endif

	return	0;
}
