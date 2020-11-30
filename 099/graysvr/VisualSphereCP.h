    #define _VISUALSPHERECP_H_
    
    
    template <class T>
    class CProxy_IServerObjectEvents : public IConnectionPointImpl<T, &DIID__IServerObjectEvents, CComDynamicUnkArray>
    {
        //Warning this class may be recreated by the wizard.
    public:
        VOID Fire_Shutdown()
        {
                T* pT = static_cast<T*>(this);
                int nConnectionIndex;
                int nConnections = m_vec.GetSize();
    
                for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
                {
                        pT->Lock();
                        CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
                        pT->Unlock();
                        IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
                        if (pDispatch != NULL)
                        {
                                DISPPARAMS disp = { NULL, NULL, 0, 0 };
                                pDispatch->Invoke(0x1, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
                        }
                }
    
        }
        VOID Fire_Startup()
        {
                T* pT = static_cast<T*>(this);
                int nConnectionIndex;
                int nConnections = m_vec.GetSize();
    
                for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
                {
                        pT->Lock();
                        CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
                        pT->Unlock();
                        IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
                        if (pDispatch != NULL)
                        {
                                DISPPARAMS disp = { NULL, NULL, 0, 0 };
                                pDispatch->Invoke(0x2, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
                        }
                }
    
        }
        VOID Fire_SysMessage(BSTR bstrMessage)
        {
                T* pT = static_cast<T*>(this);
                int nConnectionIndex;
                CComVariant* pvars = new CComVariant;
                int nConnections = m_vec.GetSize();
    
                for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
                {
                        pT->Lock();
                        CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
                        pT->Unlock();
                        IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
                        if (pDispatch != NULL)
                        {
                                pvars = bstrMessage;
                                DISPPARAMS disp = { pvars, NULL, 1, 0 };
                                pDispatch->Invoke(0x3, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
                        }
                }
                delete pvars;
    
        }
        VOID Fire_ClientAttach(LONG lSocket)
        {
                T* pT = static_cast<T*>(this);
                int nConnectionIndex;
                CComVariant* pvars = new CComVariant;
                int nConnections = m_vec.GetSize();
    
                for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
                {
                        pT->Lock();
                        CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
                        pT->Unlock();
                        IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
                        if (pDispatch != NULL)
                        {
                                pvars = lSocket;
                                DISPPARAMS disp = { pvars, NULL, 1, 0 };
                                pDispatch->Invoke(0x4, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
                        }
                }
                delete pvars;
    
        }
        VOID Fire_ClientDetach(LONG lSocket)
        {
                T* pT = static_cast<T*>(this);
                int nConnectionIndex;
                CComVariant* pvars = new CComVariant;
                int nConnections = m_vec.GetSize();
    
                for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
                {
                        pT->Lock();
                        CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
                        pT->Unlock();
                        IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
                        if (pDispatch != NULL)
                        {
                                pvars = lSocket;
                                DISPPARAMS disp = { pvars, NULL, 1, 0 };
                                pDispatch->Invoke(0x5, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
                        }
                }
                delete pvars;
    
        }
        VOID Fire_SaveBegin()
        {
                T* pT = static_cast<T*>(this);
                int nConnectionIndex;
                int nConnections = m_vec.GetSize();
    
                for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
                {
                        pT->Lock();
                        CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
                        pT->Unlock();
                        IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
                        if (pDispatch != NULL)
                        {
                                DISPPARAMS disp = { NULL, NULL, 0, 0 };
                                pDispatch->Invoke(0x6, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
                        }
                }
    
        }
        VOID Fire_SaveEnd()
        {
                T* pT = static_cast<T*>(this);
                int nConnectionIndex;
                int nConnections = m_vec.GetSize();
    
                for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
                {
                        pT->Lock();
                        CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
                        pT->Unlock();
                        IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
                        if (pDispatch != NULL)
                        {
                                DISPPARAMS disp = { NULL, NULL, 0, 0 };
                                pDispatch->Invoke(0x7, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
                        }
                }
    
        }
        VOID Fire_SavePercent(INT iPercent)
        {
                T* pT = static_cast<T*>(this);
                int nConnectionIndex;
                CComVariant* pvars = new CComVariant;
                int nConnections = m_vec.GetSize();
    
                for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
                {
                        pT->Lock();
                        CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
                        pT->Unlock();
                        IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
                        if (pDispatch != NULL)
                        {
                                pvars = iPercent;
                                DISPPARAMS disp = { pvars, NULL, 1, 0 };
                                pDispatch->Invoke(0x8, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
                        }
                }
                delete pvars;
    
        }
        VOID Fire_LoadBegin()
        {
                T* pT = static_cast<T*>(this);
                int nConnectionIndex;
                int nConnections = m_vec.GetSize();
    
                for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
                {
                        pT->Lock();
                        CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
                        pT->Unlock();
                        IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
                        if (pDispatch != NULL)
                        {
                                DISPPARAMS disp = { NULL, NULL, 0, 0 };
                                pDispatch->Invoke(0x9, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
                        }
                }
    
        }
        VOID Fire_LoadEnd()
        {
                T* pT = static_cast<T*>(this);
                int nConnectionIndex;
                int nConnections = m_vec.GetSize();
    
                for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
                {
                        pT->Lock();
                        CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
                        pT->Unlock();
                        IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
                        if (pDispatch != NULL)
                        {
                                DISPPARAMS disp = { NULL, NULL, 0, 0 };
                                pDispatch->Invoke(0xa, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
                        }
                }
    
        }
        VOID Fire_LoadPercent(INT iPercent)
        {
                T* pT = static_cast<T*>(this);
                int nConnectionIndex;
                CComVariant* pvars = new CComVariant;
                int nConnections = m_vec.GetSize();
    
                for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
                {
                        pT->Lock();
                        CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
                        pT->Unlock();
                        IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
                        if (pDispatch != NULL)
                        {
                                pvars = iPercent;
                                DISPPARAMS disp = { pvars, NULL, 1, 0 };
                                pDispatch->Invoke(0xb, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
                        }
                }
                delete pvars;
    
        }
    };
    
    template <class T>
    class CProxy_IAccountObjectEvents : public IConnectionPointImpl<T, &DIID__IAccountObjectEvents, CComDynamicUnkArray>
    {
        //Warning this class may be recreated by the wizard.
    public:
    };
    
    template <class T>
    class CProxy_IClientsObjectEvents : public IConnectionPointImpl<T, &DIID__IClientsObjectEvents, CComDynamicUnkArray>
    {
        //Warning this class may be recreated by the wizard.
    public:
    };