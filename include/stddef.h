#ifndef __STDDEF_H__
#define __STDDEF_H__
#ifdef __cplusplus
#define NULL nullptr
#else
#define NULL ((void*)0)
#endif
#ifdef _MSC_VER
typedef unsigned long long size_t;
typedef signed long long ptrdiff_t;
#else
typedef unsigned long size_t;
typedef signed long ptrdiff_t;
#endif
#endif /* __STDDEF_H__ */