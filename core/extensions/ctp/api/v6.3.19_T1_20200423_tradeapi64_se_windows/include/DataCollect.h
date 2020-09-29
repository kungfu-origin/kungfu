#ifndef  DATA_COLLECT_H
#define  DATA_COLLECT_H

#define DLL_EXPORT   __declspec(dllexport)

#if defined(IS_WINCOLLECT_LIB) && defined(WIN32)
#ifdef LIB_DATA_COLLECT_API_EXPORT
#define DATA_COLLECT_API_EXPORT __declspec(dllexport)
#else
#define DATA_COLLECT_API_EXPORT __declspec(dllimport)
#endif
#else
#define DATA_COLLECT_API_EXPORT 
#endif


///获取AES加密和RSA加密的终端信息 
///@pSystemInfo 出参 空间需要调用者自己分配 至少270个字节
///@nLen 出参 获取到的采集信息的长度
///采集信息内可能含有‘\0’ 建议调用者使用内存复制
DATA_COLLECT_API_EXPORT int CTP_GetSystemInfo(char* pSystemInfo, int& nLen);


#endif