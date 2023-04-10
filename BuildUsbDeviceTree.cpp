// Test07.cpp : �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//
//

#define UNICODE 1
#define _UNICODE 1

#define WIN32_LEAN_AND_MEAN		// Windows �w�b�_�[����g�p����Ă��Ȃ����������O���܂��B
// Windows �w�b�_�[ �t�@�C��:
#include <windows.h>

// C �����^�C�� �w�b�_�[ �t�@�C��
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// �ꕔ�� CString �R���X�g���N�^�͖����I�ł��B

#include <atlbase.h>


//
//USB�f�o�C�X���̎擾
//
//�����ł�DDK/WDK�Ɋ܂܂��T���v���\�[�X�R�[�h�uUSBView�v��
//���̂܂ܗ��p���Ă��܂��B���p�̍ۂɂ̓��C�Z���X�ɋC������
//���������B
//
//��Windows Driver Kit�̓����Ȃǂ̏��
//http://www.usefullcode.net/2006/12/windows_driver_kit.html
//


#include "winioctl.h"
#include "cfgmgr32.h"
#include "setupapi.h"
#pragma	comment(lib,"setupapi.lib")


//////////////////////////////////////////
//DDK�Ɋ܂܂��w�b�_�[�t�@�C����2�g�p
//
//�uC:\WINDDK\6000\inc\wxp,C:\WINDDK\6000\inc\crt,C:\WINDDK\6000\inc\api�v
//��Visual Studio��include�p�X�ݒ�֒ǉ����邱�ƁI
//��DDK�̃C���X�g�[����ɍ��킹�ăp�X���C������K�v������܂�
//
//�����ł�Windows Vista�ɑΉ�����Windows Driver Kit�𗘗p���܂����B
//
#include <usbioctl.h>
#include <usbiodef.h>


#include "atlstr.h"
#include "atlcoll.h"

#include <locale.h>

#include "DnpTreeData.h"


const	TCHAR* ConnectionStatuses[] =
{
	_T("NoDeviceConnected"),
	_T("DeviceConnected"),
	_T("DeviceFailedEnumeration"),
	_T("DeviceGeneralFailure"),
	_T("DeviceCausedOvercurrent"),
	_T("DeviceNotEnoughPower")
};


class	CUsbInfo
{
protected:

	class	CStringDescriptor
	{
	public:
		CStringDescriptor()
		{
			_nDescriptorIndex = 0;
			_nLanguageID = 0;
		}

		UCHAR		_nDescriptorType;
		UCHAR		_nDescriptorIndex;
		USHORT		_nLanguageID;
		CAtlString	_strDescriptor;

		CAtlArray<BYTE>	_acbRawString;
	};


	bool	DriverNameToDeviceDesc(LPCTSTR pszDriverName, BOOLEAN DeviceId,CAtlString& strDesc)
	{
		DEVINST     devInst;
		DEVINST     devInstNext;
		CONFIGRET   cr;
		ULONG       len;
		TCHAR		buf[MAX_DEVICE_ID_LEN];
		ZeroMemory(buf, sizeof(buf));


		strDesc = _T("");

		// Get Root DevNode
		//
		cr = ::CM_Locate_DevNode(&devInst,NULL,0);
		if (cr != CR_SUCCESS)
			return false;

		// Do a depth first search for the DevNode with a matching
		// DriverName value
		//
		while(1)
		{
			// Get the DriverName value
			//
			len = MAX_DEVICE_ID_LEN;
			cr = ::CM_Get_DevNode_Registry_Property(devInst,CM_DRP_DRIVER,NULL,buf,&len,0);

			if(cr != CR_SUCCESS || _tcsicmp (pszDriverName,buf) != 0)
			{
				// This DevNode didn't match, go down a level to the first child.
				//
				cr = ::CM_Get_Child(&devInstNext,devInst,0);
				if (cr == CR_SUCCESS)
				{
					devInst = devInstNext;
					continue;
				}

				// Can't go down any further, go across to the next sibling.  If
				// there are no more siblings, go back up until there is a sibling.
				// If we can't go up any further, we're back at the root and we're
				// done.
				//
				while(1)
				{
					cr = ::CM_Get_Sibling(&devInstNext,devInst,0);
					if (cr == CR_SUCCESS)
					{
						devInst = devInstNext;
						break;
					}

					cr = ::CM_Get_Parent(&devInstNext,devInst,0);
					if (cr == CR_SUCCESS)
						devInst = devInstNext;
					else
						return	false;
				}
				continue;
			}


			// If the DriverName value matches, return the DeviceDescription
			//
			len = sizeof(buf);

			if (DeviceId)
				cr = ::CM_Get_Device_ID(devInst,buf,len,0);
			else
				cr = ::CM_Get_DevNode_Registry_Property(devInst,CM_DRP_DEVICEDESC,NULL,buf,&len,0);
			if (cr == CR_SUCCESS)
			{
				strDesc = buf;
				return true;
			}
			return false;
		}

		return false;
	}


