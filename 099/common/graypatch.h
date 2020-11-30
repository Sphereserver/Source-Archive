#ifndef __STATIC_OBJECTS_H__
    #define __STATIC_OBJECTS_H__
    
    #include <windows.h>
    #include "../common/graymul.h"
    
    #pragma pack(1)
    
    
    // #define SUPERBLOCK_VERSION 0xDEAD  // Beta 1b version
    #define SUPERBLOCK_VERSION 0xBEEF	// Beta 1c version
    
    struct patchSuperblock
    {
    	DWORD dwBlockCount;
    	WORD wVersionInfo;
    };
    
    struct patchBlockHeader
    {
    	BYTE bFileID;
    	DWORD dwIndex;
    	DWORD dwLength;
    };
    
    #define PATCHFLAG_INSERT  0x00	// This is the default action
    #define PATCHFLAG_REPLACE 0x40	// Data in the patch replaces existing block -- Only applies to statics and multis
    #define PATCHFLAG_REMOVE  0x80  // Data in the patch gets removed from the block -- Only applies to statics and multis
    
    #endif