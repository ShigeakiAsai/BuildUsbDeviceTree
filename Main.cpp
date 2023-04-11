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
	CAtlString strParentProperty;

	setlocale(LC_CTYPE, "");

	strParentProperty = L"USB\\VID_174C&PID_1351\\MSFT30HBSA21311000413_____";
	//	strParentProperty = L"USB\\VID_174B&PID_55BB\\MSFT3000000000000020191024";

	GetTargetUsbDeviceInfo(strParentProperty);

	return	0;
}
