#!/usr/bin/env bash
#
# build-kext-clt.sh — build the `itlwm` kext (NOT AirportItlwm) on a macOS box that
# has only the Command Line Tools (no full Xcode).
#
# xcodebuild refuses to run against a CLT-only install, but the CLT SDK ships
# Kernel.framework headers and clang can emit and link kext bundles directly.
#
# Run from the repo root (~/itlwm-be201).
#
# The source list below is taken verbatim from the `itlwm` PBXNativeTarget's
# PBXSourcesBuildPhase in itlwm.xcodeproj/project.pbxproj (71 files: 28 .cpp + 43 .c).
# hal_iwm and hal_iwn are NOT excluded — ItlIwx.cpp / itlwm.cpp reference
# ItlIwm / ItlIwn and the HAL service, so all three HALs are needed.
#
# Compile failures do NOT abort the run: every source is attempted and a summary
# of failures is printed, so that a single full pass enumerates all root causes.
#
# Output: build/itlwm.kext
#
#   ./build-kext-clt.sh            # generate fw table if needed, compile, link, bundle
#   ./build-kext-clt.sh -k         # keep going (default); -s = stop at first failure
#
set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

STOP_ON_FIRST=0
[ "${1:-}" = "-s" ] && STOP_ON_FIRST=1

APP="itlwm"
KEXT="build/${APP}.kext"
STAGE="build/${APP}.stage"
LOGDIR="build/logs"
BUNDLE_ID="com.zxystd.itlwm"
MODULE_VERSION="2.4.0"
DEPLOY="10.9"
SDK="$(xcrun --show-sdk-path)"

say()  { printf '  %s\n' "$*"; }
head_() { printf '\n=== %s ===\n' "$*"; }
die()  { printf 'ERROR: %s\n' "$*" >&2; exit 1; }

# ----------------------------------------------------------------- preflight
head_ "preflight"
[ -f "${APP}/itlwm.cpp" ] || die "run this from the repo root (itlwm/itlwm.cpp not found)."
command -v python3 >/dev/null || die "python3 required (generates include/FwBinary.cpp)."
[ -d "$SDK" ] || die "no SDK from xcrun --show-sdk-path"
KHEADERS="${SDK}/System/Library/Frameworks/Kernel.framework/Headers"
[ -d "$KHEADERS" ] || die "Kernel.framework headers not found at $KHEADERS"
say "SDK:        $SDK"
say "kernel hdr: $KHEADERS"
xcrun clang --version | head -1 | sed 's/^/  /'

# ----------------------------------------------------------------- MacKernelSDK
head_ "MacKernelSDK"
if [ ! -d MacKernelSDK/Headers ]; then
    say "cloning MacKernelSDK..."
    git clone -q --depth=1 https://github.com/acidanthera/MacKernelSDK.git \
        || die "MacKernelSDK clone failed"
fi
say "MacKernelSDK/Headers present"
[ -f MacKernelSDK/Library/x86_64/libkmod.a ] && say "libkmod.a present (MacKernelSDK/Library/x86_64)"

# ----------------------------------------------------------------- firmware table
# Mirrors `scripts/fw_gen.sh -P "${PROJECT_DIR}/itlwm/firmware/"`, which is the
# "Generate Firmware" shell-script phase of the fw_gen aggregate target that the
# itlwm target depends on.
head_ "firmware table"
if [ -s include/FwBinary.cpp ]; then
    say "include/FwBinary.cpp already present ($(wc -c < include/FwBinary.cpp) bytes), keeping"
else
    say "generating include/FwBinary.cpp from itlwm/firmware/ ..."
    python3 scripts/zlib_compress_fw.py include/FwBinary.cpp itlwm/firmware/ \
        || die "FwBinary.cpp generation failed"
fi
[ -s include/FwBinary.cpp ] || die "include/FwBinary.cpp missing/empty"
say "include/FwBinary.cpp: $(wc -c < include/FwBinary.cpp) bytes, fwNumber=$(grep -o 'fwNumber = [0-9]*' include/FwBinary.cpp | tail -1)"

