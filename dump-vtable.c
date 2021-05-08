/*
 * Vtable-Dumper 1.2
 * A tool to list content of virtual tables in a shared library
 * 
 * Copyright (C) 2013-2018 Andrey Ponomarenko's ABI laboratory
 * 
 * Written by Andrey Ponomarenko
 * 
 * PLATFORMS
 * =========
 *  Linux, FreeBSD
 * 
 * REQUIREMENTS
 * ============
 *  libelf
 *  libdl
 *  libstdc++
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
#include <cxxabi.h>
#include "dump-vtable.h"

const char* TOOL_VERSION = "1.2";

int opt_mangled = 0;
int opt_demangled = 0;

int get_VTables(char* file, vtable_info*** vtables)
{
    int fd;
    struct stat elf_stats;
    
    size_t symbol_count;
    size_t num;
    size_t i;
    char *sname, *ssname;
    vtable_info* vt;
    
    Elf *elf = NULL;
    Elf_Scn *scn = NULL;
    GElf_Shdr shdr;
    Elf_Data *edata = NULL;
    GElf_Sym sym;
    
    if ((fd = open(file, O_RDONLY))==-1)
    {
        printf("Failed to open %s\n", file);
        return ERR;
    }
    
    if ((fstat(fd, &elf_stats)))
    {
        printf("Failed to fstat %s\n", file);
        close(fd);
        return ERR;
    }
    
    if (elf_version(EV_CURRENT) == EV_NONE)
    {
        printf("WARNING Elf Library is out of date!\n");
    }
    
    if ((elf = elf_begin(fd, ELF_C_READ, NULL)) == NULL)
    {
        return ERR;
    }

    while ((scn = elf_nextscn(elf, scn)) != NULL)
    {
        if (gelf_getshdr(scn, &shdr) != &shdr)
        {
            return ERR;
        }
        
        if (shdr.sh_type == SHT_DYNSYM)
        {
            edata = elf_getdata(scn, edata);
            if (edata == NULL)
            {
                return ERR;
            }
            
            // Prevent divide by zero.
            if (shdr.sh_entsize == 0)
            {
                return ERR;
            }

            // This could truncate on a 32bit machine inspecting a 64bit binary.
            symbol_count = shdr.sh_size / shdr.sh_entsize;

            // Check for potential multiplication wraparound.
            if (symbol_count > SIZE_MAX / sizeof(vtable_info*))
            {
                return ERR;
            }

            // Add one to symbol_count for NULL storage later.
            if (SIZE_MAX - 1 < symbol_count)
            {
                return ERR;
            }
            else 
            {
                symbol_count += 1;
            }
            *vtables = (vtable_info**)malloc(sizeof(vtable_info*) * symbol_count);
            
            num = 0;
            for (i = 0; i < symbol_count; i++)
            {
                gelf_getsym(edata, i, &sym);
                
                if (sym.st_value==0)
                { // UND
                    continue;
                }

                sname = elf_strptr(elf, shdr.sh_link, sym.st_name);
                if (sname == NULL)
                {
                    return ERR;
                }

                ssname = (char*)malloc(1 + strlen(sname));
                if (ssname == NULL)
                {
                    return ERR;
                }

                strcpy(ssname, sname);
                
                if (strstr(ssname, "_ZTV"))
                {
                    vt = (vtable_info*)malloc(sizeof(vtable_info));
                    vt->name = ssname;
                    vt->size = sym.st_size;
                    
                    (*vtables)[num] = vt;
                    num += 1;
                }
                else
                {
                    free(ssname);
                }
            }
            (*vtables)[num] = NULL;
        }
    }

    elf_end(elf);
    close(fd);
    
    if (*vtables == NULL)
    {
        return ERR;
    }
    
    return 0;
}

void *
fptr2ptrp(fptr * fptr)
{
#if defined(__ia64__)
  return fptr->func;
#else
  return (void *) *fptr;
#endif
}

int integer_len (int val)
{
  int len = 1;
  while (val > 9)
  {
      len += 1;
      val /= 10;
  }
  return len;
}

char *
demangle(const char *mangled_name) {
    return __cxxabiv1::__cxa_demangle(mangled_name, NULL, 0, NULL);
}

static void print_Typeinfo_name(const char* name)
{
    char* demngl = demangle(name);
    if(opt_mangled==1 && opt_demangled==1)
    {
	printf("Inherit from %s [%s]\n", demngl, name);
    }
    else if(opt_demangled==1)
    {
	printf("Inherit from %s\n", demngl);
    }
    else
    {
	printf("Inherit from %s\n", name);
    }
    free(demngl);
}

static void print_Typeinfo(std::type_info* typeinfo)
{
    if (typeid(*typeinfo) == typeid(__cxxabiv1::__si_class_type_info))
    {
	__cxxabiv1::__si_class_type_info* info = dynamic_cast<__cxxabiv1::__si_class_type_info*>(typeinfo);
	print_Typeinfo_name(info->__base_type->name());
    }
    else if (typeid(*typeinfo) == typeid(__cxxabiv1::__vmi_class_type_info))
    {
	__cxxabiv1::__vmi_class_type_info* info = dynamic_cast<__cxxabiv1::__vmi_class_type_info*>(typeinfo);
	for(int i=0; i<info->__base_count; i++)
	{
	    print_Typeinfo_name(info->__base_info[i].__base_type->name());
	}
    }
}

void print_VTable(void *dlhndl, vtable_info *vtable)
{
    union classvtable_mem *vtablep;
    unsigned long vtbaseoffset;
    const char *vttypeinfo;
    fptr *vtvirtfuncs;
    void* vfuncp;
    Dl_info dlainfo;
    int i, j;
    int offset;
    int step;
    char* demngl;
    int size;
    int space;
    Elf32_Sym* sosymbol;
    
    demngl = demangle(vtable->name);
    demngl[0] = toupper(demngl[0]);
    printf("%s\n", demngl);
    free(demngl);
    
    vtablep = (union classvtable_mem *)dlsym(dlhndl, vtable->name);
    
    vtbaseoffset = vtablep->cat1.baseoffset;
    vttypeinfo = vtablep->cat1.typeinfo;
    vtvirtfuncs = vtablep->cat1.virtfuncs;
    
    size = vtable->size/sizeof(ptrdiff_t);
    
    step = sizeof(ptrdiff_t);
    
    offset = 0;
    if (vtbaseoffset!=0)
    {
        printf("%d     %luu\n", offset, vtbaseoffset);
    }
    else
    {
        printf("%d     %016x\n", offset, 0);
    }
    
    offset += step;
    if (dladdr1(vttypeinfo, &dlainfo, (void**)&sosymbol, RTLD_DL_SYMENT))
    {
        demngl = demangle(dlainfo.dli_sname);
        if(opt_mangled==1 && opt_demangled==1)
        {
            printf("%d     %08x (& %s) [%s]\n", offset, sosymbol->st_value, demngl, dlainfo.dli_sname);
        }
        else if(opt_demangled==1)
        {
            printf("%d     %08x (& %s)\n", offset, sosymbol->st_value, demngl);
        }
        else
        { // show mangled name by default
            printf("%d     %08x (& %s)\n", offset, sosymbol->st_value, dlainfo.dli_sname);
        }
        free(demngl);
	print_Typeinfo((std::type_info*)dlainfo.dli_saddr);
    }
    else
    {
        if (vtbaseoffset!=0)
            printf("%d     00000000 0u\n", offset);
        else
            printf("%d     00000000 0\n", offset);
    }
    
    for (i = 0; i <= size - 3; i++)
    {
        offset += step;
        vfuncp = fptr2ptrp(&vtvirtfuncs[i]);
        memset(&dlainfo, 0, sizeof(dlainfo));
        
        space = 5 - integer_len(offset);
        
        if (dladdr1(vfuncp, &dlainfo, (void**)&sosymbol, RTLD_DL_SYMENT))
        {
            printf("%d", offset);
            for (j = 0; j<=space; j++)
            {
                printf(" ");
            }
            printf("%08x ", sosymbol->st_value);
            if (dlainfo.dli_sname==NULL)
            {
                printf("%p\n", (void*) ((unsigned long)vfuncp-(unsigned long)dlainfo.dli_fbase));
            }
            else if (strstr(dlainfo.dli_sname, "__cxa_pure"))
            {
                printf("%s\n", dlainfo.dli_sname);
            }
            else if (strstr(dlainfo.dli_sname, "_ZTI"))
            {
                demngl = demangle(dlainfo.dli_sname);
                if(opt_mangled==1 && opt_demangled==1)
                {
                    printf("(& %s) [%s]\n", demngl, dlainfo.dli_sname);
                }
                else if(opt_demangled==1)
                {
                    printf("(& %s)\n", demngl);
                }
                else
                { // show mangled name by default
                    printf("(& %s)\n", dlainfo.dli_sname);
                }
                free(demngl);
            }
            else
            {
                demngl = demangle(dlainfo.dli_sname);
                
                if(opt_mangled==1 && opt_demangled==1)
                {
                    printf("%s [%s]\n", demngl, dlainfo.dli_sname);
                }
                else if(opt_demangled==1)
                {
                    printf("%s\n", demngl);
                }
                else if(opt_mangled==1)
                {
                    printf("%s\n", dlainfo.dli_sname);
                }
                else
                {
                    if(strstr(dlainfo.dli_sname, "_ZN") == NULL)
                    { // _ZThn, _ZTv, __cxa, etc.
                      // show mangled name by default
                        printf("%s\n", dlainfo.dli_sname);
                    }
                    else
                    {
                        printf("%s\n", demngl);
                    }
                }
                
                free(demngl);
            }
        }
        else
        {
            printf("%d", offset);
            for (j = 0; j<=space; j++) 
            {
                printf(" ");
            }
            if (vfuncp == NULL)
            {
                printf("0u\n");
            }
            else
            {
                if ((ptrdiff_t)vfuncp < 0)
                {
                    printf("-%016p\n", (void*) -(ptrdiff_t)vfuncp);
                }
                else
                {
                    printf("%016p\n", vfuncp);
                }
            }
        }
    }
    printf("\n\n");
}

void free_vtables(vtable_info **vtables)
{
    size_t i;
    for (i = 0; vtables[i] != NULL; i++)
    {
        free(vtables[i]->name);
        free(vtables[i]);
    }
    free(vtables);
}

int print_Info(char* file)
{
    int i;
    void *dlhndl;
    vtable_info **vtables = NULL;
    char* error;
    
    if (get_VTables(file, &vtables) == ERR)
    {
        printf("Failed to read v-tables\n");
        return ERR;
    }
    
    dlhndl = dlopen(file, RTLD_LAZY);
    if (dlhndl == NULL)
    {
        printf("Failed to dlopen %s\n", file);
        if ((error = dlerror()) != NULL)
        {
            fprintf(stderr, "%s\n", error);
        }
        free_vtables(vtables);
        return ERR;
    }
    
    for (i = 0; vtables[i] != NULL; i++)
    {
        print_VTable(dlhndl, vtables[i]);
    }
    
    free_vtables(vtables);
    
    return 0;
}

void print_Usage()
{
    printf("Vtable-Dumper %s\n", TOOL_VERSION);
    printf("A tool to list content of virtual tables in a shared library\n");
    printf("Copyright (C) 2013-2018 Andrey Ponomarenko's ABI laboratory\n");
    printf("License: GNU LGPL 2.1\n\n");
    
    printf("Usage: vtable-dumper [options] object\n");
    printf("Example: vtable-dumper /usr/lib64/libstdc++.so.6\n");
    printf("Options:\n");
    printf("  -mangled       Show mangled symbol names\n");
    printf("  -demangled     Show de-mangled symbol names\n");
    printf("  -help          Display this help message\n");
    printf("  -dumpversion   Print the tool version (%s) and don't do anything else\n\n", TOOL_VERSION);
    
    printf("More info: https://github.com/lvc/vtable-dumper\n\n");
}

int main(int argc, char *argv[])
{
    int i;
    char *opt;
    
    if(argc==1)
    {
        print_Usage();
    }
    
    for (i = 1; i < argc; i++)
    {
        opt = argv[i];
        
        if(strstr(opt, "-mangled"))
        {
            opt_mangled = 1;
        }
        else if(strstr(opt, "-demangled") || strstr(opt, "-unmangled"))
        {
            opt_demangled = 1;
        }
        else if(strstr(opt, "-h") || strstr(opt, "-v"))
        {
            print_Usage();
            return 0;
        }
        else if(strstr(opt, "-dumpversion"))
        {
            printf("%s\n", TOOL_VERSION);
            return 0;
        }
    }
    
    for (i = 1; i < argc; i++)
    {
        opt = argv[i];
        
        if(strstr(opt, "-mangled")
        || strstr(opt, "-demangled")
        || strstr(opt, "-unmangled")
        || strstr(opt, "-h")
        || strstr(opt, "-v")
        || strstr(opt, "-dumpversion")
        )
        {
            continue;
        }
        
        print_Info(opt);
    }
    
    return 0;
}
