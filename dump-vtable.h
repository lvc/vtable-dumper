/*
 * Copyright (C) 2013-2018 Andrey Ponomarenko's ABI laboratory
 * 
 * Written by Andrey Ponomarenko
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301 USA
 *
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <libelf.h>
#include <gelf.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <limits.h>

#define ERR -1

#if defined(__ia64__)
typedef struct {
        void *func;
        void *gpoffset;
        } fptr;
#else
typedef unsigned long fptr;
#endif

struct cat1vtable_mem {
    unsigned long   baseoffset;
    const char  *typeinfo;
    fptr        virtfuncs[0];
};

struct cat2vtable_mem {
    unsigned long   vcalloffset;
    unsigned long   baseoffset;
    const char  *typeinfo;
    fptr        virtfuncs[0];
};

union classvtable_mem {
    struct cat1vtable_mem cat1;
    struct cat2vtable_mem cat2;
};

typedef struct 
{
    char* name;
    unsigned long size;
} vtable_info;
