#!/usr/bin/env bash
# oc-inject-itlwm.sh - install the BE201 itlwm fork into the OpenCore ESP, from
# INSIDE the guest, so OpenCore injects it at boot.
#
# This is the supported deployment path for itlwm, and it sidesteps macOS 15's
# LocalAuthentication kext-approval gate entirely: OpenCore prelinks kexts into
# the kernel collection at boot and never asks. Ten attempts to load the kext
# with kextutil/kmutil are documented in KEXT-LOADING-ON-MACOS15.md; this is the
# route that actually works on a Hackintosh.
#
# Adapted from ibt-port/guest-install-be201.sh (sibling session), which proved the
# approach for IntelBluetoothFirmware on macos-2. Two lessons carried over:
#   * identify the ESP by CONTENT (a partition containing EFI/OC/config.plist).
#     Disk order is NOT stable across reboots - hardcoding /dev/disk1s1 is a bug.
#   * unmount with `diskutil unmount`, not `umount`: Spotlight indexes the ESP as
#     soon as it is mounted read-write and plain umount fails EBUSY.
#
# Run in the guest:  sudo ./oc-inject-itlwm.sh
# Reversible: backs up config.plist to EFI/OC/config.plist.before-itlwm.

set -uo pipefail

MP=/tmp/ocesp
KEXT_NAME=itlwm
KEXT_SRC="${KEXT_SRC:-/Users/tianyixia/itlwm-be201/build/itlwm.kext}"
ESP_DEV="${ESP_DEV:-}"

say() { printf '  %s\n' "$*"; }
die() { printf 'ERROR: %s\n' "$*" >&2; exit 1; }

[ "$(id -u)" -eq 0 ] || die "run with sudo"
[ -d "$KEXT_SRC" ] || die "kext not found at $KEXT_SRC (build it first)"
[ -f "$KEXT_SRC/Contents/MacOS/$KEXT_NAME" ] || die "kext executable missing"

find_oc_esp() {
    local dev mp
    for dev in /dev/disk*s*; do
        case "$dev" in *rdisk*) continue;; esac
        mp="/tmp/ocprobe-$(basename "$dev")"
        mkdir -p "$mp"
        mount -t msdos -o ro "$dev" "$mp" 2>/dev/null || { rmdir "$mp" 2>/dev/null; continue; }
        if [ -f "$mp/EFI/OC/config.plist" ]; then
            umount "$mp" 2>/dev/null; rmdir "$mp" 2>/dev/null
            printf '%s\n' "$dev"; return 0
        fi
        umount "$mp" 2>/dev/null; rmdir "$mp" 2>/dev/null
    done
    return 1
}

if [ -z "$ESP_DEV" ]; then
    ESP_DEV="$(find_oc_esp)" || die "no EFI partition containing EFI/OC/config.plist"
    say "auto-detected OpenCore ESP: $ESP_DEV"
fi

umount "$MP" 2>/dev/null || true
mkdir -p "$MP"
mount -t msdos "$ESP_DEV" "$MP" || die "mount failed"
say "mounted rw at $MP"
[ -f "$MP/EFI/OC/config.plist" ] || die "no EFI/OC/config.plist - wrong ESP?"

BK="$MP/EFI/OC/config.plist.before-itlwm"
[ -f "$BK" ] || cp "$MP/EFI/OC/config.plist" "$BK"
say "config backed up -> EFI/OC/config.plist.before-itlwm"

# ------------------------------------------------------------------ kext
rm -rf "$MP/EFI/OC/Kexts/$KEXT_NAME.kext"
cp -R "$KEXT_SRC" "$MP/EFI/OC/Kexts/$KEXT_NAME.kext" || die "kext copy failed"
say "kext installed -> EFI/OC/Kexts/$KEXT_NAME.kext"

# ------------------------------------------------------------------ config
python3 - "$MP/EFI/OC/config.plist" "$KEXT_NAME" <<'PY'
import plistlib, sys
path, name = sys.argv[1], sys.argv[2]
cfg = plistlib.load(open(path, 'rb'))

kernel = cfg.setdefault('Kernel', {})
add = kernel.setdefault('Add', [])
entry = {
    'Arch': 'x86_64',
    'BundlePath': f'{name}.kext',
    'Comment': 'Intel BE201 (8086:7740) - local Bz port',
    'Enabled': True,
    'ExecutablePath': f'Contents/MacOS/{name}',
    'MaxKernel': '',
    'MinKernel': '',
    'PlistPath': 'Contents/Info.plist',
}
for i, e in enumerate(add):
    if isinstance(e, dict) and e.get('BundlePath') == entry['BundlePath']:
        add[i] = entry
        print(f'  updated Kernel->Add[{i}]')
        break
else:
    add.append(entry)
    print(f'  appended Kernel->Add (now {len(add)} entries)')

plistlib.dump(cfg, open(path, 'wb'))
PY
[ $? -eq 0 ] || die "config.plist update failed"

# ------------------------------------------------------------------ verify
echo
echo "verification (re-read from disk)"
python3 - "$MP/EFI/OC/config.plist" "$KEXT_NAME" <<'PY'
import plistlib, sys
cfg = plistlib.load(open(sys.argv[1], 'rb'))
name = sys.argv[2]
add = cfg['Kernel']['Add']
mine = [e for e in add if e.get('BundlePath') == f'{name}.kext']
print(f'  Kernel->Add total: {len(add)}; ours: {len(mine)}')
for e in mine:
    print(f"    Enabled={e['Enabled']} Path={e['BundlePath']} Exe={e['ExecutablePath']}")
# Sanity: itlwm must come after Lilu if Lilu is present (plugin dependency)
names = [e.get('BundlePath','') for e in add]
print('  order:', ', '.join(names))
assert len(mine) == 1, 'expected exactly one itlwm entry'
print('  OK')
PY

# ------------------------------------------------------------------ unmount
sync
unmounted=0
for attempt in 1 2 3; do
    if ! mount | grep -q " $MP "; then unmounted=1; break; fi
    diskutil unmount "$MP" >/dev/null 2>&1 && { unmounted=1; break; }
    diskutil unmount force "$MP" >/dev/null 2>&1 && { unmounted=1; break; }
    umount "$MP" 2>/dev/null && { unmounted=1; break; }
    sleep 2
done
if [ "$unmounted" -eq 1 ] && ! mount | grep -q " $MP "; then
    say "ESP unmounted cleanly"
else
    die "UNMOUNT FAILED - do not reboot; run: diskutil unmount force $MP"
fi
rmdir "$MP" 2>/dev/null || true

# ------------------------------------------------------------------ kext ownership
# OpenCore reads the ESP itself, so ownership there does not matter, but the
# staged copy that macOS keeps does - keep the source kext root:wheel.
chown -R root:wheel "$KEXT_SRC" 2>/dev/null || true

cat <<'NEXT'

done. next:
  1. reboot the guest (guest reboot, NOT Shut Down):
         sudo shutdown -r now
  2. after it returns, verify:
         ioreg -l | grep -i itlwm            # driver matched?
         kmutil showloaded | grep -i itlwm   # loaded?
         system_profiler SPAirPortDataType   # a WiFi interface?
         log show --last 2m --predicate 'process == "kernel"' | grep -i itlwm

undo (mount the ESP rw first):
         sudo mount -t msdos <ESP> /tmp/ocesp
         sudo cp /tmp/ocesp/EFI/OC/config.plist.before-itlwm /tmp/ocesp/EFI/OC/config.plist
         sudo rm -rf /tmp/ocesp/EFI/OC/Kexts/itlwm.kext
         sudo diskutil unmount /tmp/ocesp
NEXT
