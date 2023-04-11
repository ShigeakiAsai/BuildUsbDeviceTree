
#pragma	once

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

template<class VALUE_TYPE>
class	CDnpTreeData
{
	CRBMap<UINT,VALUE_TYPE>	_mapItem;

	CAtlList<UINT>			_listnRootIndex;
	CRBMultiMap<UINT,UINT>	_mapnChildIndex;

	int		_nNextItemIndex;

public:
	CDnpTreeData()
	{
		_nNextItemIndex = 0;
	}

	~CDnpTreeData()
	{
	}



	//
	//	先頭データ取得
	//
	//ツリー構造に関係なく全データを取得したい時にGetNextとともに用いる
	//
	POSITION	GetHead(UINT& nItemIndex)
	{
		POSITION	pos;

		pos = _mapItem.GetHeadPosition();
		if(pos == NULL)
			return	pos;

		nItemIndex = _mapItem.GetKeyAt(pos);

		return	pos;
	}

	//
	//	次データ取得
	//
	//ツリー構造に関係なく全データを取得したい時にGetHeadとともに用いる
	//
	bool	GetNext(POSITION& pos,UINT& nItemIndex)
	{
		_mapItem.GetNext(pos);
		if(pos == NULL)
			return	false;

		nItemIndex = _mapItem.GetKeyAt(pos);
		return	true;
	}




	//
	//	データ削除
	//
	//指定したインデックスのデータを削除する。
	//ツリーの"子"構造が失われることに注意（子データは削除されない！）
	//
	bool	RemoveAt(UINT nItemIndex,bool bRemoveChild=true)
	{
		bool		ret;
		POSITION	pos;

		//TODO:bRemoveChild==trueにしか対応しない！
		ATLASSERT(bRemoveChild);

		//ツリー構造からの削除
		pos = _listnRootIndex.Find(nItemIndex);
		if(pos)
			_listnRootIndex.RemoveAt(pos);
		_mapnChildIndex.RemoveKey(nItemIndex);

		//データ削除
		ret = _mapItem.RemoveKey(nItemIndex);

		return	ret;
	}



	//
	//	データの追加
	//
	//nParentItemIndex==-1ならルートに追加
	//
	bool	AddData(UINT nParentItemIndex,UINT* pnItemIndex,VALUE_TYPE newData)
	{
		POSITION	pos;

		if(pnItemIndex == NULL)
			return	false;

		pos = _mapItem.SetAt(_nNextItemIndex,newData);
		if(pos == NULL)
			return	false;

		if(nParentItemIndex == -1)
			pos = _listnRootIndex.AddTail(_nNextItemIndex);
		else
			pos = _mapnChildIndex.Insert(nParentItemIndex,_nNextItemIndex);

		if(pos)
		{
			*pnItemIndex = _nNextItemIndex;
			_nNextItemIndex++;
			return	true;
		}

		_mapItem.RemoveKey(*pnItemIndex);

		return	false;
	}



	//
	//	データ代入
	//
	bool	SetAt(UINT nItemIndex,const VALUE_TYPE& newData)
	{
		return	_mapItem.SetAt(nItemIndex,newData) ? true : false;
	}


	//
	//	データ取得
	//
	bool	GetAt(UINT nItemIndex,VALUE_TYPE& data)
	{
		return	_mapItem.Lookup(nItemIndex,data);
	}


	//
	//	ルートアイテムのインデックス取得
	//
	bool	GetRootItemIndex(CAtlArray<UINT>& anRootIndex)
	{
		POSITION	pos;
		ULONG		nIndex;

		anRootIndex.RemoveAll();

		pos = _listnRootIndex.GetHeadPosition();
		while(pos)
		{
			nIndex = _listnRootIndex.GetAt(pos);
			anRootIndex.Add(nIndex);
			_listnRootIndex.GetNext(pos);
		}

		return	true;
	}


	//
	//	子アイテムのインデックス取得
	//
	bool	GetChildItemIndex(UINT nItemIndex,CAtlArray<UINT>& anChildIndex)
	{
		POSITION	pos;
		UINT		nIndex;

		anChildIndex.RemoveAll();

		pos = _mapnChildIndex.FindFirstWithKey(nItemIndex);
		if(pos == NULL)
			return	false;

		while(pos)
		{
			_mapnChildIndex.GetAt(pos);
			if(pos == NULL)
				continue;

			nIndex = _mapnChildIndex.GetValueAt(pos);
			anChildIndex.Add(nIndex);

			_mapnChildIndex.GetNextWithKey(pos,nItemIndex);
		}

		return	true;
	}



	//
	//	親アイテムのインデックス取得
	//
	bool	GetParentItemIndex(UINT nItemIndex,UINT* pnParentItemIndex)
	{
		POSITION	pos;
		UINT		nIndex;

		if(pnParentItemIndex == NULL)
			return	false;

		pos = _listnRootIndex.GetHeadPosition();
		while(pos)
		{
			nIndex = _listnRootIndex.GetAt(pos);
			if(nIndex == nItemIndex)
			{
				*pnParentItemIndex = -1;
				return	true;
			}

			_listnRootIndex.GetNext(pos);
		}


		pos = _mapnChildIndex.GetHeadPosition();
		while(pos)
		{
			nIndex = _mapnChildIndex.GetValueAt(pos);
			if(nIndex == nItemIndex)
			{
				*pnParentItemIndex = _mapnChildIndex.GetKeyAt(pos);
				return	true;
			}

			_mapnChildIndex.GetNext(pos);
		}

		return	false;
	}
};

