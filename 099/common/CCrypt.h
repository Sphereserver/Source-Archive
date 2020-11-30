    // CCrypt.h
    //
    
    enum CONNECT_TYPE	// What type of client connection is this ?
    {
    	CONNECT_NONE,	// There is no connection.
    	CONNECT_UNK,		// client has just connected. waiting for first message.
    	CONNECT_CRYPT,		// It's a game or login protocol but i don't know which yet.
    	CONNECT_LOGIN,			// login client protocol
    	CONNECT_GAME,			// game client protocol
    	CONNECT_CONSOLE,		// we at the local console.
    	CONNECT_HTTP,			// We are serving web pages to this.
    	CONNECT_AUTO_SERVER,	// Auto listing server request.
    	CONNECT_PEER_SERVER,	// only secure listed peer servers can talk to us like this.
    	CONNECT_TELNET,			// we are in telnet console mode.
    	CONNECT_IRC,			// we are an IRC client.
    	CONNECT_PING,			// This will be dropped immediately anyhow.
    	CONNECT_GAME_PROXY,		// Just proxying this to another server. (Where the char really is)
    	CONNECT_QTY,
    };
    
    class CCompressBranch
    {
    	// For compressing/decompressing stuff from game server to client.
    	friend class CCompressTree;
    private:
    	int m_Value; // -1=tree branch, 0-255=character value, 256=PROXY_BRANCH_QTY-1=end
        CCompressBranch *m_pZero;
        CCompressBranch *m_pOne;
    private:
    	CCompressBranch()
    	{
    		m_Value=-1;	// just a pass thru branch til later.
    		m_pZero=NULL;
    		m_pOne=NULL;
    	}
    	~CCompressBranch()
    	{
    		if ( m_pOne != NULL )
    			delete m_pOne;
    		if ( m_pZero != NULL )
    			delete m_pZero;
    	}
    	bool IsLoaded() const
    	{
    		return( m_pZero != NULL );
    	}
    };
    
    #define COMPRESS_TREE_SIZE (256+1)
    
    class CCompressTree
    {
    private:
    	CCompressBranch m_Root;
    	const CCompressBranch * m_pDecodeCur;
    	int m_iDecodeIndex;
    private:
    	static const WORD sm_xCompress_Base;
    private:
    	bool AddBranch(int Value, WORD wCode, int iBits );
    public:
    	void Reset()
    	{
    		m_pDecodeCur = &m_Root;
    		m_iDecodeIndex = 0;
    	}
    	bool IsLoaded() const
    	{
    		return( m_Root.IsLoaded() );
    	}
    	bool IsCompletePacket() const
    	{
    		return( m_iDecodeIndex == 0 );
    	}
    	int GetIncompleteSize() const
    	{
    		return( m_iDecodeIndex );
    	}
    	static int Encode( BYTE * pOutput, const BYTE * pInput, int inplen );
    	bool Load();
    	int  Decode( BYTE * pOutput, const BYTE * pInput, int inpsize );
    
    	static void CompressXOR( BYTE * pData, int iLen, DWORD & dwIndex );
    };
    
    class CCryptBase
    {
    	// The old rotary encrypt/decrypt interface.
    private:
    	bool m_fInit;
    	bool m_fIgnition;		// Did ignition turn off the crypt ?
    	int m_iClientVersion;
    
    protected:
    	DWORD m_MasterHi;
    	DWORD m_MasterLo;
    
    	DWORD m_CryptMaskHi;
    	DWORD m_CryptMaskLo;
    
    	DWORD m_seed;	// seed ip we got from the client.
    
    public:
    	CCryptBase();
    	int GetClientVer() const
    	{
    		return( m_iClientVersion );
    	}
    	TCHAR* WriteClientVer( TCHAR * pStr ) const;
    
    	bool SetClientVerEnum( int iVer );
    	bool SetClientVer( LPCTSTR pszVersion );
    	void SetClientVer( const CCryptBase & crypt )
    	{
    		m_fInit = false;
    		m_iClientVersion = crypt.m_iClientVersion;
    		m_fIgnition = crypt.m_fIgnition;
    		m_MasterHi = crypt.m_MasterHi;
    	    m_MasterLo = crypt.m_MasterLo;
    	}
    
    	bool GetClientIgnition() const
    	{
    		return m_fIgnition;
    	}
    	void SetClientIgnition( bool fIgnition )
    	{
    		m_fIgnition = fIgnition;
    	}
    
    	bool IsInit() const
    	{
    		return( m_fInit );
    	}
    	bool IsValid() const
    	{
    		return( m_iClientVersion >= 0 );
    	}
    
    	void Init( DWORD dwIP );
    	virtual void Init()
    	{
    		ASSERT( m_fInit);
    		Init( m_seed );
    	}
    	void Decrypt( BYTE * pOutput, const BYTE * pInput, int iLen );
    	void Encrypt( BYTE * pOutput, const BYTE * pInput, int iLen );
    };
    
    union CCryptKey
    {
    #define CRYPT_GAMESEED_LENGTH	8
    	BYTE  u_cKey;
    	DWORD u_iKey;
    };
    
    #pragma pack(1)
    
    union CWord
    {
    	BYTE  u_ch;
    	DWORD u_dw;
    };
    struct CCryptSubData1
    {
    	BYTE  type;               //  00
    	BYTE  unused1;         //  01
    	DWORD size1;              //  04
    	BYTE  initCopy;     //  08
    	BYTE  zero;               //  48
    	BYTE  unused3;         //  49
    	DWORD size2;              //  50
    	CWord data1;        //  54
    	CWord data2;           //  74
    	DWORD data3;     //  84
    	DWORD data4; // 124
    };
    struct CCryptSubData2
    {
    	BYTE  type;         // 0x00
    	BYTE  data1  ;  // 0x01
    	BYTE  unused1;  // 0x11
    	union
    	{
    		BYTE  u_ch; // 0x18
    		DWORD u_dw;  // 0x18
    	} data2;
    };
    
    #pragma pack()
    
    struct CCryptNew
    {
    	// New crypt stuff in ver 2.0.0c
    public:
    	static const DWORD sm_InitData1;
    	static const BYTE  sm_InitData2;
    
    	static DWORD sm_CodingData;
    	static bool  sm_fInitTables;
    
    private:
    	CCryptSubData1 m_subData1;
    	CCryptSubData2 m_subData2;
    
    	BYTE  m_subData3;
    	DWORD m_pos;
    
    public:
    	void  Init(DWORD key);
    	BYTE  CodeNewByte(BYTE code);
    };
    
    struct CCrypt : public CCryptBase
    {
    	// Basic blowfish stuff.
    // #define CRYPT_AUTO_VALUE	0x80		// for SERVER_Auto
    
    #define CRYPT_GAMEKEY_COUNT		25		// CRYPT_MAX_SEQ
    #define CRYPT_GAMEKEY_LENGTH	6
    
    #define CRYPT_GAMETABLE_START	1
    #define CRYPT_GAMETABLE_STEP	3
    #define CRYPT_GAMETABLE_MODULO	11
    #define CRYPT_GAMETABLE_TRIGGER	21036
    
    protected:
    	static const BYTE sm_key_table;
    	static const BYTE sm_seed_table;
    	static bool	sm_fTablesReady;
    
    protected:
    	CONNECT_TYPE m_ConnectType;
    	int  m_gameTable;
    	int	m_gameBlockPos;		// 0-7
    	int	m_gameStreamPos;	// use this to track the 21K move to the new Blowfish m_gameTable.
    
    private:
    	static void PrepareKey( CCryptKey & key, int iTable );
    
    	CCryptKey m_Key;
    	CCryptNew m_NewCoder;	// New crypt stuff in ver 2.0.0c
    
    private:
    	BYTE EnCryptByte(BYTE data);
    	BYTE DeCryptByte(BYTE data);
    
    	void InitSeed( int iTable );
    	static void InitTables();
    
    public:
    	void Init( DWORD dwIP, CONNECT_TYPE type );
    	virtual void Init()
    	{
    		ASSERT( IsInit());
    		Init( m_seed, m_ConnectType );
    	}
    	void Decrypt( BYTE * pOutput, const BYTE * pInput, int iLen );
    	void Encrypt( BYTE * pOutput, const BYTE * pInput, int iLen );
    };