#ifndef __STDDEF_H__
#define NULL ((void*)0)
#ifdef _MSC_VER
typedef unsigned long long size_t;
typedef signed long long ptrdiff_t;
#else
typedef unsigned long size_t;
typedef signed long ptrdiff_t;
#endif
#endif /* __STDDEF_H__ */