	//*****************************************************************************
	//
	// GetHCDDriverKeyName()
	//
	//*****************************************************************************
	bool	GetHCDDriverKeyName (HANDLE HCD,CAtlString& strDriverKeyName)
	{
		BOOL	ret;
		ULONG	nBytes;
		USB_HCD_DRIVERKEY_NAME	sDriverKeyName;
		PUSB_HCD_DRIVERKEY_NAME	pDriverKeyName;

		strDriverKeyName = _T("");
		::ZeroMemory(&sDriverKeyName,sizeof(USB_HCD_DRIVERKEY_NAME));
		ret = ::DeviceIoControl(HCD,IOCTL_GET_HCD_DRIVERKEY_NAME,
								  &sDriverKeyName,sizeof(USB_HCD_DRIVERKEY_NAME),
								  &sDriverKeyName,sizeof(USB_HCD_DRIVERKEY_NAME),&nBytes,NULL);
		if(ret == FALSE)
			return	false;

		nBytes = sDriverKeyName.ActualLength;
		if (nBytes <= sizeof(USB_HCD_DRIVERKEY_NAME))
			return	false;

		pDriverKeyName = (PUSB_HCD_DRIVERKEY_NAME)new BYTE[nBytes];
		if (pDriverKeyName == NULL)
			return	false;

		::ZeroMemory(pDriverKeyName,nBytes);
		ret = ::DeviceIoControl(HCD,IOCTL_GET_HCD_DRIVERKEY_NAME,
								  pDriverKeyName,nBytes,
								  pDriverKeyName,nBytes,&nBytes,NULL);

		if(ret)
			strDriverKeyName = pDriverKeyName->DriverKeyName;

		delete[]	pDriverKeyName;

		return ret ? true : false;
	}


	//*****************************************************************************
	//
	// GetRootHubName()
	//
	//*****************************************************************************
	bool	GetRootHubName(HANDLE HostController,CAtlString& strRootHubName)
	{
		BOOL	ret;
		ULONG	nBytes;
		USB_ROOT_HUB_NAME	sRootHubName;
		PUSB_ROOT_HUB_NAME	pRootHubName;

		strRootHubName = _T("");
		ret = ::DeviceIoControl(HostController,IOCTL_USB_GET_ROOT_HUB_NAME,NULL,0,&sRootHubName,sizeof(USB_ROOT_HUB_NAME),&nBytes,NULL);
		if (!ret)
			return	false;

		nBytes = sRootHubName.ActualLength;
		pRootHubName = (PUSB_ROOT_HUB_NAME)new BYTE[nBytes];
		if (pRootHubName == NULL)
			return	false;

		ret = ::DeviceIoControl(HostController,IOCTL_USB_GET_ROOT_HUB_NAME,NULL,0,pRootHubName,nBytes,&nBytes,NULL);
		if(ret)
			strRootHubName = pRootHubName->RootHubName;

		delete[]	pRootHubName;

		return ret ? true : false;
	}


	//*****************************************************************************
	//
	// GetDriverKeyName()
	//
	//*****************************************************************************
	bool	GetDriverKeyName (HANDLE Hub,ULONG ConnectionIndex,CAtlString& strDriverKeyName)
	{
		BOOL	ret;
		ULONG	nBytes;
		USB_NODE_CONNECTION_DRIVERKEY_NAME	sDriverKeyName;
		PUSB_NODE_CONNECTION_DRIVERKEY_NAME	pDriverKeyName;

		strDriverKeyName = _T("");

		::ZeroMemory(&sDriverKeyName,sizeof(USB_NODE_CONNECTION_DRIVERKEY_NAME));
		sDriverKeyName.ConnectionIndex = ConnectionIndex;
		ret = ::DeviceIoControl(Hub,IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME,
								  &sDriverKeyName,sizeof(USB_NODE_CONNECTION_DRIVERKEY_NAME),
								  &sDriverKeyName,sizeof(USB_NODE_CONNECTION_DRIVERKEY_NAME),
								  &nBytes,NULL);
		if (!ret)
			return	false;

		nBytes = sDriverKeyName.ActualLength;
		if (nBytes <= sizeof(USB_NODE_CONNECTION_DRIVERKEY_NAME))
			return	false;

		pDriverKeyName = (PUSB_NODE_CONNECTION_DRIVERKEY_NAME)new BYTE[nBytes];
		if(pDriverKeyName == NULL)
			return	false;

		::ZeroMemory(pDriverKeyName,nBytes);
		pDriverKeyName->ConnectionIndex = ConnectionIndex;
		ret = ::DeviceIoControl(Hub,IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME,
								  pDriverKeyName,nBytes,
								  pDriverKeyName,nBytes,
								  &nBytes,NULL);
		if(ret)
			strDriverKeyName = pDriverKeyName->DriverKeyName;

		delete[]	pDriverKeyName;

		return ret ? true : false;
	}


