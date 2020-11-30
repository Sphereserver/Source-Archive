// ServerObject.h : Declaration of the CServerObject
    
    #ifndef __SERVEROBJECT_H_
    #define __SERVEROBJECT_H_
    
    #include "resource.h"       // main symbols
    #include "VisualSphereCP.h"
    
    /////////////////////////////////////////////////////////////////////////////
    // CServerObject
    class CServerObject;
    extern CServerObject* g_pServerObject;
    
    class ATL_NO_VTABLE CServerObject : 
        public CComObjectRootEx<CComSingleThreadModel>,
    //  public CComObjectRootEx<CComMultiThreadModel>,
        public CComCoClass<CServerObject, &CLSID_ServerObject>,
        public ISupportErrorInfo,
        public IConnectionPointContainerImpl<CServerObject>,
        public IDispatchImpl<IServerObject, &IID_IServerObject, &LIBID_VISUALSPHERELib>,
        public CProxy_IServerObjectEvents< CServerObject >
    {
    public:
        CServerObject()
        {
                g_pServerObject = this;
        }
    
    DECLARE_REGISTRY_RESOURCEID(IDR_SERVEROBJECT)
    
    DECLARE_PROTECT_FINAL_CONSTRUCT()
    
    BEGIN_COM_MAP(CServerObject)
        COM_INTERFACE_ENTRY(IServerObject)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(ISupportErrorInfo)
        COM_INTERFACE_ENTRY(IConnectionPointContainer)
        COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
    END_COM_MAP()
    BEGIN_CONNECTION_POINT_MAP(CServerObject)
    CONNECTION_POINT_ENTRY(DIID__IServerObjectEvents)
    END_CONNECTION_POINT_MAP()
    
    
    // ISupportsErrorInfo
        STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
    
    // IServerObject
    public:
        // PROPERTIES
    
        // (read only)
        STDMETHOD(get_ExitFlag)(/**/ long *pVal);
        STDMETHOD(get_ModeCode)(/**/ int *pVal);
        STDMETHOD(get_WorldSaveLocation)(/**/ BSTR *pVal);
        STDMETHOD(get_ScriptFiles)(/**/ BSTR *pVal);
        STDMETHOD(get_MulFiles)(/**/ BSTR *pVal);
        STDMETHOD(get_Name)(/**/ BSTR *pVal);
        STDMETHOD(get_Clients)(/**/ long *pVal);
    
        // (read/write)
        STDMETHOD(get_GuestsMaximum)(/**/ long *pVal);
        STDMETHOD(put_GuestsMaximum)(/**/ long newVal);
        STDMETHOD(get_MonsterFight)(/**/ VARIANT_BOOL *pVal);
        STDMETHOD(put_MonsterFight)(/**/ VARIANT_BOOL newVal);
    
        // METHODS
        STDMETHOD(Broadcast)(/**/ BSTR bstrMessage);
        STDMETHOD(Shutdown)(/**/ BSTR bstrMsg, /**/ int iMinutes);
        STDMETHOD(Startup)();
        STDMETHOD(SaveWorld)(/**/ int iMinutes);
    
        // Event firing helpers
        void Fire_OnSysMessage(BSTR bstrMsg);
    };
    
    extern bool g_bStarted;
    
    #endif //__SERVEROBJECT_H_