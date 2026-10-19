#!/usr/bin/env bash
# aux-install-itlwm.sh - install itlwm through the macOS AUXILIARY KERNEL COLLECTION.
#
# WHY THIS EXISTS
# ---------------
# On this VM, OpenCore's kext injection does NOT register IOKit personalities.
# The kext shows as loaded (`kmutil showloaded`) but nothing ever matches its
# IOKitPersonalities, so itlwm::probe() is never called and the driver never runs.
# Proven by a panic() at the top of probe(): it never fired via OpenCore, and
# fired immediately when the kext was loaded via the aux kernel collection.
#
# `kmutil rebuild` does NOT work for this: it consults a recorded list, not the
# filesystem, and reports "No changes detected" even with the kext staged in
# /Library/Extensions and /Library/StagedExtensions and an allowed=1 row in the
# kext-consent DB. The collection must be built explicitly.
#
# Run INSIDE the guest as root:  sudo ./aux-install-itlwm.sh [path/to/itlwm.kext]
#
# Requires a KDK matching the running build (for 15.8.1/24H32:
# /Library/Developer/KDKs/KDK_15.8.1_24H32.kdk).
set -euo pipefail

KEXT_SRC="${1:-$HOME/itlwm-be201/build/itlwm.kext}"
KDK="${KDK:-/Library/Developer/KDKs/KDK_15.8.1_24H32.kdk}"
AUX=/Library/KernelCollections/AuxiliaryKernelExtensions.kc
DEST=/Library/Extensions/itlwm.kext

say() { printf '  %s\n' "$*"; }
die() { printf 'ERROR: %s\n' "$*" >&2; exit 1; }

[ "$(id -u)" -eq 0 ] || die "run with sudo"
[ -d "$KEXT_SRC" ] || die "kext not found at $KEXT_SRC"
[ -d "$KDK" ] || die "KDK not found at $KDK (set KDK=... to override)"
[ -f /System/Library/KernelCollections/BootKernelExtensions.kc ] || die "no boot KC"

say "source      : $KEXT_SRC"
say "kdk         : $KDK"

# 1. install the kext where the collection builder will find it
rm -rf "$DEST"
cp -R "$KEXT_SRC" "$DEST"
chown -R root:wheel "$DEST"
say "installed   : $DEST"

# 2. build the aux collection explicitly. -B and -S are REQUIRED; without them
#    kmutil fails with "Cannot build pageableKC/auxKC without baseKC".
say "building aux kernel collection (takes a minute)..."
kmutil create -z \
  -n aux \
  -k /System/Library/Kernels/kernel \
  --kdk "$KDK" \
  -B /System/Library/KernelCollections/BootKernelExtensions.kc \
  -S /System/Library/KernelCollections/SystemKernelExtensions.kc \
  -A "$AUX" \
  -r /Library/Extensions \
  -a x86_64 --variant-suffix release >/tmp/aux-install.log 2>&1 \
  || { tail -20 /tmp/aux-install.log; die "kmutil create failed (see /tmp/aux-install.log)"; }

chown root:wheel "$AUX"; chmod 644 "$AUX"
size=$(stat -f%z "$AUX")
say "aux KC      : $size bytes"

# 3. verify it actually contains the kext, rather than assuming
if kmutil inspect -A "$AUX" 2>/dev/null | grep -q 'com.zxystd.itlwm'; then
    say "VERIFIED    : com.zxystd.itlwm is in the aux kernel collection"
else
    die "itlwm is NOT in the built collection - do not reboot expecting it to work"
fi

# 4. the OpenCore entry must be OFF, or the prelinked copy shadows this one
say ""
say "NEXT:"
say "  1. disable the OpenCore entry (on the host):"
say "       sudo ./oc-kext-toggle.sh disable itlwm.kext"
say "  2. reboot (clean shutdown preferred):  sudo shutdown -r now"
say "  3. verify:  kmutil showloaded | grep itlwm   # address should be 0xffffff7f..."
say ""
say "If the kext panics, recover OFFLINE over NBD (the guest will not boot) and"
say "delete $DEST, then rebuild the collection with the same command."
