//
//	CDataBase
//		mySQL wrapper for easier data operations witheen in-game server
//
#ifndef CDATABASE_H
#define	CDATABASE_H

#include "../common/graycom.h"
#ifndef _DBPLUGIN
	#include <mysql.h>
	#include <errmsg.h>	// mysql standard include
#else
	#include "../common/CDatabaseLoader.h"
#endif
#include "../common/CScriptObj.h"
#include "../sphere/mutex.h"

#ifndef _DBPLUGIN
	#ifdef _WIN32
		#pragma comment(lib, "libmySQL")
	#else
		#pragma comment(lib, "libmysqlclient")
	#endif

	#define	MIN_MYSQL_VERSION_ALLOW	40115
#else
	#define DEFAULT_RESULT_SIZE 30
#endif

class CDataBase : public CScriptObj
{
public:
	static const char *m_sClassName;
	//	construction
	CDataBase();
	~CDataBase();

private:
	CDataBase(const CDataBase& copy);
	CDataBase& operator=(const CDataBase& other);

public:
	bool Connect(const char *user, const char *password, const char *base = "", const char *host = "localhost");
	bool Connect();
	void Close();							//	close link with db

	//	select
	bool query(const char *query, CVarDefMap & mapQueryResult);			//	proceeds the query for SELECT
	bool __cdecl queryf(CVarDefMap & mapQueryResult, char *fmt, ...) __printfargs(3,4);
	bool exec(const char *query);			//	executes query (pretty faster) for ALTER, UPDATE, INSERT, DELETE, ...
	bool __cdecl execf(char *fmt, ...) __printfargs(2,3);
	void addQueryResult(CGString & theFunction, CScriptTriggerArgs * theResult);

	//	set / get / info methods
	bool	isConnected();
#ifdef _DBPLUGIN

private:
	fieldarray_t * GetFieldArrayBuffer();
	int GetFieldArraySize();
	void ResizeFieldArraySize(int howmuch, bool bForceResize = false);

	resultarray_t * GetResultArrayBuffer();
	int GetResultArraySize();
	void ResizeResultArraySize(int howmuch, bool bForceResize = false);

public:

#endif

	bool OnTick();
	int FixWeirdness();

	virtual bool r_GetRef( LPCTSTR & pszKey, CScriptObj * & pRef );
	virtual bool r_LoadVal( CScript & s );
	virtual bool r_WriteVal( LPCTSTR pszKey, CGString &sVal, CTextConsole * pSrc );
	virtual bool r_Verb( CScript & s, CTextConsole * pSrc );

	LPCTSTR GetName() const
	{
		return "SQL_OBJ";
	}

public:
	CVarDefMap	m_QueryResult;
	static LPCTSTR const sm_szLoadKeys[];
	static LPCTSTR const sm_szVerbKeys[];

private:
	typedef std::pair<CGString, CScriptTriggerArgs *> FunctionArgsPair_t;
	typedef std::queue<FunctionArgsPair_t> QueueFunction_t;

protected:
#ifndef _DBPLUGIN
	bool	_bConnected;					//	are we online?
	MYSQL	*_myData;						//	mySQL link
#else
	struct __fieldarray_container
	{
		fieldarray_t * faData;
		int faDataSize;
		int faDataActualSize;
	} m_faContainer;

	struct __resultarray_container
	{
		resultarray_t * raData;
		int raDataSize;
		int raDataActualSize;
	} m_raContainer;
#endif
	QueueFunction_t m_QueryArgs;

private:
	SimpleMutex m_connectionMutex;
	SimpleMutex m_resultMutex;
	bool addQuery(bool isQuery, LPCTSTR theFunction, LPCTSTR theQuery);
};

#endif
