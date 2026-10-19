/*
 * kmodinfo.c - supply the `_kmod_info` symbol the kernel-collection builder requires.
 *
 * WHY THIS FILE EXISTS
 * --------------------
 * build-kext-clt.sh builds this kext by invoking clang directly, because
 * `xcodebuild` refuses to run against a Command Line Tools-only install. The
 * result is a valid "Mach-O 64-bit kext bundle x86_64" which OpenCore will
 * happily prelink, but which `kmutil`/`kextutil` reject:
 *
 *     cannot be placed in kernel collection because:
 *         kexts must have a _kmod_info symbol
 *
 * The Xcode toolchain synthesises that symbol from the target's settings; the
 * direct-clang path does not. Measured before this file existed:
 *
 *     $ nm build/itlwm.kext/Contents/MacOS/itlwm | grep _kmod_info
 *     (nothing)
 *
 * THE DETAIL THAT MATTERS
 * -----------------------
 * The symbol must be an ordinary external data symbol landing in __DATA,__data.
 * Giving it a custom section produces a *section* symbol ('S'), which kmutil
 * does not accept. So: declare a plain global, no section attribute, and let the
 * default placement happen.
 *
 * Compile this file as C OR C++ - it is written to be valid as both, because
 * itlwm's project sets GCC_INPUT_FILETYPE so that every source is compiled as
 * C++. The extern "C" guards below are what keep `_kmod_info`, `_start` and
 * `_stop` unmangled either way; C++ mangling would defeat the whole purpose.
 *
 * Fields the kernel fills in at load time (id, address, size, hdr_size,
 * reference_count, reference_list) are left zero. start/stop point at the
 * C-linkage _start/_stop defined in itl80211/openbsd/net80211/ieee80211.c.
 *
 * NOTE: this file is only needed for the CLT build path. It is harmless in the
 * Xcode build, which simply gets the symbol from here instead of synthesising it.
 */

#include <mach/kmod.h>

#define KMOD_NAME_VALUE    "com.zxystd.itlwm"
#define KMOD_VERSION_VALUE "2.4.0"

/* The declarations MUST be inside the extern "C" block below. Declared outside
 * it, this file is compiled as C++ (the project forces that on every source) and
 * they bind to the mangled names instead:
 *     U __Z16itlwm_kmod_startP9kmod_infoPv
 * which is exactly the failure the first attempt produced. Plain C names are
 * used because in C a source name of `_start` would become `__start` in the
 * Mach-O symbol table -- the toolchain adds its own leading underscore -- so the
 * obvious name is the wrong one. */


#ifdef __cplusplus
extern "C" {
#endif

extern int itlwm_kmod_start(struct kmod_info *, void *);
extern int itlwm_kmod_stop(struct kmod_info *, void *);

/* No section() attribute on purpose: a plain global lands in __DATA,__data. */
kmod_info_t kmod_info
    __attribute__((used, visibility("default"))) = {
        .next            = 0,
        .info_version    = KMOD_INFO_VERSION,
        .id              = 0,
        .name            = KMOD_NAME_VALUE,
        .version         = KMOD_VERSION_VALUE,
        .reference_count = 0,
        .reference_list  = 0,
        .address         = 0,
        .size            = 0,
        .hdr_size        = 0,
        .start           = &itlwm_kmod_start,
        .stop            = &itlwm_kmod_stop,
};

#ifdef __cplusplus
}
#endif
