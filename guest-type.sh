#!/usr/bin/env bash
# guest-type.sh - type a string into a libvirt guest's console via virsh send-key.
#
# WHY: single-user mode (-s) has no sshd and no vnc/spice listener reachable from
# here, so the only way to run recovery commands is to drive the guest's keyboard.
# Used to break a panic loop where an aux-KC kext panics before sshd starts.
#
#   ./guest-type.sh "rm -rf /Library/Extensions/itlwm.kext"
#
# NOTE: this is slow (one virsh call per character) but reliable.
set -uo pipefail
DOMAIN="${DOMAIN:-macos-1}"
TEXT="$1"

# chars that need shift, and the keycode names virsh expects
shift_map() {
    case "$1" in
        '!') echo 'shift-1';; '@') echo 'shift-2';; '#' ) echo 'shift-3';;
        '$') echo 'shift-4';; '%') echo 'shift-5';; '^') echo 'shift-6';;
        '&') echo 'shift-7';; '*') echo 'shift-8';; '(') echo 'shift-9';;
        ')') echo 'shift-0';; '_') echo 'shift-minus';; '+') echo 'shift-equal';;
        '{') echo 'shift-bracket_left';; '}') echo 'shift-bracket_right';;
        '|') echo 'shift-backslash';; ':') echo 'shift-semicolon';;
        '"') echo 'shift-apostrophe';; '<') echo 'shift-comma';;
        '>') echo 'shift-dot';; '?') echo 'shift-slash';; '~') echo 'shift-grave_accent';;
        *) echo '';;
    esac
}
plain_map() {
    case "$1" in
        '-') echo 'minus';; '=') echo 'equal';; '[') echo 'bracket_left';;
        ']') echo 'bracket_right';; '\\') echo 'backslash';; ';') echo 'semicolon';;
        "'") echo 'apostrophe';; ',') echo 'comma';; '.') echo 'dot';;
        '/') echo 'slash';; '`') echo 'grave_accent';; ' ') echo 'spc';;
        *) echo "$1";;
    esac
}

while IFS= read -r -n1 ch; do
    [ -z "$ch" ] && continue
    k=$(shift_map "$ch")
    if [ -n "$k" ]; then
        virsh -c qemu:///system send-key "$DOMAIN" --codeset linux $k >/dev/null 2>&1
    else
        virsh -c qemu:///system send-key "$DOMAIN" --codeset linux "$(plain_map "$ch")" >/dev/null 2>&1
    fi
    sleep 0.03
done <<< "$TEXT"

virsh -c qemu:///system send-key "$DOMAIN" --codeset linux ret >/dev/null 2>&1
sleep 0.2
