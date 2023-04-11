#pragma once

typedef struct _TARGET_USB_DEVICE_INFO {
	USHORT      nTargetidVendor;
	USHORT      nTargetidProduct;
	CAtlString  strTargetiSerialNumber;
	USHORT      nFindTargetNumber;
	ULONG       nTargetDeviceSpeed_Usb110;
	ULONG       nTargetDeviceSpeed_Usb200;
	ULONG       nTargetDeviceSpeed_Usb300;
	ULONG       nTargetDeviceDeviceIsOperatingAtSuperSpeedOrHigher;
	ULONG       nTargetDeviceDeviceIsSuperSpeedCapableOrHigher;
	ULONG       nTargetDeviceDeviceIsOperatingAtSuperSpeedPlusOrHigher;
	ULONG       nTargetDeviceDeviceIsSuperSpeedPlusCapableOrHigher;
} TARGET_USB_DEVICE_INFO, * PTARGET_USB_DEVICE_INFO;


extern void	GetTargetUsbDeviceInfo(CAtlString& strParentProperty , PTARGET_USB_DEVICE_INFO pTargetUsbDeviceInfo);


