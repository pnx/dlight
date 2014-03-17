/* version.c
 *
 *   Copyright (C) 2013       Henrik Hautakoski <henrik@fiktivkod.org>
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
#include "version.h"

/* If you walk down a new path in the woods, you leave
   trails of where you were. The same goes for software. 
   when you compile, you no longer know from what source the binary 
   came from. Therefore we leave a trail.
   For now this is unknown but soon we will know were we walked. */

#ifndef DLIGHT_VERSION
#define DLIGHT_VERSION "Unknown"
#endif /* DLIGHT_VERSION */

char const dlight_version_str[] = "dlight: " DLIGHT_VERSION;
