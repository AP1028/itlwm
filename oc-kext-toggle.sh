#!/usr/bin/env bash
# oc-kext-toggle.sh - enable/disable an OpenCore Kernel>Add entry, offline over NBD.
#
#   sudo ./oc-kext-toggle.sh disable itlwm.kext
#   sudo ./oc-kext-toggle.sh enable  itlwm.kext
#   sudo ./oc-kext-toggle.sh list
#
# Works even when the guest will not boot, which is the whole point: a kext that
# hangs macOS leaves no other way in. The ESP must not be held by a running
# guest, so stop the domain first (virsh destroy/shutdown).
set -uo pipefail
IMG=/home/tianyixia/OSX-KVM/OpenCore/OpenCore-1.qcow2
NBD=/dev/nbd0
MP=/mnt/oc
ACT="${1:-list}"; TARGET="${2:-}"

cleanup() { umount "$MP" 2>/dev/null || true; qemu-nbd --disconnect "$NBD" >/dev/null 2>&1 || true; }
trap cleanup EXIT

modprobe nbd max_part=8 2>/dev/null || true
qemu-nbd --disconnect "$NBD" >/dev/null 2>&1 || true
sleep 1
qemu-nbd --connect="$NBD" "$IMG" || { echo "qemu-nbd connect failed (ESP in use by a running guest?)" >&2; exit 1; }
sleep 3
partx -a "$NBD" 2>/dev/null || true
mkdir -p "$MP"
mount -t vfat "${NBD}p1" "$MP" || { echo "mount failed" >&2; exit 1; }

python3 - "$MP/EFI/OC/config.plist" "$ACT" "$TARGET" <<'PY'
import plistlib, sys
path, act, target = sys.argv[1], sys.argv[2], sys.argv[3]
c = plistlib.load(open(path, 'rb'))
add = c['Kernel']['Add']
if act == 'list':
    for e in add:
        print("  %-6s %s" % (e.get('Enabled'), e.get('BundlePath')))
else:
    if not target:
        sys.exit("need a BundlePath")
    n = 0
    for e in add:
        if e.get('BundlePath') == target:
            e['Enabled'] = (act == 'enable'); n += 1
    if not n:
        sys.exit("no entry with BundlePath=%r" % target)
    plistlib.dump(c, open(path, 'wb'))
    c2 = plistlib.load(open(path, 'rb'))
    for e in c2['Kernel']['Add']:
        print("  %-6s %s" % (e.get('Enabled'), e.get('BundlePath')))
PY
rc=$?
sync
exit $rc
