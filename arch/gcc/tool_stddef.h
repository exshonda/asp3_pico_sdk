/*
 *  TOPPERS Software
 *      Toyohashi Open Platform for Embedded Real-Time Systems
 *
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2004-2018 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *
 *  上記著作権者は，以下の(1)～(4)の条件を満たす場合に限り，本ソフトウェ
 *  ア（本ソフトウェアを改変したものを含む．以下同じ）を使用・複製・改
 *  変・再配布（以下，利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
 *      用できる形で再配布する場合には，再配布に伴うドキュメント（利用
 *      者マニュアルなど）に，上記の著作権表示，この利用条件および下記
 *      の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
 *      用できない形で再配布する場合には，次のいずれかの条件を満たすこ
 *      と．
 *    (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
 *        作権表示，この利用条件および下記の無保証規定を掲載すること．
 *    (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
 *        報告すること．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
 *      また，本ソフトウェアのユーザまたはエンドユーザからのいかなる理
 *      由に基づく請求からも，上記著作権者およびTOPPERSプロジェクトを
 *      免責すること．
 *
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，特定の使用目的
 *  に対する適合性も含めて，いかなる保証も行わない．また，本ソフトウェ
 *  アの利用により直接的または間接的に生じたいかなる損害に関しても，そ
 *  の責任を負わない．
 */

/*
 *		t_stddef.hの開発環境依存部（GCC用）
 */

#ifndef TOPPERS_TOOL_STDDEF_H
#define TOPPERS_TOOL_STDDEF_H

#ifndef __cplusplus
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
#define inline	__inline__
#endif /* !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L */
#endif /* __cplusplus */

#define Inline	static inline

#ifndef __cplusplus
#define asm		__asm__
#endif /* __cplusplus */

#define Asm		__asm__ volatile

#define NoReturn	__attribute__((__noreturn__))

#ifndef TOPPERS_MACRO_ONLY
#include <stddef.h>
#include <limits.h>
#endif /* TOPPERS_MACRO_ONLY */

#ifdef TOPPERS_STDINT_TYPE1

#ifndef TOPPERS_MACRO_ONLY

typedef signed char			int8_t;
typedef unsigned char		uint8_t;
typedef signed short		int16_t;
typedef unsigned short		uint16_t;
typedef signed int			int32_t;
typedef unsigned int		uint32_t;
typedef signed long long	int64_t;
typedef unsigned long long	uint64_t;
typedef int8_t				int_least8_t;
typedef uint8_t				uint_least8_t;
typedef long				intptr_t;
typedef unsigned long		uintptr_t;

#endif /* TOPPERS_MACRO_ONLY */

#ifndef INT8_C
#define INT8_C(val)		(val)
#endif
#ifndef UINT8_C
#define UINT8_C(val)	(val ## U)
#endif
#ifndef INT16_C
#define INT16_C(val)	(val)
#endif
#ifndef UINT16_C
#define UINT16_C(val)	(val ## U)
#endif
#ifndef INT32_C
#define INT32_C(val)	(val)
#endif
#ifndef UINT32_C
#define UINT32_C(val)	(val ## U)
#endif
#ifndef INT64_C
#define INT64_C(val)	(val ## LL)
#endif
#ifndef UINT64_C
#define UINT64_C(val)	(val ## ULL)
#endif

#define INT8_MAX		SCHAR_MAX
#define INT8_MIN		SCHAR_MIN
#define UINT8_MAX		UCHAR_MAX
#define INT16_MAX		SHRT_MAX
#define INT16_MIN		SHRT_MIN
#define UINT16_MAX		USHRT_MAX
#define INT32_MAX		INT_MAX
#define INT32_MIN		INT_MIN
#define UINT32_MAX		UINT_MAX
#define INT64_MAX		LLONG_MAX
#define INT64_MIN		LLONG_MIN
#define UINT64_MAX		ULLONG_MAX
#define INT_LEAST8_MAX	INT8_MAX
#define INT_LEAST8_MIN	INT8_MIN
#define UINT_LEAST8_MAX	INT8_MAX
#define SIZE_MAX		ULONG_MAX

#endif /* TOPPERS_STDINT_TYPE1 */

#ifdef TOPPERS_STDFLOAT_TYPE1
#ifndef TOPPERS_MACRO_ONLY

typedef float		float32_t;
typedef double		double64_t;

#endif /* TOPPERS_MACRO_ONLY */

#define FLOAT32_MIN		1.17549435e-38F
#define FLOAT32_MAX		3.40282347e+38F
#define DOUBLE64_MIN	2.2250738585072014e-308
#define DOUBLE64_MAX	1.7976931348623157e+308

#endif /* TOPPERS_STDFLOAT_TYPE1 */
#endif /* TOPPERS_TOOL_STDDEF_H */