# ----------------------------------------------------------------- sources
# Verbatim from PBXSourcesBuildPhase 024A07A823FCBC3C009FBA6C (target "itlwm").
CPP_SOURCES=(
    include/FwBinary.cpp                 # generated above
    include/HAL/ItlHalService.cpp
    itlwm/itlwm.cpp
    itlwm/pm.cpp
    itlwm/ItlNetworkUserClient.cpp
    itlwm/hal_iwx/ItlIwx.cpp
    itlwm/hal_iwm/ItlIwm.cpp
    itlwm/hal_iwm/coex.cpp
    itlwm/hal_iwm/ctxt.cpp
    itlwm/hal_iwm/fw.cpp
    itlwm/hal_iwm/hw.cpp
    itlwm/hal_iwm/io.cpp
    itlwm/hal_iwm/led.cpp
    itlwm/hal_iwm/mac80211.cpp
    itlwm/hal_iwm/nvm.cpp
    itlwm/hal_iwm/phy.cpp
    itlwm/hal_iwm/power.cpp
    itlwm/hal_iwm/rs.cpp
    itlwm/hal_iwm/rx.cpp
    itlwm/hal_iwm/scan.cpp
    itlwm/hal_iwm/tx.cpp
    itlwm/hal_iwm/utils.cpp
    itlwm/hal_iwn/ItlIwn.cpp
    itl80211/compat.cpp
    itl80211/openbsd/net80211/CTimeout.cpp
    itl80211/openbsd/sys/_ifq.cpp
    itl80211/openbsd/sys/_mbuf.cpp
    itl80211/openbsd/sys/_task.cpp
)
C_SOURCES=(
    # Supplies the _kmod_info symbol that kmutil requires before it will place a
    # kext into a kernel collection. Xcode synthesises this; the direct-clang
    # path does not, and without it the load fails with
    # "kexts must have a _kmod_info symbol".
    itlwm/kmodinfo.c
    itl80211/zutil.c
    itl80211/openbsd/sys/arp.c
    itl80211/openbsd/net80211/_string.c
    itl80211/openbsd/net80211/timeout.c
    itl80211/openbsd/net80211/ieee80211.c
    itl80211/openbsd/net80211/ieee80211_amrr.c
    itl80211/openbsd/net80211/ieee80211_crypto.c
    itl80211/openbsd/net80211/ieee80211_crypto_bip.c
    itl80211/openbsd/net80211/ieee80211_crypto_ccmp.c
    itl80211/openbsd/net80211/ieee80211_crypto_tkip.c
    itl80211/openbsd/net80211/ieee80211_crypto_wep.c
    itl80211/openbsd/net80211/ieee80211_input.c
    itl80211/openbsd/net80211/ieee80211_ioctl.c
    itl80211/openbsd/net80211/ieee80211_mira.c
    itl80211/openbsd/net80211/ieee80211_node.c
    itl80211/openbsd/net80211/ieee80211_output.c
    itl80211/openbsd/net80211/ieee80211_pae_input.c
    itl80211/openbsd/net80211/ieee80211_pae_output.c
    itl80211/openbsd/net80211/ieee80211_proto.c
    itl80211/openbsd/net80211/ieee80211_ra.c
    itl80211/openbsd/net80211/ieee80211_regdomain.c
    itl80211/openbsd/net80211/ieee80211_rssadapt.c
    itl80211/openbsd/crypto/aes.c
    itl80211/openbsd/crypto/arc4.c
    itl80211/openbsd/crypto/blf.c
    itl80211/openbsd/crypto/cast.c
    itl80211/openbsd/crypto/chachapoly.c
    itl80211/openbsd/crypto/cmac.c
    itl80211/openbsd/crypto/ecb3_enc.c
    itl80211/openbsd/crypto/ecb_enc.c
    itl80211/openbsd/crypto/gmac.c
    itl80211/openbsd/crypto/hmac.c
    itl80211/openbsd/crypto/idgen.c
    itl80211/openbsd/crypto/key_wrap.c
    itl80211/openbsd/crypto/md5.c
    itl80211/openbsd/crypto/michael.c
    itl80211/openbsd/crypto/poly1305.c
    itl80211/openbsd/crypto/rijndael.c
    itl80211/openbsd/crypto/rmd160.c
    itl80211/openbsd/crypto/set_key.c
    itl80211/openbsd/crypto/sha1.c
    itl80211/openbsd/crypto/sha1-pbkdf2.c
    itl80211/openbsd/crypto/sha2.c
)

for f in "${CPP_SOURCES[@]}" "${C_SOURCES[@]}"; do
    [ -f "$f" ] || die "missing source: $f"
