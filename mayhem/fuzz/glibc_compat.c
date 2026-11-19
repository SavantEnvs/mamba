// conda-forge's static krb5 was built against an old glibc (2.17) and references __res_nsearch,
// which newer glibc only exports as a versioned compat symbol (not linkable). Forward it to the
// public res_nsearch. This only adapts the libc environment; no mamba code is replaced.
#include <netinet/in.h>
#include <resolv.h>

int __res_nsearch(res_state statp, const char* dname, int klass, int type, unsigned char* answer, int anslen)
{
    return res_nsearch(statp, dname, klass, type, answer, anslen);
}
