#ifndef TVTEST_KEYHOOK_H
#define TVTEST_KEYHOOK_H


#ifdef __cplusplus
extern "C" {
#endif


#define KEYHOOK_MESSAGE TEXT("TVTest KeyHook")

#define KEYHOOK_LPARAM_REPEATCOUNT	0x0000FFFF
#define KEYHOOK_LPARAM_SHIFT		0x00010000
#define KEYHOOK_LPARAM_CONTROL		0x00020000
#define KEYHOOK_GET_REPEATCOUNT(lParam)	((lParam)&KEYHOOK_LPARAM_REPEATCOUNT)
#define KEYHOOK_GET_SHIFT(lParam)		(((lParam)&KEYHOOK_LPARAM_SHIFT)!=0)
#define KEYHOOK_GET_CONTROL(lParam)		(((lParam)&KEYHOOK_LPARAM_CONTROL)!=0)

typedef BOOL (WINAPI *BeginHookFunc)(HWND hwnd);
typedef BOOL (WINAPI *EndHookFunc)(void);


#ifdef __cplusplus
}
#endif


#endif