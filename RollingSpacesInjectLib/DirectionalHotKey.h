#pragma once
#include "hotkey.h"

class CDirectionalHotKey :
	public CHotKey
{
public:
	CDirectionalHotKey(void);
	~CDirectionalHotKey(void);

	VOID RegisterDirectionalKeys ();
	VOID UnregisterDirectionalKeys ();

	virtual VOID ProcessHotKey (const INT id) const;
private:	

	INT ivUp;
	INT ivDown;
	INT ivLeft;
	INT ivRight;
};