	//*****************************************************************************
	//
	// GetConfigDescriptor()
	//
	// hHubDevice - Handle of the hub device containing the port from which the
	// Configuration Descriptor will be requested.
	//
	// ConnectionIndex - Identifies the port on the hub to which a device is
	// attached from which the Configuration Descriptor will be requested.
	//
	// DescriptorIndex - Configuration Descriptor index, zero based.
	//
	//*****************************************************************************
	PUSB_DESCRIPTOR_REQUEST
	GetConfigDescriptor (
		HANDLE  hHubDevice,
		ULONG   ConnectionIndex,
		UCHAR   DescriptorIndex
	)
	{
		BOOL    success;
		ULONG   nBytes;
		ULONG   nBytesReturned;

		UCHAR   configDescReqBuf[sizeof(USB_DESCRIPTOR_REQUEST) + sizeof(USB_CONFIGURATION_DESCRIPTOR)];

		PUSB_DESCRIPTOR_REQUEST         configDescReq;
		PUSB_CONFIGURATION_DESCRIPTOR   configDesc;


		// Request the Configuration Descriptor the first time using our
		// local buffer, which is just big enough for the Cofiguration
		// Descriptor itself.
		//
		nBytes = sizeof(configDescReqBuf);

		configDescReq = (PUSB_DESCRIPTOR_REQUEST)configDescReqBuf;
		configDesc = (PUSB_CONFIGURATION_DESCRIPTOR)(configDescReq+1);

		::ZeroMemory(configDescReq,nBytes);

		// Indicate the port from which the descriptor will be requested
		//
		configDescReq->ConnectionIndex = ConnectionIndex;

		//
		// USBHUB uses URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE to process this
		// IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION request.
		//
		// USBD will automatically initialize these fields:
		//     bmRequest = 0x80
		//     bRequest  = 0x06
		//
		// We must inititialize these fields:
		//     wValue    = Descriptor Type (high) and Descriptor Index (low byte)
		//     wIndex    = Zero (or Language ID for String Descriptors)
		//     wLength   = Length of descriptor buffer
		//
		configDescReq->SetupPacket.wValue = (USB_CONFIGURATION_DESCRIPTOR_TYPE << 8) | DescriptorIndex;
		configDescReq->SetupPacket.wLength = (USHORT)(nBytes - sizeof(USB_DESCRIPTOR_REQUEST));

		success = DeviceIoControl(hHubDevice,IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
								  configDescReq,nBytes,
								  configDescReq,nBytes,
								  &nBytesReturned,NULL);

		if(success == FALSE || nBytes != nBytesReturned)
			return	NULL;

		if (configDesc->wTotalLength < sizeof(USB_CONFIGURATION_DESCRIPTOR))
			return NULL;

		// Now request the entire Configuration Descriptor using a dynamically
		// allocated buffer which is sized big enough to hold the entire descriptor
		//
		nBytes = sizeof(USB_DESCRIPTOR_REQUEST) + configDesc->wTotalLength;

		configDescReq = (PUSB_DESCRIPTOR_REQUEST)new BYTE[nBytes];
		if (configDescReq == NULL)
			return NULL;
		::ZeroMemory(configDescReq,nBytes);

		configDesc = (PUSB_CONFIGURATION_DESCRIPTOR)(configDescReq+1);

		// Indicate the port from which the descriptor will be requested
		//
		configDescReq->ConnectionIndex = ConnectionIndex;

		//
		// USBHUB uses URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE to process this
		// IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION request.
		//
		// USBD will automatically initialize these fields:
		//     bmRequest = 0x80
		//     bRequest  = 0x06
		//
		// We must inititialize these fields:
		//     wValue    = Descriptor Type (high) and Descriptor Index (low byte)
		//     wIndex    = Zero (or Language ID for String Descriptors)
		//     wLength   = Length of descriptor buffer
		//
		configDescReq->SetupPacket.wValue = (USB_CONFIGURATION_DESCRIPTOR_TYPE << 8) | DescriptorIndex;
		configDescReq->SetupPacket.wLength = (USHORT)(nBytes - sizeof(USB_DESCRIPTOR_REQUEST));

		success = DeviceIoControl(hHubDevice,IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
								  configDescReq,nBytes,
								  configDescReq,nBytes,
								  &nBytesReturned,NULL);

		if(success == FALSE || nBytes != nBytesReturned)
		{
			//OOPS();
			delete[]	(configDescReq);
			return NULL;
		}

		if (configDesc->wTotalLength != (nBytes - sizeof(USB_DESCRIPTOR_REQUEST)))
		{
			//OOPS();
			delete[]	(configDescReq);
			return NULL;
		}

		return configDescReq;
	}


	//*****************************************************************************
	//
	// GetStringDescriptor()
	//
	// hHubDevice - Handle of the hub device containing the port from which the
	// String Descriptor will be requested.
	//
	// ConnectionIndex - Identifies the port on the hub to which a device is
	// attached from which the String Descriptor will be requested.
	//
	// DescriptorIndex - String Descriptor index.
	//
	// LanguageID - Language in which the string should be requested.
	//
	//*****************************************************************************
	bool	GetStringDescriptor (
		HANDLE  hHubDevice,
		ULONG   ConnectionIndex,
		UCHAR   DescriptorIndex,
		USHORT  LanguageID,
		CAtlArray<CStringDescriptor>& acDescriptor
	)
	{
		BOOL    success;
		ULONG   nBytes;
		ULONG   nBytesReturned;

		UCHAR   stringDescReqBuf[sizeof(USB_DESCRIPTOR_REQUEST) + MAXIMUM_USB_STRING_LENGTH];

		PUSB_DESCRIPTOR_REQUEST stringDescReq;
		PUSB_STRING_DESCRIPTOR  stringDesc;

		nBytes = sizeof(stringDescReqBuf);

		stringDescReq = (PUSB_DESCRIPTOR_REQUEST)stringDescReqBuf;
		stringDesc = (PUSB_STRING_DESCRIPTOR)(stringDescReq+1);

		//
		// USBHUB uses URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE to process this
		// IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION request.
		//
		// USBD will automatically initialize these fields:
		//     bmRequest = 0x80
		//     bRequest  = 0x06
		//
		// We must inititialize these fields:
		//     wValue    = Descriptor Type (high) and Descriptor Index (low byte)
		//     wIndex    = Zero (or Language ID for String Descriptors)
		//     wLength   = Length of descriptor buffer
		//
		::ZeroMemory(stringDescReq,nBytes);
		stringDescReq->ConnectionIndex = ConnectionIndex;
		stringDescReq->SetupPacket.wValue = (USB_STRING_DESCRIPTOR_TYPE << 8) | DescriptorIndex;
		stringDescReq->SetupPacket.wIndex = LanguageID;
		stringDescReq->SetupPacket.wLength = (USHORT)(nBytes - sizeof(USB_DESCRIPTOR_REQUEST));

		success = DeviceIoControl(hHubDevice,IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
								  stringDescReq,nBytes,
								  stringDescReq,nBytes,
								  &nBytesReturned,NULL);

		if (!success || nBytesReturned < 2)
			return false;

		if (stringDesc->bDescriptorType != USB_STRING_DESCRIPTOR_TYPE)
			return false;

		if (stringDesc->bLength != nBytesReturned - sizeof(USB_DESCRIPTOR_REQUEST))
			return false;

		if (stringDesc->bLength % 2 != 0)
			return false;


		size_t	nIndex;

		nIndex = acDescriptor.GetCount();
		acDescriptor.SetCount(nIndex + 1);

		acDescriptor[nIndex]._nDescriptorType	= stringDesc->bDescriptorType;
		acDescriptor[nIndex]._nDescriptorIndex	= DescriptorIndex;
		acDescriptor[nIndex]._nLanguageID		= LanguageID;
		acDescriptor[nIndex]._strDescriptor		= stringDesc->bString;

		acDescriptor[nIndex]._acbRawString.SetCount(stringDesc->bLength);
		memcpy(acDescriptor[nIndex]._acbRawString.GetData(),stringDesc->bString,acDescriptor[nIndex]._acbRawString.GetCount());

		return true;
	}


