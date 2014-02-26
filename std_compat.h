/* std_compat.h
 *
 *   Copyright (C) 2014       Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *   MA 02110-1301, USA.
 */

#ifndef __STD_COMPAT_H
#define __STD_COMPAT_H

/* Symbols for function names/signatures for different compilers
 *
 * __FUNCTION__ -- GCC, MSVC, Intel and IBM
 * __FUNC__ -- Borland
 * __func__ -- ANSI C99
 *
 * Function Signature:
 * __PRETTY_FUNCTION__ -- GCC, MetroWerks, Digital Mars, ICC, MinGW
 * __FUNCSIG__ -- MSVC
 */

/* define __FUNC__ across platforms */

#if defined(__GNUC__)
# define __FUNC__ __FUNCTION__
#elif defined(_WIN32) || (defined(_MSC_VER) && (_MSC_VER >= 1300))
# define __FUNC__ __FUNCTION__
#elif defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)
# define __FUNC__ __FUNCTION__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
# define __FUNC__ __func__
#elif ! defined(__BORLANDC__)
# define __FUNC__ "<unknown>"
#endif

/* define __FUNC_SIG__ across platforms */

#if defined(__GNUC__)
# define __FUNC_SIG__ __PRETTY_FUNCTION__
#elif defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)
# define __FUNC_SIG__ __PRETTY_FUNCTION__
#elif defined(_FUNCSIG_) /* Windows defines this */
# define __FUNC_SIG__ _FUNCSIG_
#else
# define __FUNC_SIG__ __FUNC__
#endif

#endif /* __STD_COMPAT_H */