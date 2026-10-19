#!/usr/bin/env bash
# guest-recover.sh - poll for the short SSH window during boot and remove the
# aux-KC kext, breaking a panic loop.
#
# WHY: an aux-KC kext that panics while macOS is starting leaves a VM that
# panics again on every boot. macOS also drops /Library/KernelCollections and
# /Library/StagedExtensions, so a healthy boot is impossible until the kext is
# gone from /Library/Extensions. The APFS volume cannot be edited from the Linux
# host, so the only way in is the brief window where sshd is up before the panic.
#
#   ./guest-recover.sh [timeout_seconds]
set -uo pipefail
HOST=tianyixia@192.168.122.39
KEY=~/.ssh/id_ed25519
DEADLINE=$(( $(date +%s) + ${1:-300} ))

echo "polling $HOST for a boot window..."
while [ "$(date +%s)" -lt "$DEADLINE" ]; do
    if timeout 4 bash -c "echo > /dev/tcp/192.168.122.39/22" 2>/dev/null; then
        echo "  port 22 open - attempting removal"
        if timeout 30 ssh -i "$KEY" -o StrictHostKeyChecking=no -o ConnectTimeout=6 \
                "$HOST" 'sudo rm -rf /Library/Extensions/itlwm.kext /Library/StagedExtensions && echo RECOVERED' 2>/dev/null | grep -q RECOVERED; then
            echo "  REMOVED /Library/Extensions/itlwm.kext - guest should now boot clean"
            exit 0
        fi
        echo "  window missed, continuing"
    fi
    sleep 4
done
echo "gave up after ${1:-300}s - the guest may need manual recovery" >&2
exit 1