	//*****************************************************************************
	//
	// GetStringDescriptors()
	//
	// hHubDevice - Handle of the hub device containing the port from which the
	// String Descriptor will be requested.
	//
	// ConnectionIndex - Identifies the port on the hub to which a device is
	// attached from which the String Descriptor will be requested.
	//
	// DescriptorIndex - String Descriptor index.
	//
	// NumLanguageIDs -  Number of languages in which the string should be
	// requested.
	//
	// LanguageIDs - Languages in which the string should be requested.
	//
	//*****************************************************************************
	bool	GetStringDescriptors (
		HANDLE  hHubDevice,
		ULONG   ConnectionIndex,
		UCHAR   DescriptorIndex,
		ULONG   NumLanguageIDs,
		USHORT  *LanguageIDs,
		CAtlArray<CStringDescriptor>& acDescriptor
	)
	{
		ULONG	i;
		bool	ret;

		for (i=0; i<NumLanguageIDs; i++)
		{
			ret = GetStringDescriptor(hHubDevice,ConnectionIndex,DescriptorIndex,*LanguageIDs,acDescriptor);
			LanguageIDs++;
		}

		return true;
	}


	//*****************************************************************************
	//
	// GetAllStringDescriptors()
	//
	// hHubDevice - Handle of the hub device containing the port from which the
	// String Descriptors will be requested.
	//
	// ConnectionIndex - Identifies the port on the hub to which a device is
	// attached from which the String Descriptors will be requested.
	//
	// DeviceDesc - Device Descriptor for which String Descriptors should be
	// requested.
	//
	// ConfigDesc - Configuration Descriptor (also containing Interface Descriptor)
	// for which String Descriptors should be requested.
	//
	//*****************************************************************************
	bool
	GetAllStringDescriptors (
		HANDLE                          hHubDevice,
		ULONG                           ConnectionIndex,
		PUSB_DEVICE_DESCRIPTOR          DeviceDesc,
		CAtlArray<CStringDescriptor>& acDescriptor
	)
	{
		bool	ret;

		ULONG	numLanguageIDs;
		USHORT*	languageIDs;
		PUCHAR	descEnd;

		PUSB_COMMON_DESCRIPTOR			commonDesc;
		PUSB_CONFIGURATION_DESCRIPTOR	ConfigDesc;

		CAtlArray<CStringDescriptor>	acLanguageIDDescriptor;

		////////////////////////////////
		//�������񂪑��݂��邩�`�F�b�N
		//
		ConfigDesc = (PUSB_CONFIGURATION_DESCRIPTOR)GetConfigDescriptor(hHubDevice,ConnectionIndex,0);
		if(ConfigDesc == NULL)
		{
			//�擾�ł���String Descriptors���Ȃ�
			delete	ConfigDesc;
			return	true;
		}
		//���ӁF��̏�����ʂ��Ă��m���ɕ������񂪂���Ƃ͌���Ȃ�


		////////////////////////////////
		//������̌����ƌ��ꐔ���擾
		//
		ret = GetStringDescriptor(hHubDevice,ConnectionIndex,0,0,acLanguageIDDescriptor);
		if (ret == false)
		{
			delete	ConfigDesc;
			return false;
		}

		numLanguageIDs = ((ULONG)acLanguageIDDescriptor[0]._acbRawString.GetCount() - 2) / 2;
		languageIDs = (USHORT*)new BYTE[acLanguageIDDescriptor[0]._acbRawString.GetCount()];
		if(languageIDs == NULL)
		{
			delete	ConfigDesc;
			return false;
		}
		memcpy(languageIDs,acLanguageIDDescriptor[0]._acbRawString.GetData(),acLanguageIDDescriptor[0]._acbRawString.GetCount());


		////////////////////////////////
		//��������擾
		//

		if (DeviceDesc->iManufacturer)
			ret = GetStringDescriptors(hHubDevice,ConnectionIndex,DeviceDesc->iManufacturer,numLanguageIDs,languageIDs,acDescriptor);

		if (DeviceDesc->iProduct)
			ret = GetStringDescriptors(hHubDevice,ConnectionIndex,DeviceDesc->iProduct,numLanguageIDs,languageIDs,acDescriptor);

		if (DeviceDesc->iSerialNumber)
			ret = GetStringDescriptors(hHubDevice,ConnectionIndex,DeviceDesc->iSerialNumber,numLanguageIDs,languageIDs,acDescriptor);


		descEnd = (PUCHAR)ConfigDesc + ConfigDesc->wTotalLength;

		commonDesc = (PUSB_COMMON_DESCRIPTOR)ConfigDesc;
		while ((PUCHAR)commonDesc + sizeof(USB_COMMON_DESCRIPTOR) < descEnd && (PUCHAR)commonDesc + commonDesc->bLength <= descEnd)
		{
			switch (commonDesc->bDescriptorType)
			{
				case USB_CONFIGURATION_DESCRIPTOR_TYPE:
				case USB_INTERFACE_DESCRIPTOR_TYPE:
					if (commonDesc->bLength != sizeof(USB_CONFIGURATION_DESCRIPTOR))
					{
						//OOPS();
						break;
					}
					if(commonDesc->bDescriptorType == USB_CONFIGURATION_DESCRIPTOR_TYPE && ((PUSB_CONFIGURATION_DESCRIPTOR)commonDesc)->iConfiguration)
					{
						ret = GetStringDescriptors(hHubDevice,ConnectionIndex,
												 ((PUSB_CONFIGURATION_DESCRIPTOR)commonDesc)->iConfiguration,
												 numLanguageIDs,languageIDs,acDescriptor);
					}
					else if(commonDesc->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE && ((PUSB_INTERFACE_DESCRIPTOR)commonDesc)->iInterface)
					{
						ret = GetStringDescriptors(hHubDevice,ConnectionIndex,
												 ((PUSB_INTERFACE_DESCRIPTOR)commonDesc)->iInterface,
												 numLanguageIDs,languageIDs,acDescriptor);
					}
					//������break;��z�u���Ȃ����ƁI
				default:
					commonDesc = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)commonDesc + commonDesc->bLength);
					continue;
			}
			break;
		}

		delete[]	languageIDs;
		delete[]	ConfigDesc;

		return true;
	}


	//*****************************************************************************
	//
	// GetExternalHubName()
	//
	//*****************************************************************************
	bool	GetExternalHubName (HANDLE Hub,ULONG ConnectionIndex,CAtlString& strExternalHubName)
	{
		BOOL	ret;
		ULONG	nBytes;
		USB_NODE_CONNECTION_NAME	extHubName;
		PUSB_NODE_CONNECTION_NAME	extHubNameW;

		strExternalHubName = _T("");

		::ZeroMemory(&extHubName,sizeof(USB_NODE_CONNECTION_NAME));
		extHubName.ConnectionIndex = ConnectionIndex;
		ret = ::DeviceIoControl(Hub,IOCTL_USB_GET_NODE_CONNECTION_NAME,
								  &extHubName,sizeof(USB_NODE_CONNECTION_NAME),
								  &extHubName,sizeof(USB_NODE_CONNECTION_NAME),
								  &nBytes,NULL);
		if(ret == FALSE)
			return	false;

		nBytes = extHubName.ActualLength;
		if (nBytes <= sizeof(USB_NODE_CONNECTION_NAME))
			return	false;

		extHubNameW = (PUSB_NODE_CONNECTION_NAME)new BYTE[nBytes];
		if(extHubNameW == NULL)
			return	false;

		::ZeroMemory(extHubNameW,nBytes);
		extHubNameW->ConnectionIndex = ConnectionIndex;
		ret = ::DeviceIoControl(Hub,IOCTL_USB_GET_NODE_CONNECTION_NAME,
								  extHubNameW,nBytes,
								  extHubNameW,nBytes,
								  &nBytes,NULL);

		if(ret)
			strExternalHubName = extHubNameW->NodeName;

		delete[]	extHubNameW;

		return ret ? true : false;
	}




	class	CInfo
	{
	public:
		CInfo()
		{
			_bNodeInfo = false;
			::ZeroMemory(&_sNodeInfo,sizeof(USB_NODE_INFORMATION));

			_bNodeConnectionInfoEx = false;
			//_pNodeConnectionInfoEx = (PUSB_NODE_CONNECTION_INFORMATION_EX)_pNodeConnectionInfoEx_buffer;
			::ZeroMemory(_pNodeConnectionInfoEx_buffer,sizeof(USB_NODE_CONNECTION_INFORMATION_EX) + sizeof(USB_PIPE_INFO) * 30);

			_nPort = 0;
		}

		int			_nPort;
		CAtlString	_strName;

		CAtlArray<CStringDescriptor>	acDescriptor;

		bool	_bNodeInfo;
		USB_NODE_INFORMATION	_sNodeInfo;

		bool	_bNodeConnectionInfoEx;
		PUSB_NODE_CONNECTION_INFORMATION_EX	Get_pNodeConnectionInfoEx()
		{
			return	(PUSB_NODE_CONNECTION_INFORMATION_EX)_pNodeConnectionInfoEx_buffer;
		}
		//PUSB_NODE_CONNECTION_INFORMATION_EX	_pNodeConnectionInfoEx;

	private:
		BYTE	_pNodeConnectionInfoEx_buffer[sizeof(USB_NODE_CONNECTION_INFORMATION_EX) + sizeof(USB_PIPE_INFO) * 30];

	};

	CAtlArray<CInfo>		_acDeviceInfo;

	CDnpTreeData<UINT>	_treeDeviceInfoIndex;


	//*****************************************************************************
	//
	// EnumerateHubPorts()
	//
	// hTreeParent - Handle of the TreeView item under which the hub port should
	// be added.
	//
	// hHubDevice - Handle of the hub device to enumerate.
	//
	// NumPorts - Number of ports on the hub.
	//
	//*****************************************************************************
	void EnumerateHubPorts(UINT nTreeIndex,HANDLE hHubDevice,ULONG NumPorts)
	{
		ULONG       index;
		BOOL        success;

		PUSB_NODE_CONNECTION_INFORMATION_EX connectionInfoEx;

		CAtlString	strDeviceDesc;

		// Loop over all ports of the hub.
		//
		// Port indices are 1 based, not 0 based.
		//
		for (index=1; index <= NumPorts; index++)
		{
			ULONG	nBytesEx;
			BYTE	pBuff[sizeof(USB_NODE_CONNECTION_INFORMATION_EX) + sizeof(USB_PIPE_INFO) * 30];

			nBytesEx = sizeof(pBuff);
			connectionInfoEx = (PUSB_NODE_CONNECTION_INFORMATION_EX)pBuff;
			::ZeroMemory(connectionInfoEx,nBytesEx);


			//IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX���擾
			connectionInfoEx->ConnectionIndex = index;
			success = DeviceIoControl(hHubDevice,IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX,
									  connectionInfoEx,nBytesEx,
									  connectionInfoEx,nBytesEx,
									  &nBytesEx,NULL);

			//IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX�̑����IOCTL_USB_GET_NODE_CONNECTION_INFORMATION���擾
			if (!success)
			{
				PUSB_NODE_CONNECTION_INFORMATION    connectionInfo;
				ULONG                               nBytes;

				// Try using IOCTL_USB_GET_NODE_CONNECTION_INFORMATION
				// instead of IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX
				//
				nBytes = sizeof(USB_NODE_CONNECTION_INFORMATION) + sizeof(USB_PIPE_INFO) * 30;

				connectionInfo = (PUSB_NODE_CONNECTION_INFORMATION)new BYTE[nBytes];

				::ZeroMemory(connectionInfo,nBytes);
				connectionInfo->ConnectionIndex = index;
				success = DeviceIoControl(hHubDevice,IOCTL_USB_GET_NODE_CONNECTION_INFORMATION,
										  connectionInfo,nBytes,
										  connectionInfo,nBytes,
										  &nBytes,NULL);

				if (!success)
				{
					delete[]	(connectionInfo);
					continue;
				}

				// Copy IOCTL_USB_GET_NODE_CONNECTION_INFORMATION into
				// IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX structure.
				//
				connectionInfoEx->ConnectionIndex	= connectionInfo->ConnectionIndex;
				connectionInfoEx->DeviceDescriptor	= connectionInfo->DeviceDescriptor;
				connectionInfoEx->CurrentConfigurationValue = connectionInfo->CurrentConfigurationValue;
				connectionInfoEx->Speed				= connectionInfo->LowSpeed ? UsbLowSpeed : UsbFullSpeed;
				connectionInfoEx->DeviceIsHub		= connectionInfo->DeviceIsHub;
				connectionInfoEx->DeviceAddress		= connectionInfo->DeviceAddress;
				connectionInfoEx->NumberOfOpenPipes	= connectionInfo->NumberOfOpenPipes;
				connectionInfoEx->ConnectionStatus	= connectionInfo->ConnectionStatus;
				memcpy(&connectionInfoEx->PipeList[0],&connectionInfo->PipeList[0],sizeof(USB_PIPE_INFO) * 30);
				delete[]	(connectionInfo);
			}


			bool	ret;
			UINT	nChildTreeIndex;
			UINT	nIndex;

			//�c���[��USB�n�u�^�f�o�C�X�̏���ǉ�
			nIndex = (UINT)_acDeviceInfo.GetCount();
			_acDeviceInfo.SetCount(nIndex + 1);
			ret = _treeDeviceInfoIndex.AddData(nTreeIndex,&nChildTreeIndex,nIndex);		//�q�Ƃ��Ēǉ�
			if(ret == false)
				continue;

			_acDeviceInfo[nIndex]._bNodeConnectionInfoEx = true;
			::memcpy(_acDeviceInfo[nIndex].Get_pNodeConnectionInfoEx(),connectionInfoEx,nBytesEx);
			_acDeviceInfo[nIndex]._nPort = index;


			if(connectionInfoEx->ConnectionStatus == DeviceConnected)
				GetAllStringDescriptors(hHubDevice,index,&connectionInfoEx->DeviceDescriptor,_acDeviceInfo[nIndex].acDescriptor);


			strDeviceDesc = _T("");
			if(connectionInfoEx->ConnectionStatus != NoDeviceConnected)
			{
				bool		ret;
				CAtlString	strDriverKeyName;

				ret = GetDriverKeyName(hHubDevice,index,strDriverKeyName);
				if(ret)
					ret = DriverNameToDeviceDesc(strDriverKeyName,FALSE,strDeviceDesc);
				if(ret)
					_acDeviceInfo[nIndex]._strName = strDeviceDesc;

				//ATLTRACE(_T("%s\n"),strDeviceDesc);
			}


			if (connectionInfoEx->DeviceIsHub)
			{
				CAtlString	strExtHubName;

				ret = GetExternalHubName(hHubDevice,index,strExtHubName);
				if(ret && _acDeviceInfo[nIndex]._strName == _T(""))
					_acDeviceInfo[nIndex]._strName = strExtHubName;

				//����USB�n�u���
				if(ret)
					EnumerateHub(nChildTreeIndex,strExtHubName);
			}
		}
	}


	bool	EnumerateHub(UINT nTreeIndex,LPCTSTR HubName)
	{
		HANDLE                  hHubDevice;
		BOOL                    ret;
		ULONG                   nBytes;
		USB_NODE_INFORMATION	sHubInfo;
		CAtlString	strHubName;
		CAtlString	strDeviceName;

		strDeviceName = _T("\\\\.\\");
		strDeviceName += HubName;
		hHubDevice = ::CreateFile(strDeviceName,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
		if (hHubDevice == INVALID_HANDLE_VALUE)
			return	false;

		//�n�u�̎��|�[�g���擾
		::ZeroMemory(&sHubInfo,sizeof(USB_NODE_INFORMATION));
		ret = ::DeviceIoControl(hHubDevice,IOCTL_USB_GET_NODE_INFORMATION,
								  &sHubInfo,sizeof(USB_NODE_INFORMATION),
								  &sHubInfo,sizeof(USB_NODE_INFORMATION),&nBytes,NULL);

		if(ret && nTreeIndex < _acDeviceInfo.GetCount())
		{
			//USB�n�u�ڍ׏��̒ǉ��ۑ�
			_acDeviceInfo[nTreeIndex]._bNodeInfo = true;
			::memcpy(&_acDeviceInfo[nTreeIndex]._sNodeInfo,&sHubInfo,sizeof(USB_NODE_INFORMATION));
		}

		if(ret)
			EnumerateHubPorts(nTreeIndex,hHubDevice,sHubInfo.u.HubInformation.HubDescriptor.bNumberOfPorts);

		::CloseHandle(hHubDevice);

		return	ret ? true : false;
	}


public:

	bool	Test()
	{
		BOOL		ret;
		HANDLE		hHCDev;
		ULONG		index;
		ULONG		nLen;
		HDEVINFO	hDevInfo;
		IID			iidUSBHostController;
		SP_DEVICE_INTERFACE_DATA			sDeviceInterfaceData;
		PSP_DEVICE_INTERFACE_DETAIL_DATA	pDeviceDetailData;


		//USB�R���g���[���[�N���X�pGUID�𕶎��񂩂�擾
		if (S_OK != ::IIDFromString(L"{3ABF6F2D-71C4-462a-8A92-1E6861E6AF27}",&iidUSBHostController))
			return	false;

		//USB�z�X�g�R���g���[���[�̗񋓊J�n
		hDevInfo = ::SetupDiGetClassDevs(&iidUSBHostController,NULL,NULL,DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
		if(hDevInfo == INVALID_HANDLE_VALUE)
			return	false;

		sDeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

		index = 0;
		while(1)
		{
			//�z�X�g�R���g���[���[���J��
			ret = ::SetupDiEnumDeviceInterfaces(hDevInfo,0,&iidUSBHostController,index,&sDeviceInterfaceData);
			if(ret == FALSE)
				break;
			index++;

			//�ڍ׏��T�C�Y���擾
			ret = ::SetupDiGetDeviceInterfaceDetail(hDevInfo,&sDeviceInterfaceData,NULL,0,&nLen,NULL);

			pDeviceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) new BYTE[nLen];
			if(pDeviceDetailData == NULL)
				return	false;

			::ZeroMemory(pDeviceDetailData,nLen);
			pDeviceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			//�ڍ׏����擾
			ret = ::SetupDiGetDeviceInterfaceDetail(hDevInfo,&sDeviceInterfaceData,pDeviceDetailData,nLen,&nLen,NULL);
			if(ret == FALSE)
			{
				delete[]	pDeviceDetailData;
				continue;
			}

			//�z�X�g�R���g���[���[��CreateFile�ŊJ��
			hHCDev = ::CreateFile(pDeviceDetailData->DevicePath,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
			if(hHCDev == INVALID_HANDLE_VALUE)
			{
				delete[]	pDeviceDetailData;
				continue;
			}


			bool		bRet;
			CAtlString	strRootHubName;
			CAtlString	strDeviceDesc;
			CAtlString	strDriverKeyName;

			bRet = GetHCDDriverKeyName(hHCDev,strDriverKeyName);
			if(bRet)
				bRet = GetRootHubName(hHCDev,strRootHubName);
			::CloseHandle(hHCDev);


			UINT	nTreeIndex;

			if(bRet)
				bRet = DriverNameToDeviceDesc(strDriverKeyName,FALSE,strDeviceDesc);
			if(bRet)
			{
				//ATLTRACE(_T("%s\n"),strDeviceDesc);

				//�c���[�փz�X�g�R���g���[���[�̏���ǉ�
				UINT	nIndex;

				nIndex = (UINT)_acDeviceInfo.GetCount();
				_acDeviceInfo.SetCount(nIndex + 1);
				_acDeviceInfo[nIndex]._strName = strDeviceDesc;

				bRet = _treeDeviceInfoIndex.AddData(-1,&nTreeIndex,nIndex);		//���[�g�֒ǉ�
			}

			//USB�n�u�̗񋓊J�n
			if(bRet)
				EnumerateHub(nTreeIndex,strRootHubName);

			delete[]	pDeviceDetailData;
		}

		::SetupDiDestroyDeviceInfoList(hDevInfo);

		return	true;
	}


	bool	PrintTreeData(CAtlArray<UINT>* panIndex=NULL,CAtlString strPrefix=_T(""))
	{
		bool	ret;
		CAtlArray<UINT>	anRootIndex;

		if(panIndex == NULL)
		{
			ret = _treeDeviceInfoIndex.GetRootItemIndex(anRootIndex);
			if(ret == false)
				return	false;
			panIndex = &anRootIndex;
		}

		size_t	i;
		size_t	nSize;

		nSize = panIndex->GetCount();
		for(i = nSize - 1; i != -1; i--)
		{
			CInfo*	pInfo;

			if((*panIndex)[i] >= _acDeviceInfo.GetCount())
				continue;

			CAtlArray<UINT>	anNextIndex;

			pInfo = &_acDeviceInfo[(*panIndex)[i]];

			//�f�o�C�X���̕\��
			{
				CAtlString	strName;

				if(pInfo->_nPort && pInfo->_bNodeConnectionInfoEx)
					strName.Format(_T("[Port%d] %s "),pInfo->_nPort,ConnectionStatuses[pInfo->Get_pNodeConnectionInfoEx()->ConnectionStatus]);

				if(pInfo->_strName != _T(""))
					strName += pInfo->_strName;

//				ATLTRACE("%s %s\n",strPrefix,strName);
				wprintf(_T("%s %s\n"), (LPCTSTR)strPrefix, (LPCTSTR)strName);
			}

			//USB�n�u���̕\��
			if(pInfo->_bNodeInfo)
			{
				CAtlString	strMessage;
				CAtlString	strBuff;

				strBuff.Format(_T("%s    bNumberOfPorts: %d\n"), (LPCTSTR)strPrefix,pInfo->_sNodeInfo.u.HubInformation.HubDescriptor.bNumberOfPorts);
				strMessage += strBuff;
				//strBuff.Format(_T("%s    bPowerOnToPowerGood: 0x%02X\n"),strPrefix,pInfo->_sNodeInfo.u.HubInformation.HubDescriptor.bPowerOnToPowerGood);
				//strMessage += strBuff;
				strBuff.Format(_T("%s    bHubControlCurrent: 0x%02X\n"), (LPCTSTR)strPrefix,pInfo->_sNodeInfo.u.HubInformation.HubDescriptor.bHubControlCurrent);
				strMessage += strBuff;
				//strBuff.Format(_T("%s    bRemoveAndPowerMask: 0x%02X\n"),strPrefix,pInfo->_sNodeInfo.u.HubInformation.HubDescriptor.bRemoveAndPowerMask);
				//strMessage += strBuff;

//				ATLTRACE(_T("%s"),strMessage);
				wprintf(_T("%s"), (LPCTSTR)strMessage);
			}


			//�f�o�C�X�^�n�u�ڍ׏��̕\��
			if(pInfo->_bNodeConnectionInfoEx && pInfo->Get_pNodeConnectionInfoEx()->ConnectionStatus == DeviceConnected)
			{
				CAtlString	strMessage;
				CAtlString	strBuff;

				strBuff.Format(_T("%s    idVendor : 0x%04X\n"), (LPCTSTR)strPrefix,pInfo->Get_pNodeConnectionInfoEx()->DeviceDescriptor.idVendor);
				strMessage += strBuff;

				strBuff.Format(_T("%s    idProduct: 0x%04X\n"), (LPCTSTR)strPrefix, pInfo->Get_pNodeConnectionInfoEx()->DeviceDescriptor.idProduct);
				strMessage += strBuff;

				BYTE	iData;

				iData = pInfo->Get_pNodeConnectionInfoEx()->DeviceDescriptor.iProduct;
				if(iData)
				{
					CAtlString	strDescriptor;
					{
						size_t	j;
						size_t	nSize;

						nSize = pInfo->acDescriptor.GetCount();
						for(j = 0; j < nSize; j++)
						{
							if(pInfo->acDescriptor[j]._nDescriptorIndex != iData)
								continue;

							strDescriptor = pInfo->acDescriptor[j]._strDescriptor;
							break;
						}
					}
					strBuff.Format(_T("%s    iProduct: 0x%02X�i%s�j\n"), (LPCTSTR)strPrefix,iData, (LPCTSTR)strDescriptor);
					strMessage += strBuff;
				}
				iData = pInfo->Get_pNodeConnectionInfoEx()->DeviceDescriptor.iManufacturer;
				if(iData)
				{
					CAtlString	strDescriptor;
					{
						size_t	j;
						size_t	nSize;

						nSize = pInfo->acDescriptor.GetCount();
						for(j = 0; j < nSize; j++)
						{
							if(pInfo->acDescriptor[j]._nDescriptorIndex != iData)
								continue;

							strDescriptor = pInfo->acDescriptor[j]._strDescriptor;
							break;
						}
					}
					strBuff.Format(_T("%s    iManufacturer: 0x%02X�i%s�j\n"), (LPCTSTR)strPrefix,iData, (LPCTSTR)strDescriptor);
					strMessage += strBuff;
				}
				iData = pInfo->Get_pNodeConnectionInfoEx()->DeviceDescriptor.iSerialNumber;
				if(iData)
				{
					CAtlString	strDescriptor;
					{
						size_t	j;
						size_t	nSize;

						nSize = pInfo->acDescriptor.GetCount();
						for(j = 0; j < nSize; j++)
						{
							if(pInfo->acDescriptor[j]._nDescriptorIndex != iData)
								continue;

							strDescriptor = pInfo->acDescriptor[j]._strDescriptor;
							break;
						}
					}
					strBuff.Format(_T("%s    iSerialNumber: 0x%02X�i%s�j\n"), (LPCTSTR)strPrefix,iData, (LPCTSTR)strDescriptor);
					strMessage += strBuff;
				}

				//strBuff.Format(_T("%s    bcdUSB: 0x%04X\n"),strPrefix,pInfo->Get_pNodeConnectionInfoEx()->DeviceDescriptor.bcdUSB);
				//strMessage += strBuff;
				//strBuff.Format(_T("%s    bcdDevice: 0x%04X\n"),strPrefix,pInfo->Get_pNodeConnectionInfoEx()->DeviceDescriptor.bcdDevice);
				//strMessage += strBuff;
				//strBuff.Format(_T("%s    bDeviceClass: 0x%02X\n"),strPrefix,pInfo->Get_pNodeConnectionInfoEx()->DeviceDescriptor.bDeviceClass);
				//strMessage += strBuff;
				//strBuff.Format(_T("%s    bDeviceProtocol: 0x%02X\n"),strPrefix,pInfo->Get_pNodeConnectionInfoEx()->DeviceDescriptor.bDeviceProtocol);
				//strMessage += strBuff;
				//strBuff.Format(_T("%s    bDeviceSubClass: 0x%02X\n"),strPrefix,pInfo->Get_pNodeConnectionInfoEx()->DeviceDescriptor.bDeviceSubClass);
				//strMessage += strBuff;

//				ATLTRACE(_T("%s"),strMessage);
				wprintf(_T("%s"), (LPCTSTR)strMessage);
			}
			//else
			//{
			//	ATLTRACE(_T("\n"));
			//}


			ret = _treeDeviceInfoIndex.GetChildItemIndex((*panIndex)[i],anNextIndex);
			if(ret)
				PrintTreeData(&anNextIndex,strPrefix + _T("   "));
		}

		return	true;
	}


};


void	Test()
{
	CUsbInfo	cInfo;

	cInfo.Test();
	cInfo.PrintTreeData();
}


int main(int argc, wchar_t* argv[])
{

	setlocale(LC_CTYPE, "");

	Test();

	return	0;
}