done
say "${#CPP_SOURCES[@]} C++ sources, ${#C_SOURCES[@]} C sources ($(( ${#CPP_SOURCES[@]} + ${#C_SOURCES[@]} )) total)"

# ----------------------------------------------------------------- flags
# Mirrors the itlwm target's build settings:
#   SYSTEM_HEADER_SEARCH_PATHS = "itl80211/openbsd itl80211 include"
#   GCC_PREFIX_HEADER          = itlwm/PrivateSPI.pch   (+ GCC_PREPROCESSOR_DEFINITIONS __PRIVATE_SPI__)
#   KERNEL_EXTENSION_HEADER_SEARCH_PATHS = MacKernelSDK/Headers
#   MACOSX_DEPLOYMENT_TARGET   = 10.9
#
# Project-level settings (XCBuildConfiguration 024A07B5 = project "itlwm" Release):
#   GCC_INPUT_FILETYPE = sourcecode.cpp.cpp   <-- *EVERY* source is compiled as C++,
#                                                 including the .c files. This is not
#                                                 cosmetic: itl80211/openbsd/sys/timeout.h
#                                                 (pulled in through ieee80211_var.h)
#                                                 #includes IOKit C++ headers, and
#                                                 itl80211/zutil.{c,h} contain a bare
#                                                 `extern "C" {`.  Compiling them as C
#                                                 fails with "unknown type name 'class'".
#   GCC_PREPROCESSOR_DEFINITIONS = ITLWM_VERSION="2.4.0" GIT_COMMIT="" IEEE80211_STA_ONLY
#   MODULE_VERSION = 2.4.0 ; MACOSX_DEPLOYMENT_TARGET = 10.12 (target overrides to 10.9)
#   OTHER_CPLUSPLUSFLAGS = -Wno-deprecated-register
INCLUDES=(
    -IMacKernelSDK/Headers
    -Iinclude
    -Iinclude/HAL
    -Iitlwm
    -Iitlwm/hal_iwx
    -Iitlwm/hal_iwm
    -Iitlwm/hal_iwn
    -Iitl80211
    -Iitl80211/openbsd
    -Iitl80211/openbsd/sys
    -Iitl80211/openbsd/net80211
    -Iitl80211/openbsd/crypto
    -Iitl80211/linux
)
COMMON=(
    -mkernel -fno-builtin -fno-common -fno-exceptions -fno-rtti
    -DKERNEL=1
    -D__PRIVATE_SPI__
    -DIEEE80211_STA_ONLY
    -DITLWM_VERSION="\"${MODULE_VERSION}\""
    -DGIT_COMMIT="\"\""
    -DKMOD_NAME="\"${BUNDLE_ID}\""
    -DKMOD_VERSION="\"${MODULE_VERSION}\""
    -Wno-deprecated-register
    -mmacosx-version-min="$DEPLOY" -isysroot "$SDK"
    -Wno-everything
    -include itlwm/PrivateSPI.pch
)

# ----------------------------------------------------------------- compile
head_ "compile"
rm -rf "$STAGE" "$LOGDIR"; mkdir -p "$STAGE" "$LOGDIR"
OBJS=(); FAILED=(); OK=()

# Everything goes through clang++ with -x c++ : GCC_INPUT_FILETYPE = sourcecode.cpp.cpp.
compile_one() {
    local src="$1" obj="$2"
    local log="${LOGDIR}/$(echo "$src" | tr '/' '_').log"
    if xcrun clang++ "${COMMON[@]}" "${INCLUDES[@]}" -std=gnu++14 -x c++ -c "$src" -o "$obj" >"$log" 2>&1; then
        printf '  OK   %s\n' "$src"
        OK+=("$src"); OBJS+=("$obj")
    else
        printf '  FAIL %s  (%s)\n' "$src" "$log"
        FAILED+=("$src")
        if [ "$STOP_ON_FIRST" = 1 ]; then
            printf '\n--- %s ---\n' "$src"; sed -n '1,40p' "$log"
        fi
    fi
}

for src in "${CPP_SOURCES[@]}"; do
    compile_one "$src" "${STAGE}/$(echo "${src%.*}" | tr '/' '_').o"
done
for src in "${C_SOURCES[@]}"; do
    compile_one "$src" "${STAGE}/$(echo "${src%.*}" | tr '/' '_').o"
