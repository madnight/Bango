#ifndef _CSERVER_
#define _CSERVER_

#include <map>

#include <minwindef.h>

#include "CAccount.h"

typedef std::map<SOCKET, CAccount*> AccountMap;

class CServer
{
public:
	static AccountMap g_mAccount;
	static std::mutex g_mxAccount;

	static void Add(CAccount* pAccount);
	static void Remove(CAccount* pAccount);

	// Remember to call m_Access.Release() after work on account is done.
	static CAccount* FindAccount(int nCID);
	static CAccount* FindAccountByAID(int nAccountID);
	static void EmptyAccount();
};

#endif
