/* this ALWAYS GENERATED file contains the definitions for the interfaces */
    
    
    /* File created by MIDL compiler version 5.01.0164 */
    /* at Thu Oct 19 17:20:51 2000
     */
    /* Compiler settings for D:\Menace\VisualSphere\VisualSphere.idl:
        Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext
        error checks: allocation ref bounds_check enum stub_data 
    */
    //@@MIDL_FILE_HEADING(  )
    
    
    /* verify that the <rpcndr.h> version is high enough to compile this file*/
    #ifndef __REQUIRED_RPCNDR_H_VERSION__
    #define __REQUIRED_RPCNDR_H_VERSION__ 440
    #endif
    
    #include "rpc.h"
    #include "rpcndr.h"
    
    #ifndef __VisualSphere_h__
    #define __VisualSphere_h__
    
    #ifdef __cplusplus
    extern "C"{
    #endif 
    
    /* Forward Declarations */ 
    
    #ifndef __IServerObject_FWD_DEFINED__
    #define __IServerObject_FWD_DEFINED__
    typedef interface IServerObject IServerObject;
    #endif      /* __IServerObject_FWD_DEFINED__ */
    
    
    #ifndef ___IServerObjectEvents_FWD_DEFINED__
    #define ___IServerObjectEvents_FWD_DEFINED__
    typedef interface _IServerObjectEvents _IServerObjectEvents;
    #endif      /* ___IServerObjectEvents_FWD_DEFINED__ */
    
    
    #ifndef __ServerObject_FWD_DEFINED__
    #define __ServerObject_FWD_DEFINED__
    
    #ifdef __cplusplus
    typedef class ServerObject ServerObject;
    #else
    typedef struct ServerObject ServerObject;
    #endif /* __cplusplus */
    
    #endif      /* __ServerObject_FWD_DEFINED__ */
    
    
    #ifndef __IClientsObject_FWD_DEFINED__
    #define __IClientsObject_FWD_DEFINED__
    typedef interface IClientsObject IClientsObject;
    #endif      /* __IClientsObject_FWD_DEFINED__ */
    
    
    #ifndef ___IClientsObjectEvents_FWD_DEFINED__
    #define ___IClientsObjectEvents_FWD_DEFINED__
    typedef interface _IClientsObjectEvents _IClientsObjectEvents;
    #endif      /* ___IClientsObjectEvents_FWD_DEFINED__ */
    
    
    #ifndef __IAccountObject_FWD_DEFINED__
    #define __IAccountObject_FWD_DEFINED__
    typedef interface IAccountObject IAccountObject;
    #endif      /* __IAccountObject_FWD_DEFINED__ */
    
    
    #ifndef __ClientsObject_FWD_DEFINED__
    #define __ClientsObject_FWD_DEFINED__
    
    #ifdef __cplusplus
    typedef class ClientsObject ClientsObject;
    #else
    typedef struct ClientsObject ClientsObject;
    #endif /* __cplusplus */
    
    #endif      /* __ClientsObject_FWD_DEFINED__ */
    
    
    #ifndef ___IAccountObjectEvents_FWD_DEFINED__
    #define ___IAccountObjectEvents_FWD_DEFINED__
    typedef interface _IAccountObjectEvents _IAccountObjectEvents;
    #endif      /* ___IAccountObjectEvents_FWD_DEFINED__ */
    
    
    #ifndef __AccountObject_FWD_DEFINED__
    #define __AccountObject_FWD_DEFINED__
    
    #ifdef __cplusplus
    typedef class AccountObject AccountObject;
    #else
    typedef struct AccountObject AccountObject;
    #endif /* __cplusplus */
    
    #endif      /* __AccountObject_FWD_DEFINED__ */
    
    
    /* header files for imported files */
    #include "oaidl.h"
    #include "ocidl.h"
    
    void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
    void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 
    
    /* interface __MIDL_itf_VisualSphere_0000 */
    /*  */ 
    
    
    
    
    
    extern RPC_IF_HANDLE __MIDL_itf_VisualSphere_0000_v0_0_c_ifspec;
    extern RPC_IF_HANDLE __MIDL_itf_VisualSphere_0000_v0_0_s_ifspec;
    
    
    #ifndef __VISUALSPHERELib_LIBRARY_DEFINED__
    #define __VISUALSPHERELib_LIBRARY_DEFINED__
    
    /* library VISUALSPHERELib */
    /*  */ 
    
    typedef /*  */ 
    enum __MIDL___MIDL_itf_VisualSphere_0000_0001
        {       Priv_Guest      = 0,
        Priv_Player     = Priv_Guest + 1,
        Priv_Counsel    = Priv_Player + 1,
        Priv_Seer       = Priv_Counsel + 1,
        Priv_GM = Priv_Seer + 1,
        Priv_Dev        = Priv_GM + 1,
        Priv_Admin      = Priv_Dev + 1,
        Priv_Owner      = Priv_Admin + 1
        }       PRIV_TYPE;
    
    
    EXTERN_C const IID LIBID_VISUALSPHERELib;
    
    
    #ifndef __Constants_MODULE_DEFINED__
    #define __Constants_MODULE_DEFINED__
    
    
    /* module Constants */
    /*  */ 
    
    /*  */ const int vsSomeConstant     =       0;
    
    /*  */ const int vsSomeOtherConstant        =       1;
    
    #endif /* __Constants_MODULE_DEFINED__ */
    
    #ifndef __IServerObject_INTERFACE_DEFINED__
    #define __IServerObject_INTERFACE_DEFINED__
    
    /* interface IServerObject */
    /*  */ 
    
    
    EXTERN_C const IID IID_IServerObject;
    
    #if defined(__cplusplus) && !defined(CINTERFACE)
        
        MIDL_INTERFACE("EAE5AC8E-1332-11D4-838A-DFBA6891355B")
        IServerObject : public IDispatch
        {
        public:
            virtual /*  */ HRESULT STDMETHODCALLTYPE Startup( void) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE Shutdown( 
                /*  */ BSTR bstrMsg,
                /*  */ int iMinutes) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE Broadcast( 
                /*  */ BSTR bstrMessage) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE SaveWorld( 
                /*  */ int iMinutes) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_MonsterFight( 
                /*  */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE put_MonsterFight( 
                /*  */ VARIANT_BOOL newVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_Name( 
                /*  */ BSTR __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_MulFiles( 
                /*  */ BSTR __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_ScriptFiles( 
                /*  */ BSTR __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_WorldSaveLocation( 
                /*  */ BSTR __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_GuestsMaximum( 
                /*  */ long __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE put_GuestsMaximum( 
                /*  */ long newVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_ModeCode( 
                /*  */ int __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_ExitFlag( 
                /*  */ long __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_Clients( 
                /*  */ long __RPC_FAR *pVal) = 0;
            
        };
        
    #else       /* C style interface */
    
        typedef struct IServerObjectVtbl
        {
            BEGIN_INTERFACE
            
            HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
                IServerObject __RPC_FAR * This,
                /*  */ REFIID riid,
                /*  */ void __RPC_FAR *__RPC_FAR *ppvObject);
            
            ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
                IServerObject __RPC_FAR * This);
            
            ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
                IServerObject __RPC_FAR * This);
            
            HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
                IServerObject __RPC_FAR * This,
                /*  */ UINT __RPC_FAR *pctinfo);
            
            HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
                IServerObject __RPC_FAR * This,
                /*  */ UINT iTInfo,
                /*  */ LCID lcid,
                /*  */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
            
            HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
                IServerObject __RPC_FAR * This,
                /*  */ REFIID riid,
                /*  */ LPOLESTR __RPC_FAR *rgszNames,
                /*  */ UINT cNames,
                /*  */ LCID lcid,
                /*  */ DISPID __RPC_FAR *rgDispId);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
                IServerObject __RPC_FAR * This,
                /*  */ DISPID dispIdMember,
                /*  */ REFIID riid,
                /*  */ LCID lcid,
                /*  */ WORD wFlags,
                /*  */ DISPPARAMS __RPC_FAR *pDispParams,
                /*  */ VARIANT __RPC_FAR *pVarResult,
                /*  */ EXCEPINFO __RPC_FAR *pExcepInfo,
                /*  */ UINT __RPC_FAR *puArgErr);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Startup )( 
                IServerObject __RPC_FAR * This);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Shutdown )( 
                IServerObject __RPC_FAR * This,
                /*  */ BSTR bstrMsg,
                /*  */ int iMinutes);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Broadcast )( 
                IServerObject __RPC_FAR * This,
                /*  */ BSTR bstrMessage);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SaveWorld )( 
                IServerObject __RPC_FAR * This,
                /*  */ int iMinutes);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MonsterFight )( 
                IServerObject __RPC_FAR * This,
                /*  */ VARIANT_BOOL __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MonsterFight )( 
                IServerObject __RPC_FAR * This,
                /*  */ VARIANT_BOOL newVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
                IServerObject __RPC_FAR * This,
                /*  */ BSTR __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MulFiles )( 
                IServerObject __RPC_FAR * This,
                /*  */ BSTR __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ScriptFiles )( 
                IServerObject __RPC_FAR * This,
                /*  */ BSTR __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_WorldSaveLocation )( 
                IServerObject __RPC_FAR * This,
                /*  */ BSTR __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_GuestsMaximum )( 
                IServerObject __RPC_FAR * This,
                /*  */ long __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_GuestsMaximum )( 
                IServerObject __RPC_FAR * This,
                /*  */ long newVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ModeCode )( 
                IServerObject __RPC_FAR * This,
                /*  */ int __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ExitFlag )( 
                IServerObject __RPC_FAR * This,
                /*  */ long __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Clients )( 
                IServerObject __RPC_FAR * This,
                /*  */ long __RPC_FAR *pVal);
            
            END_INTERFACE
        } IServerObjectVtbl;
    
        interface IServerObject
        {
            CONST_VTBL struct IServerObjectVtbl __RPC_FAR *lpVtbl;
        };
    
        
    
    #ifdef COBJMACROS
    
    
    #define IServerObject_QueryInterface(This,riid,ppvObject)   \
        (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
    
    #define IServerObject_AddRef(This)  \
        (This)->lpVtbl -> AddRef(This)
    
    #define IServerObject_Release(This) \
        (This)->lpVtbl -> Release(This)
    
    
    #define IServerObject_GetTypeInfoCount(This,pctinfo)        \
        (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
    
    #define IServerObject_GetTypeInfo(This,iTInfo,lcid,ppTInfo) \
        (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
    
    #define IServerObject_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)       \
        (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
    
    #define IServerObject_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) \
        (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
    
    
    #define IServerObject_Startup(This) \
        (This)->lpVtbl -> Startup(This)
    
    #define IServerObject_Shutdown(This,bstrMsg,iMinutes)       \
        (This)->lpVtbl -> Shutdown(This,bstrMsg,iMinutes)
    
    #define IServerObject_Broadcast(This,bstrMessage)   \
        (This)->lpVtbl -> Broadcast(This,bstrMessage)
    
    #define IServerObject_SaveWorld(This,iMinutes)      \
        (This)->lpVtbl -> SaveWorld(This,iMinutes)
    
    #define IServerObject_get_MonsterFight(This,pVal)   \
        (This)->lpVtbl -> get_MonsterFight(This,pVal)
    
    #define IServerObject_put_MonsterFight(This,newVal) \
        (This)->lpVtbl -> put_MonsterFight(This,newVal)
    
    #define IServerObject_get_Name(This,pVal)   \
        (This)->lpVtbl -> get_Name(This,pVal)
    
    #define IServerObject_get_MulFiles(This,pVal)       \
        (This)->lpVtbl -> get_MulFiles(This,pVal)
    
    #define IServerObject_get_ScriptFiles(This,pVal)    \
        (This)->lpVtbl -> get_ScriptFiles(This,pVal)
    
    #define IServerObject_get_WorldSaveLocation(This,pVal)      \
        (This)->lpVtbl -> get_WorldSaveLocation(This,pVal)
    
    #define IServerObject_get_GuestsMaximum(This,pVal)  \
        (This)->lpVtbl -> get_GuestsMaximum(This,pVal)
    
    #define IServerObject_put_GuestsMaximum(This,newVal)        \
        (This)->lpVtbl -> put_GuestsMaximum(This,newVal)
    
    #define IServerObject_get_ModeCode(This,pVal)       \
        (This)->lpVtbl -> get_ModeCode(This,pVal)
    
    #define IServerObject_get_ExitFlag(This,pVal)       \
        (This)->lpVtbl -> get_ExitFlag(This,pVal)
    
    #define IServerObject_get_Clients(This,pVal)        \
        (This)->lpVtbl -> get_Clients(This,pVal)
    
    #endif /* COBJMACROS */
    
    
    #endif      /* C style interface */
    
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IServerObject_Startup_Proxy( 
        IServerObject __RPC_FAR * This);
    
    
    void __RPC_STUB IServerObject_Startup_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IServerObject_Shutdown_Proxy( 
        IServerObject __RPC_FAR * This,
        /*  */ BSTR bstrMsg,
        /*  */ int iMinutes);
    
    
    void __RPC_STUB IServerObject_Shutdown_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IServerObject_Broadcast_Proxy( 
        IServerObject __RPC_FAR * This,
        /*  */ BSTR bstrMessage);
    
    
    void __RPC_STUB IServerObject_Broadcast_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IServerObject_SaveWorld_Proxy( 
        IServerObject __RPC_FAR * This,
        /*  */ int iMinutes);
    
    
    void __RPC_STUB IServerObject_SaveWorld_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IServerObject_get_MonsterFight_Proxy( 
        IServerObject __RPC_FAR * This,
        /*  */ VARIANT_BOOL __RPC_FAR *pVal);
    
    
    void __RPC_STUB IServerObject_get_MonsterFight_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IServerObject_put_MonsterFight_Proxy( 
        IServerObject __RPC_FAR * This,
        /*  */ VARIANT_BOOL newVal);
    
    
    void __RPC_STUB IServerObject_put_MonsterFight_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IServerObject_get_Name_Proxy( 
        IServerObject __RPC_FAR * This,
        /*  */ BSTR __RPC_FAR *pVal);
    
    
    void __RPC_STUB IServerObject_get_Name_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IServerObject_get_MulFiles_Proxy( 
        IServerObject __RPC_FAR * This,
        /*  */ BSTR __RPC_FAR *pVal);
    
    
    void __RPC_STUB IServerObject_get_MulFiles_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IServerObject_get_ScriptFiles_Proxy( 
        IServerObject __RPC_FAR * This,
        /*  */ BSTR __RPC_FAR *pVal);
    
    
    void __RPC_STUB IServerObject_get_ScriptFiles_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IServerObject_get_WorldSaveLocation_Proxy( 
        IServerObject __RPC_FAR * This,
        /*  */ BSTR __RPC_FAR *pVal);
    
    
    void __RPC_STUB IServerObject_get_WorldSaveLocation_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IServerObject_get_GuestsMaximum_Proxy( 
        IServerObject __RPC_FAR * This,
        /*  */ long __RPC_FAR *pVal);
    
    
    void __RPC_STUB IServerObject_get_GuestsMaximum_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IServerObject_put_GuestsMaximum_Proxy( 
        IServerObject __RPC_FAR * This,
        /*  */ long newVal);
    
    
    void __RPC_STUB IServerObject_put_GuestsMaximum_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IServerObject_get_ModeCode_Proxy( 
        IServerObject __RPC_FAR * This,
        /*  */ int __RPC_FAR *pVal);
    
    
    void __RPC_STUB IServerObject_get_ModeCode_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IServerObject_get_ExitFlag_Proxy( 
        IServerObject __RPC_FAR * This,
        /*  */ long __RPC_FAR *pVal);
    
    
    void __RPC_STUB IServerObject_get_ExitFlag_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IServerObject_get_Clients_Proxy( 
        IServerObject __RPC_FAR * This,
        /*  */ long __RPC_FAR *pVal);
    
    
    void __RPC_STUB IServerObject_get_Clients_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    
    #endif      /* __IServerObject_INTERFACE_DEFINED__ */
    
    
    #ifndef ___IServerObjectEvents_DISPINTERFACE_DEFINED__
    #define ___IServerObjectEvents_DISPINTERFACE_DEFINED__
    
    /* dispinterface _IServerObjectEvents */
    /*  */ 
    
    
    EXTERN_C const IID DIID__IServerObjectEvents;
    
    #if defined(__cplusplus) && !defined(CINTERFACE)
    
        MIDL_INTERFACE("EAE5AC90-1332-11D4-838A-DFBA6891355B")
        _IServerObjectEvents : public IDispatch
        {
        };
        
    #else       /* C style interface */
    
        typedef struct _IServerObjectEventsVtbl
        {
            BEGIN_INTERFACE
            
            HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
                _IServerObjectEvents __RPC_FAR * This,
                /*  */ REFIID riid,
                /*  */ void __RPC_FAR *__RPC_FAR *ppvObject);
            
            ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
                _IServerObjectEvents __RPC_FAR * This);
            
            ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
                _IServerObjectEvents __RPC_FAR * This);
            
            HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
                _IServerObjectEvents __RPC_FAR * This,
                /*  */ UINT __RPC_FAR *pctinfo);
            
            HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
                _IServerObjectEvents __RPC_FAR * This,
                /*  */ UINT iTInfo,
                /*  */ LCID lcid,
                /*  */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
            
            HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
                _IServerObjectEvents __RPC_FAR * This,
                /*  */ REFIID riid,
                /*  */ LPOLESTR __RPC_FAR *rgszNames,
                /*  */ UINT cNames,
                /*  */ LCID lcid,
                /*  */ DISPID __RPC_FAR *rgDispId);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
                _IServerObjectEvents __RPC_FAR * This,
                /*  */ DISPID dispIdMember,
                /*  */ REFIID riid,
                /*  */ LCID lcid,
                /*  */ WORD wFlags,
                /*  */ DISPPARAMS __RPC_FAR *pDispParams,
                /*  */ VARIANT __RPC_FAR *pVarResult,
                /*  */ EXCEPINFO __RPC_FAR *pExcepInfo,
                /*  */ UINT __RPC_FAR *puArgErr);
            
            END_INTERFACE
        } _IServerObjectEventsVtbl;
    
        interface _IServerObjectEvents
        {
            CONST_VTBL struct _IServerObjectEventsVtbl __RPC_FAR *lpVtbl;
        };
    
        
    
    #ifdef COBJMACROS
    
    
    #define _IServerObjectEvents_QueryInterface(This,riid,ppvObject)    \
        (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
    
    #define _IServerObjectEvents_AddRef(This)   \
        (This)->lpVtbl -> AddRef(This)
    
    #define _IServerObjectEvents_Release(This)  \
        (This)->lpVtbl -> Release(This)
    
    
    #define _IServerObjectEvents_GetTypeInfoCount(This,pctinfo) \
        (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
    
    #define _IServerObjectEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)  \
        (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
    
    #define _IServerObjectEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)        \
        (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
    
    #define _IServerObjectEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)  \
        (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
    
    #endif /* COBJMACROS */
    
    
    #endif      /* C style interface */
    
    
    #endif      /* ___IServerObjectEvents_DISPINTERFACE_DEFINED__ */
    
    
    EXTERN_C const CLSID CLSID_ServerObject;
    
    #ifdef __cplusplus
    
    class DECLSPEC_UUID("EAE5AC8F-1332-11D4-838A-DFBA6891355B")
    ServerObject;
    #endif
    
    #ifndef __IClientsObject_INTERFACE_DEFINED__
    #define __IClientsObject_INTERFACE_DEFINED__
    
    /* interface IClientsObject */
    /*  */ 
    
    
    EXTERN_C const IID IID_IClientsObject;
    
    #if defined(__cplusplus) && !defined(CINTERFACE)
        
        MIDL_INTERFACE("BDC897C4-13E7-11D4-838A-444553540000")
        IClientsObject : public IDispatch
        {
        public:
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_AccountName( 
                /*  */ long lSocket,
                /*  */ BSTR __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE put_AccountName( 
                /*  */ long lSocket,
                /*  */ BSTR newVal) = 0;
            
        };
        
    #else       /* C style interface */
    
        typedef struct IClientsObjectVtbl
        {
            BEGIN_INTERFACE
            
            HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
                IClientsObject __RPC_FAR * This,
                /*  */ REFIID riid,
                /*  */ void __RPC_FAR *__RPC_FAR *ppvObject);
            
            ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
                IClientsObject __RPC_FAR * This);
            
            ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
                IClientsObject __RPC_FAR * This);
            
            HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
                IClientsObject __RPC_FAR * This,
                /*  */ UINT __RPC_FAR *pctinfo);
            
            HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
                IClientsObject __RPC_FAR * This,
                /*  */ UINT iTInfo,
                /*  */ LCID lcid,
                /*  */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
            
            HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
                IClientsObject __RPC_FAR * This,
                /*  */ REFIID riid,
                /*  */ LPOLESTR __RPC_FAR *rgszNames,
                /*  */ UINT cNames,
                /*  */ LCID lcid,
                /*  */ DISPID __RPC_FAR *rgDispId);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
                IClientsObject __RPC_FAR * This,
                /*  */ DISPID dispIdMember,
                /*  */ REFIID riid,
                /*  */ LCID lcid,
                /*  */ WORD wFlags,
                /*  */ DISPPARAMS __RPC_FAR *pDispParams,
                /*  */ VARIANT __RPC_FAR *pVarResult,
                /*  */ EXCEPINFO __RPC_FAR *pExcepInfo,
                /*  */ UINT __RPC_FAR *puArgErr);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AccountName )( 
                IClientsObject __RPC_FAR * This,
                /*  */ long lSocket,
                /*  */ BSTR __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_AccountName )( 
                IClientsObject __RPC_FAR * This,
                /*  */ long lSocket,
                /*  */ BSTR newVal);
            
            END_INTERFACE
        } IClientsObjectVtbl;
    
        interface IClientsObject
        {
            CONST_VTBL struct IClientsObjectVtbl __RPC_FAR *lpVtbl;
        };
    
        
    
    #ifdef COBJMACROS
    
    
    #define IClientsObject_QueryInterface(This,riid,ppvObject)  \
        (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
    
    #define IClientsObject_AddRef(This) \
        (This)->lpVtbl -> AddRef(This)
    
    #define IClientsObject_Release(This)        \
        (This)->lpVtbl -> Release(This)
    
    
    #define IClientsObject_GetTypeInfoCount(This,pctinfo)       \
        (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
    
    #define IClientsObject_GetTypeInfo(This,iTInfo,lcid,ppTInfo)        \
        (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
    
    #define IClientsObject_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)      \
        (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
    
    #define IClientsObject_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)        \
        (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
    
    
    #define IClientsObject_get_AccountName(This,lSocket,pVal)   \
        (This)->lpVtbl -> get_AccountName(This,lSocket,pVal)
    
    #define IClientsObject_put_AccountName(This,lSocket,newVal) \
        (This)->lpVtbl -> put_AccountName(This,lSocket,newVal)
    
    #endif /* COBJMACROS */
    
    
    #endif      /* C style interface */
    
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IClientsObject_get_AccountName_Proxy( 
        IClientsObject __RPC_FAR * This,
        /*  */ long lSocket,
        /*  */ BSTR __RPC_FAR *pVal);
    
    
    void __RPC_STUB IClientsObject_get_AccountName_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IClientsObject_put_AccountName_Proxy( 
        IClientsObject __RPC_FAR * This,
        /*  */ long lSocket,
        /*  */ BSTR newVal);
    
    
    void __RPC_STUB IClientsObject_put_AccountName_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    
    #endif      /* __IClientsObject_INTERFACE_DEFINED__ */
    
    
    #ifndef ___IClientsObjectEvents_DISPINTERFACE_DEFINED__
    #define ___IClientsObjectEvents_DISPINTERFACE_DEFINED__
    
    /* dispinterface _IClientsObjectEvents */
    /*  */ 
    
    
    EXTERN_C const IID DIID__IClientsObjectEvents;
    
    #if defined(__cplusplus) && !defined(CINTERFACE)
    
        MIDL_INTERFACE("BDC897C6-13E7-11D4-838A-444553540000")
        _IClientsObjectEvents : public IDispatch
        {
        };
        
    #else       /* C style interface */
    
        typedef struct _IClientsObjectEventsVtbl
        {
            BEGIN_INTERFACE
            
            HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
                _IClientsObjectEvents __RPC_FAR * This,
                /*  */ REFIID riid,
                /*  */ void __RPC_FAR *__RPC_FAR *ppvObject);
            
            ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
                _IClientsObjectEvents __RPC_FAR * This);
            
            ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
                _IClientsObjectEvents __RPC_FAR * This);
            
            HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
                _IClientsObjectEvents __RPC_FAR * This,
                /*  */ UINT __RPC_FAR *pctinfo);
            
            HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
                _IClientsObjectEvents __RPC_FAR * This,
                /*  */ UINT iTInfo,
                /*  */ LCID lcid,
                /*  */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
            
            HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
                _IClientsObjectEvents __RPC_FAR * This,
                /*  */ REFIID riid,
                /*  */ LPOLESTR __RPC_FAR *rgszNames,
                /*  */ UINT cNames,
                /*  */ LCID lcid,
                /*  */ DISPID __RPC_FAR *rgDispId);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
                _IClientsObjectEvents __RPC_FAR * This,
                /*  */ DISPID dispIdMember,
                /*  */ REFIID riid,
                /*  */ LCID lcid,
                /*  */ WORD wFlags,
                /*  */ DISPPARAMS __RPC_FAR *pDispParams,
                /*  */ VARIANT __RPC_FAR *pVarResult,
                /*  */ EXCEPINFO __RPC_FAR *pExcepInfo,
                /*  */ UINT __RPC_FAR *puArgErr);
            
            END_INTERFACE
        } _IClientsObjectEventsVtbl;
    
        interface _IClientsObjectEvents
        {
            CONST_VTBL struct _IClientsObjectEventsVtbl __RPC_FAR *lpVtbl;
        };
    
        
    
    #ifdef COBJMACROS
    
    
    #define _IClientsObjectEvents_QueryInterface(This,riid,ppvObject)   \
        (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
    
    #define _IClientsObjectEvents_AddRef(This)  \
        (This)->lpVtbl -> AddRef(This)
    
    #define _IClientsObjectEvents_Release(This) \
        (This)->lpVtbl -> Release(This)
    
    
    #define _IClientsObjectEvents_GetTypeInfoCount(This,pctinfo)        \
        (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
    
    #define _IClientsObjectEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo) \
        (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
    
    #define _IClientsObjectEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)       \
        (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
    
    #define _IClientsObjectEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) \
        (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
    
    #endif /* COBJMACROS */
    
    
    #endif      /* C style interface */
    
    
    #endif      /* ___IClientsObjectEvents_DISPINTERFACE_DEFINED__ */
    
    
    #ifndef __IAccountObject_INTERFACE_DEFINED__
    #define __IAccountObject_INTERFACE_DEFINED__
    
    /* interface IAccountObject */
    /*  */ 
    
    
    EXTERN_C const IID IID_IAccountObject;
    
    #if defined(__cplusplus) && !defined(CINTERFACE)
        
        MIDL_INTERFACE("C7FE0E75-1580-11D4-838A-444553540000")
        IAccountObject : public IDispatch
        {
        public:
            virtual /*  */ HRESULT STDMETHODCALLTYPE COMInit( 
                /*  */ long lSocket) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_Name( 
                /*  */ BSTR __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_Password( 
                /*  */ BSTR __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE put_Password( 
                /*  */ BSTR newVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_PLevel( 
                /*  */ PRIV_TYPE __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE put_PLevel( 
                /*  */ PRIV_TYPE newVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_Language( 
                /*  */ BSTR __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_LastIP( 
                /*  */ long __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_FirstIP( 
                /*  */ long __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_TotalConnectTime( 
                /*  */ DATE __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_LastConnectTime( 
                /*  */ DATE __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_FirstConnectDate( 
                /*  */ DATE __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_LastConnectDate( 
                /*  */ DATE __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_LastCharUID( 
                /*  */ long __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_EMailAddress( 
                /*  */ BSTR __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_EMailFailures( 
                /*  */ int __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_ChatName( 
                /*  */ BSTR __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_GMComments( 
                /*  */ BSTR __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_EMailMessage( 
                /*  */ int __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE put_EMailMessage( 
                /*  */ int newVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_Block( 
                /*  */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE put_Block( 
                /*  */ VARIANT_BOOL newVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_Guest( 
                /*  */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE put_Guest( 
                /*  */ VARIANT_BOOL newVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_Jail( 
                /*  */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE put_Jail( 
                /*  */ VARIANT_BOOL newVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_T2A( 
                /*  */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE put_T2A( 
                /*  */ VARIANT_BOOL newVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE get_Priv( 
                /*  */ long __RPC_FAR *pVal) = 0;
            
            virtual /*  */ HRESULT STDMETHODCALLTYPE put_Priv( 
                /*  */ long newVal) = 0;
            
        };
        
    #else       /* C style interface */
    
        typedef struct IAccountObjectVtbl
        {
            BEGIN_INTERFACE
            
            HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
                IAccountObject __RPC_FAR * This,
                /*  */ REFIID riid,
                /*  */ void __RPC_FAR *__RPC_FAR *ppvObject);
            
            ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
                IAccountObject __RPC_FAR * This);
            
            ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
                IAccountObject __RPC_FAR * This);
            
            HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
                IAccountObject __RPC_FAR * This,
                /*  */ UINT __RPC_FAR *pctinfo);
            
            HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
                IAccountObject __RPC_FAR * This,
                /*  */ UINT iTInfo,
                /*  */ LCID lcid,
                /*  */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
            
            HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
                IAccountObject __RPC_FAR * This,
                /*  */ REFIID riid,
                /*  */ LPOLESTR __RPC_FAR *rgszNames,
                /*  */ UINT cNames,
                /*  */ LCID lcid,
                /*  */ DISPID __RPC_FAR *rgDispId);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
                IAccountObject __RPC_FAR * This,
                /*  */ DISPID dispIdMember,
                /*  */ REFIID riid,
                /*  */ LCID lcid,
                /*  */ WORD wFlags,
                /*  */ DISPPARAMS __RPC_FAR *pDispParams,
                /*  */ VARIANT __RPC_FAR *pVarResult,
                /*  */ EXCEPINFO __RPC_FAR *pExcepInfo,
                /*  */ UINT __RPC_FAR *puArgErr);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *COMInit )( 
                IAccountObject __RPC_FAR * This,
                /*  */ long lSocket);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
                IAccountObject __RPC_FAR * This,
                /*  */ BSTR __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Password )( 
                IAccountObject __RPC_FAR * This,
                /*  */ BSTR __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Password )( 
                IAccountObject __RPC_FAR * This,
                /*  */ BSTR newVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_PLevel )( 
                IAccountObject __RPC_FAR * This,
                /*  */ PRIV_TYPE __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_PLevel )( 
                IAccountObject __RPC_FAR * This,
                /*  */ PRIV_TYPE newVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Language )( 
                IAccountObject __RPC_FAR * This,
                /*  */ BSTR __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LastIP )( 
                IAccountObject __RPC_FAR * This,
                /*  */ long __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_FirstIP )( 
                IAccountObject __RPC_FAR * This,
                /*  */ long __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_TotalConnectTime )( 
                IAccountObject __RPC_FAR * This,
                /*  */ DATE __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LastConnectTime )( 
                IAccountObject __RPC_FAR * This,
                /*  */ DATE __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_FirstConnectDate )( 
                IAccountObject __RPC_FAR * This,
                /*  */ DATE __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LastConnectDate )( 
                IAccountObject __RPC_FAR * This,
                /*  */ DATE __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LastCharUID )( 
                IAccountObject __RPC_FAR * This,
                /*  */ long __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_EMailAddress )( 
                IAccountObject __RPC_FAR * This,
                /*  */ BSTR __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_EMailFailures )( 
                IAccountObject __RPC_FAR * This,
                /*  */ int __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ChatName )( 
                IAccountObject __RPC_FAR * This,
                /*  */ BSTR __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_GMComments )( 
                IAccountObject __RPC_FAR * This,
                /*  */ BSTR __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_EMailMessage )( 
                IAccountObject __RPC_FAR * This,
                /*  */ int __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_EMailMessage )( 
                IAccountObject __RPC_FAR * This,
                /*  */ int newVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Block )( 
                IAccountObject __RPC_FAR * This,
                /*  */ VARIANT_BOOL __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Block )( 
                IAccountObject __RPC_FAR * This,
                /*  */ VARIANT_BOOL newVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Guest )( 
                IAccountObject __RPC_FAR * This,
                /*  */ VARIANT_BOOL __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Guest )( 
                IAccountObject __RPC_FAR * This,
                /*  */ VARIANT_BOOL newVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Jail )( 
                IAccountObject __RPC_FAR * This,
                /*  */ VARIANT_BOOL __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Jail )( 
                IAccountObject __RPC_FAR * This,
                /*  */ VARIANT_BOOL newVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_T2A )( 
                IAccountObject __RPC_FAR * This,
                /*  */ VARIANT_BOOL __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_T2A )( 
                IAccountObject __RPC_FAR * This,
                /*  */ VARIANT_BOOL newVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Priv )( 
                IAccountObject __RPC_FAR * This,
                /*  */ long __RPC_FAR *pVal);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Priv )( 
                IAccountObject __RPC_FAR * This,
                /*  */ long newVal);
            
            END_INTERFACE
        } IAccountObjectVtbl;
    
        interface IAccountObject
        {
            CONST_VTBL struct IAccountObjectVtbl __RPC_FAR *lpVtbl;
        };
    
        
    
    #ifdef COBJMACROS
    
    
    #define IAccountObject_QueryInterface(This,riid,ppvObject)  \
        (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
    
    #define IAccountObject_AddRef(This) \
        (This)->lpVtbl -> AddRef(This)
    
    #define IAccountObject_Release(This)        \
        (This)->lpVtbl -> Release(This)
    
    
    #define IAccountObject_GetTypeInfoCount(This,pctinfo)       \
        (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
    
    #define IAccountObject_GetTypeInfo(This,iTInfo,lcid,ppTInfo)        \
        (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
    
    #define IAccountObject_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)      \
        (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
    
    #define IAccountObject_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)        \
        (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
    
    
    #define IAccountObject_COMInit(This,lSocket)        \
        (This)->lpVtbl -> COMInit(This,lSocket)
    
    #define IAccountObject_get_Name(This,pVal)  \
        (This)->lpVtbl -> get_Name(This,pVal)
    
    #define IAccountObject_get_Password(This,pVal)      \
        (This)->lpVtbl -> get_Password(This,pVal)
    
    #define IAccountObject_put_Password(This,newVal)    \
        (This)->lpVtbl -> put_Password(This,newVal)
    
    #define IAccountObject_get_PLevel(This,pVal)        \
        (This)->lpVtbl -> get_PLevel(This,pVal)
    
    #define IAccountObject_put_PLevel(This,newVal)      \
        (This)->lpVtbl -> put_PLevel(This,newVal)
    
    #define IAccountObject_get_Language(This,pVal)      \
        (This)->lpVtbl -> get_Language(This,pVal)
    
    #define IAccountObject_get_LastIP(This,pVal)        \
        (This)->lpVtbl -> get_LastIP(This,pVal)
    
    #define IAccountObject_get_FirstIP(This,pVal)       \
        (This)->lpVtbl -> get_FirstIP(This,pVal)
    
    #define IAccountObject_get_TotalConnectTime(This,pVal)      \
        (This)->lpVtbl -> get_TotalConnectTime(This,pVal)
    
    #define IAccountObject_get_LastConnectTime(This,pVal)       \
        (This)->lpVtbl -> get_LastConnectTime(This,pVal)
    
    #define IAccountObject_get_FirstConnectDate(This,pVal)      \
        (This)->lpVtbl -> get_FirstConnectDate(This,pVal)
    
    #define IAccountObject_get_LastConnectDate(This,pVal)       \
        (This)->lpVtbl -> get_LastConnectDate(This,pVal)
    
    #define IAccountObject_get_LastCharUID(This,pVal)   \
        (This)->lpVtbl -> get_LastCharUID(This,pVal)
    
    #define IAccountObject_get_EMailAddress(This,pVal)  \
        (This)->lpVtbl -> get_EMailAddress(This,pVal)
    
    #define IAccountObject_get_EMailFailures(This,pVal) \
        (This)->lpVtbl -> get_EMailFailures(This,pVal)
    
    #define IAccountObject_get_ChatName(This,pVal)      \
        (This)->lpVtbl -> get_ChatName(This,pVal)
    
    #define IAccountObject_get_GMComments(This,pVal)    \
        (This)->lpVtbl -> get_GMComments(This,pVal)
    
    #define IAccountObject_get_EMailMessage(This,pVal)  \
        (This)->lpVtbl -> get_EMailMessage(This,pVal)
    
    #define IAccountObject_put_EMailMessage(This,newVal)        \
        (This)->lpVtbl -> put_EMailMessage(This,newVal)
    
    #define IAccountObject_get_Block(This,pVal) \
        (This)->lpVtbl -> get_Block(This,pVal)
    
    #define IAccountObject_put_Block(This,newVal)       \
        (This)->lpVtbl -> put_Block(This,newVal)
    
    #define IAccountObject_get_Guest(This,pVal) \
        (This)->lpVtbl -> get_Guest(This,pVal)
    
    #define IAccountObject_put_Guest(This,newVal)       \
        (This)->lpVtbl -> put_Guest(This,newVal)
    
    #define IAccountObject_get_Jail(This,pVal)  \
        (This)->lpVtbl -> get_Jail(This,pVal)
    
    #define IAccountObject_put_Jail(This,newVal)        \
        (This)->lpVtbl -> put_Jail(This,newVal)
    
    #define IAccountObject_get_T2A(This,pVal)   \
        (This)->lpVtbl -> get_T2A(This,pVal)
    
    #define IAccountObject_put_T2A(This,newVal) \
        (This)->lpVtbl -> put_T2A(This,newVal)
    
    #define IAccountObject_get_Priv(This,pVal)  \
        (This)->lpVtbl -> get_Priv(This,pVal)
    
    #define IAccountObject_put_Priv(This,newVal)        \
        (This)->lpVtbl -> put_Priv(This,newVal)
    
    #endif /* COBJMACROS */
    
    
    #endif      /* C style interface */
    
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_COMInit_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ long lSocket);
    
    
    void __RPC_STUB IAccountObject_COMInit_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_get_Name_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ BSTR __RPC_FAR *pVal);
    
    
    void __RPC_STUB IAccountObject_get_Name_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_get_Password_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ BSTR __RPC_FAR *pVal);
    
    
    void __RPC_STUB IAccountObject_get_Password_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_put_Password_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ BSTR newVal);
    
    
    void __RPC_STUB IAccountObject_put_Password_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_get_PLevel_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ PRIV_TYPE __RPC_FAR *pVal);
    
    
    void __RPC_STUB IAccountObject_get_PLevel_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_put_PLevel_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ PRIV_TYPE newVal);
    
    
    void __RPC_STUB IAccountObject_put_PLevel_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_get_Language_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ BSTR __RPC_FAR *pVal);
    
    
    void __RPC_STUB IAccountObject_get_Language_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_get_LastIP_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ long __RPC_FAR *pVal);
    
    
    void __RPC_STUB IAccountObject_get_LastIP_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_get_FirstIP_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ long __RPC_FAR *pVal);
    
    
    void __RPC_STUB IAccountObject_get_FirstIP_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_get_TotalConnectTime_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ DATE __RPC_FAR *pVal);
    
    
    void __RPC_STUB IAccountObject_get_TotalConnectTime_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_get_LastConnectTime_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ DATE __RPC_FAR *pVal);
    
    
    void __RPC_STUB IAccountObject_get_LastConnectTime_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_get_FirstConnectDate_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ DATE __RPC_FAR *pVal);
    
    
    void __RPC_STUB IAccountObject_get_FirstConnectDate_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_get_LastConnectDate_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ DATE __RPC_FAR *pVal);
    
    
    void __RPC_STUB IAccountObject_get_LastConnectDate_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_get_LastCharUID_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ long __RPC_FAR *pVal);
    
    
    void __RPC_STUB IAccountObject_get_LastCharUID_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_get_EMailAddress_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ BSTR __RPC_FAR *pVal);
    
    
    void __RPC_STUB IAccountObject_get_EMailAddress_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_get_EMailFailures_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ int __RPC_FAR *pVal);
    
    
    void __RPC_STUB IAccountObject_get_EMailFailures_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_get_ChatName_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ BSTR __RPC_FAR *pVal);
    
    
    void __RPC_STUB IAccountObject_get_ChatName_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_get_GMComments_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ BSTR __RPC_FAR *pVal);
    
    
    void __RPC_STUB IAccountObject_get_GMComments_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_get_EMailMessage_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ int __RPC_FAR *pVal);
    
    
    void __RPC_STUB IAccountObject_get_EMailMessage_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_put_EMailMessage_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ int newVal);
    
    
    void __RPC_STUB IAccountObject_put_EMailMessage_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_get_Block_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ VARIANT_BOOL __RPC_FAR *pVal);
    
    
    void __RPC_STUB IAccountObject_get_Block_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_put_Block_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ VARIANT_BOOL newVal);
    
    
    void __RPC_STUB IAccountObject_put_Block_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_get_Guest_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ VARIANT_BOOL __RPC_FAR *pVal);
    
    
    void __RPC_STUB IAccountObject_get_Guest_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_put_Guest_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ VARIANT_BOOL newVal);
    
    
    void __RPC_STUB IAccountObject_put_Guest_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_get_Jail_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ VARIANT_BOOL __RPC_FAR *pVal);
    
    
    void __RPC_STUB IAccountObject_get_Jail_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_put_Jail_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ VARIANT_BOOL newVal);
    
    
    void __RPC_STUB IAccountObject_put_Jail_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_get_T2A_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ VARIANT_BOOL __RPC_FAR *pVal);
    
    
    void __RPC_STUB IAccountObject_get_T2A_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_put_T2A_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ VARIANT_BOOL newVal);
    
    
    void __RPC_STUB IAccountObject_put_T2A_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_get_Priv_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ long __RPC_FAR *pVal);
    
    
    void __RPC_STUB IAccountObject_get_Priv_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    /*  */ HRESULT STDMETHODCALLTYPE IAccountObject_put_Priv_Proxy( 
        IAccountObject __RPC_FAR * This,
        /*  */ long newVal);
    
    
    void __RPC_STUB IAccountObject_put_Priv_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    
    
    #endif      /* __IAccountObject_INTERFACE_DEFINED__ */
    
    
    EXTERN_C const CLSID CLSID_ClientsObject;
    
    #ifdef __cplusplus
    
    class DECLSPEC_UUID("C7FE0E63-1580-11D4-838A-444553540000")
    ClientsObject;
    #endif
    
    #ifndef ___IAccountObjectEvents_DISPINTERFACE_DEFINED__
    #define ___IAccountObjectEvents_DISPINTERFACE_DEFINED__
    
    /* dispinterface _IAccountObjectEvents */
    /*  */ 
    
    
    EXTERN_C const IID DIID__IAccountObjectEvents;
    
    #if defined(__cplusplus) && !defined(CINTERFACE)
    
        MIDL_INTERFACE("C7FE0E77-1580-11D4-838A-444553540000")
        _IAccountObjectEvents : public IDispatch
        {
        };
        
    #else       /* C style interface */
    
        typedef struct _IAccountObjectEventsVtbl
        {
            BEGIN_INTERFACE
            
            HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
                _IAccountObjectEvents __RPC_FAR * This,
                /*  */ REFIID riid,
                /*  */ void __RPC_FAR *__RPC_FAR *ppvObject);
            
            ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
                _IAccountObjectEvents __RPC_FAR * This);
            
            ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
                _IAccountObjectEvents __RPC_FAR * This);
            
            HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
                _IAccountObjectEvents __RPC_FAR * This,
                /*  */ UINT __RPC_FAR *pctinfo);
            
            HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
                _IAccountObjectEvents __RPC_FAR * This,
                /*  */ UINT iTInfo,
                /*  */ LCID lcid,
                /*  */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
            
            HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
                _IAccountObjectEvents __RPC_FAR * This,
                /*  */ REFIID riid,
                /*  */ LPOLESTR __RPC_FAR *rgszNames,
                /*  */ UINT cNames,
                /*  */ LCID lcid,
                /*  */ DISPID __RPC_FAR *rgDispId);
            
            /*  */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
                _IAccountObjectEvents __RPC_FAR * This,
                /*  */ DISPID dispIdMember,
                /*  */ REFIID riid,
                /*  */ LCID lcid,
                /*  */ WORD wFlags,
                /*  */ DISPPARAMS __RPC_FAR *pDispParams,
                /*  */ VARIANT __RPC_FAR *pVarResult,
                /*  */ EXCEPINFO __RPC_FAR *pExcepInfo,
                /*  */ UINT __RPC_FAR *puArgErr);
            
            END_INTERFACE
        } _IAccountObjectEventsVtbl;
    
        interface _IAccountObjectEvents
        {
            CONST_VTBL struct _IAccountObjectEventsVtbl __RPC_FAR *lpVtbl;
        };
    
        
    
    #ifdef COBJMACROS
    
    
    #define _IAccountObjectEvents_QueryInterface(This,riid,ppvObject)   \
        (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
    
    #define _IAccountObjectEvents_AddRef(This)  \
        (This)->lpVtbl -> AddRef(This)
    
    #define _IAccountObjectEvents_Release(This) \
        (This)->lpVtbl -> Release(This)
    
    
    #define _IAccountObjectEvents_GetTypeInfoCount(This,pctinfo)        \
        (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
    
    #define _IAccountObjectEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo) \
        (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
    
    #define _IAccountObjectEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)       \
        (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
    
    #define _IAccountObjectEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) \
        (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
    
    #endif /* COBJMACROS */
    
    
    #endif      /* C style interface */
    
    
    #endif      /* ___IAccountObjectEvents_DISPINTERFACE_DEFINED__ */
    
    
    EXTERN_C const CLSID CLSID_AccountObject;
    
    #ifdef __cplusplus
    
    class DECLSPEC_UUID("C7FE0E76-1580-11D4-838A-444553540000")
    AccountObject;
    #endif
    #endif /* __VISUALSPHERELib_LIBRARY_DEFINED__ */
    
    /* Additional Prototypes for ALL interfaces */
    
    /* end of Additional Prototypes */
    
    #ifdef __cplusplus
    }
    #endif
    
    #endif