done

printf '\n  compiled OK: %d   failed: %d\n' "${#OK[@]}" "${#FAILED[@]}"

if [ "${#FAILED[@]}" -gt 0 ]; then
    head_ "FAILURES (first distinct root cause per file)"
    for src in "${FAILED[@]}"; do
        log="${LOGDIR}/$(echo "$src" | tr '/' '_').log"
        printf '\n--- %s ---\n' "$src"
        grep -E '^[^ ].*: (error|fatal error):' "$log" | head -6
        grep -m1 -E 'error:|fatal error:' "$log" | sed 's/^/    first: /'
    done
fi

if [ "${#OBJS[@]}" -eq 0 ]; then
    head_ "VERDICT"; echo "  no objects produced; nothing to link."; exit 1
fi

# ----------------------------------------------------------------- bundle + link
head_ "link"
rm -rf "$KEXT"
mkdir -p "${KEXT}/Contents/MacOS"
EXE="${KEXT}/Contents/MacOS/${APP}"

LINKLOG="${LOGDIR}/link.log"
xcrun clang++ -mkernel -nostdlib -r -Xlinker -kext \
    -mmacosx-version-min="$DEPLOY" -isysroot "$SDK" \
    "${OBJS[@]}" -o "$EXE" >"$LINKLOG" 2>&1
LINKRC=$?
if [ $LINKRC -ne 0 ]; then
    say "link failed (rc=$LINKRC); first 40 lines:"
    sed -n '1,40p' "$LINKLOG" | sed 's/^/    /'
    # retry with libkmod.a (Xcode links it via LIBRARY_SEARCH_PATHS)
    if [ -f MacKernelSDK/Library/x86_64/libkmod.a ]; then
        say "retrying with MacKernelSDK/Library/x86_64/libkmod.a ..."
        xcrun clang++ -mkernel -nostdlib -r -Xlinker -kext \
            -mmacosx-version-min="$DEPLOY" -isysroot "$SDK" \
            "${OBJS[@]}" MacKernelSDK/Library/x86_64/libkmod.a -o "$EXE" \
            >>"$LINKLOG" 2>&1 && LINKRC=0
        [ $LINKRC -ne 0 ] && tail -30 "$LINKLOG" | sed 's/^/    /'
    fi
fi

if [ $LINKRC -ne 0 ] || [ ! -f "$EXE" ]; then
    head_ "VERDICT"
    echo "  compiled ${#OK[@]}/${#OBJS[@]} objects but LINK FAILED; no kext produced."
    echo "  see $LINKLOG"
    exit 1
fi
say "linked: $EXE"

# ----------------------------------------------------------------- Info.plist
head_ "bundle"
xcrun strip -x "$EXE" 2>/dev/null || true

python3 - "$APP/Info.plist" "${KEXT}/Contents/Info.plist" "$BUNDLE_ID" "$MODULE_VERSION" <<'PY'
import plistlib, sys
src = plistlib.load(open(sys.argv[1], 'rb'))
out = {
    'CFBundleDevelopmentRegion': 'en',
    'CFBundleExecutable': 'itlwm',
    'CFBundleIdentifier': sys.argv[3],
    'CFBundleInfoDictionaryVersion': '6.0',
    'CFBundleName': 'itlwm',
    'CFBundlePackageType': 'KEXT',
    'CFBundleShortVersionString': sys.argv[4],
    'CFBundleVersion': sys.argv[4],
    'OSBundleRequired': 'Root',
    'IOKitPersonalities': src['IOKitPersonalities'],
}
for k in ('OSBundleLibraries', 'NSHumanReadableCopyright'):
    if k in src:
        out[k] = src[k]
plistlib.dump(out, open(sys.argv[2], 'wb'))
print("    personalities:", list(src['IOKitPersonalities'].keys()))
print("    OSBundleLibraries:", list(out.get('OSBundleLibraries', {}).keys()))
PY

codesign --force --sign - --timestamp=none "$KEXT" >/dev/null 2>&1 \
    && say "ad-hoc signed" || say "codesign skipped/failed (non-fatal)"

head_ "RESULT"
say "kext: $KEXT"
ls -l "$EXE" | awk '{printf "  executable: %s bytes\n", $5}'
file "$EXE" | sed 's/^/  /'
echo
