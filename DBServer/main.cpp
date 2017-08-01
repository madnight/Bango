#include "Socket/CMainSocket.h"
#include "Database/CDatabase.h"

#include <Config/CConfig.h>

int main()
{
	DBCONFIG dbconf;
	if (CConfig::Read(dbconf) == CConfig::READ_ERROR
		|| !CDatabase::Connect(dbconf.szHostname, dbconf.szPort, dbconf.szUser, dbconf.szPassword, dbconf.szSchema)
		|| !CMainSocket::Start(dbconf.wListenPort))
		return 1;

	printf(KGRN "DBServer started on port %d.\n" KNRM, dbconf.wListenPort);

	CMainSocket::Accept();

	return 0;
}
