#include "kernel_print.h"
#include "stab.h"
#include <console.h>
#include <x86.h>
#include <string.h>

#define STACK_FRAME_DEPTH 20

extern const struct Stab __STAB_BEGIN__[]; // beginning of stabs table
extern const struct Stab __STAB_END__[];   // end of stabs table
extern const char __STAB_STR_BEGIN__[];     // beginning of string table
extern const char __STAB_STR_END__[];       // end of string table

struct EipDebugInfo
{
    const char *eip_file;    // source code filename for eip
    int eip_line;            // source code line number for eip
    const char *eip_fn_name; // name of function containing eip
    int eip_fn_namelen;      // length of function's name
    uintptr eip_fn_addr;   // start address of function
    int eip_fn_n_arg;         // number of function arguments
};

static void stab_binary_search(const struct Stab *stabs, int *region_left, int *region_right, int type, uintptr addr)
{
    int l = *region_left, r = *region_right, any_matches = 0;

    while (l <= r)
    {
        int true_m = (l + r) / 2, m = true_m;

        // search for earliest stab with right type
        while (m >= l && stabs[m].n_type != type)
        {
            m--;
        }
        if (m < l)
        { // no match in [l, m]
            l = true_m + 1;
            continue;
        }

        // actual binary search
        any_matches = 1;
        if (stabs[m].n_value < addr)
        {
            *region_left = m;
            l = true_m + 1;
        }
        else if (stabs[m].n_value > addr)
        {
            *region_right = m - 1;
            r = m - 1;
        }
        else
        {
            // exact match for 'addr', but continue loop to find
            // *region_right
            *region_left = m;
            l = m;
            addr++;
        }
    }

    if (!any_matches)
    {
        *region_right = *region_left - 1;
    }
    else
    {
        // find rightmost region containing 'addr'
        l = *region_right;
        for (; l > *region_left && stabs[l].n_type != type; l--)
            /* do nothing */;
        *region_left = l;
    }
}

static int debug_info_eip(uintptr addr, struct EipDebugInfo *info)
{
    const struct Stab *stabs, *stab_end;
    const char *stab_str, *stab_str_end;

    info->eip_file = "<unknown>";
    info->eip_line = 0;
    info->eip_fn_name = "<unknown>";
    info->eip_fn_namelen = 9;
    info->eip_fn_addr = addr;
    info->eip_fn_n_arg = 0;

    stabs = __STAB_BEGIN__;
    stab_end = __STAB_END__;
    stab_str = __STAB_STR_BEGIN__;
    stab_str_end = __STAB_STR_END__;

    // String table validity checks
    if (stab_str_end <= stab_str || stab_str_end[-1] != 0)
    {
        return -1;
    }

    // Now we find the right stabs that define the function containing
    // 'eip'.  First, we find the basic source file containing 'eip'.
    // Then, we look in that source file for the function.  Then we look
    // for the line number.

    // Search the entire set of stabs for the source file (type N_SO).
    int lfile = 0, rfile = (stab_end - stabs) - 1;
    stab_binary_search(stabs, &lfile, &rfile, N_SO, addr);
    if (lfile == 0)
        return -1;

    // Search within that file's stabs for the function definition
    // (N_FUN).
    int lfun = lfile, rfun = rfile;
    int lline, rline;
    stab_binary_search(stabs, &lfun, &rfun, N_FUN, addr);

    if (lfun <= rfun)
    {
        // stabs[lfun] points to the function name
        // in the string table, but check bounds just in case.
        if (stabs[lfun].n_strx < stab_str_end - stab_str)
        {
            info->eip_fn_name = stab_str + stabs[lfun].n_strx;
        }
        info->eip_fn_addr = stabs[lfun].n_value;
        addr -= info->eip_fn_addr;
        // Search within the function definition for the line number.
        lline = lfun;
        rline = rfun;
    }
    else
    {
        // Couldn't find function stab!  Maybe we're in an assembly
        // file.  Search the whole file for the line number.
        info->eip_fn_addr = addr;
        lline = lfile;
        rline = rfile;
    }
    info->eip_fn_namelen = string_find(info->eip_fn_name, ':') - info->eip_fn_name;

    // Search within [lline, rline] for the line number stab.
    // If found, set info->eip_line to the right line number.
    // If not found, return -1.
    stab_binary_search(stabs, &lline, &rline, N_SLINE, addr);
    if (lline <= rline)
    {
        info->eip_line = stabs[rline].n_desc;
    }
    else
    {
        return -1;
    }

    // Search backwards from the line number for the relevant filename stab.
    // We can't just use the "lfile" stab because inlined functions
    // can interpolate code from a different file!
    // Such included source files use the N_SOL stab type.
    while (lline >= lfile && stabs[lline].n_type != N_SOL && (stabs[lline].n_type != N_SO || !stabs[lline].n_value))
    {
        lline--;
    }
    if (lline >= lfile && stabs[lline].n_strx < stab_str_end - stab_str)
    {
        info->eip_file = stab_str + stabs[lline].n_strx;
    }

    // Set eip_fn_narg to the number of arguments taken by the function,
    // or 0 if there was no containing function.
    if (lfun < rfun)
    {
        for (lline = lfun + 1;
             lline < rfun && stabs[lline].n_type == N_PSYM;
             lline++)
        {
            info->eip_fn_n_arg++;
        }
    }
    return 0;
}

void print_kernel_info()
{
    extern char etext[], edata[], end[], kernel_init[];
    kernel_print("Special kernel symbols:\n");
    kernel_print("  entry  0x%08x (phys)\n", kernel_init);
    kernel_print("  etext  0x%08x (phys)\n", etext);
    kernel_print("  edata  0x%08x (phys)\n", edata);
    kernel_print("  end    0x%08x (phys)\n", end);
    kernel_print("Kernel executable memory footprint: %dKB\n", (end - kernel_init + 1023) / 1024);
}

void print_stack_frame()
{
    uint32 ebp = read_ebp(), eip = read_eip();

    int i, j;
    for (i = 0; ebp != 0 && i < STACK_FRAME_DEPTH; i++)
    {
        kernel_print("ebp:0x%08x eip:0x%08x args:", ebp, eip);
        uint32 *args = (uint32 *) ebp + 2;
        for (j = 0; j < 4; j++)
        {
            kernel_print("0x%08x ", args[j]);
        }
        kernel_print("\n");
        print_debug_info(eip - 1);
        eip = ((uint32 *) ebp)[1];
        ebp = ((uint32 *) ebp)[0];
    }
}

void print_debug_info(uintptr eip)
{
    struct EipDebugInfo info;
    if (debug_info_eip(eip, &info) != 0)
    {
        kernel_print("    <unknown>: -- 0x%08x --\n", eip);
    }
    else
    {
        char fn_name[256];
        int j;
        for (j = 0; j < info.eip_fn_namelen; j++)
        {
            fn_name[j] = info.eip_fn_name[j];
        }
        fn_name[j] = '\0';
        kernel_print("    %s:%d: %s+%d\n", info.eip_file, info.eip_line, fn_name, eip - info.eip_fn_addr);
    }
}
