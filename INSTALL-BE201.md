# BE201 (8086:7740) — installing and bringing up the Wi-Fi

Verified on macOS 15.8.1, both in a VM (aux-KC install) and on bare metal
(OpenCore ESP injection). The driver side is in this tree; the two things below
are **macOS** requirements and are the two steps that make the difference
between "the list shows" and "it connects".

---

## 1 · OpenCore (bare metal)

Copy `AirportItlwm.kext` to `EFI/OC/Kexts/` and add it to `Kernel → Add`:

```
BundlePath      AirportItlwm.kext
ExecutablePath  Contents/MacOS/AirportItlwm
PlistPath       Contents/Info.plist
Arch            Any
Enabled         true
Comment         AirportItlwm BE201
```

The kext must be **ad-hoc signed** (`codesign --force --sign - --timestamp=none`)
and must export its entry points. An unsigned product, or one built without the
C++-linkage `_start`/`_stop` stubs, fails to load with
`Failed to bind '__Z6_startP9kmod_infoPv'`.

---

## 2 · The System-keychain item — the step everyone misses

**Symptom without it:** the Wi-Fi menu lists networks, you pick one, macOS
prompts, and the join fails with a generic error ("could not be joined",
"could not find network"). `airportd` logs:

```
CWKeychainFindWiFiPassword() failed with error −25300   (errSecItemNotFound)
… then −25308                                           (errSecInteractionNotAllowed)
```

It reads like an RF fault. It is not. The preferred-network list knows the
network; **macOS has nowhere to read the secret from**, and a background daemon
cannot answer an "Allow" prompt.

**Fix — create the item with the ACL at creation time:**

```sh
sudo security add-generic-password \
     -a "<SSID>" \
     -s "AirPort" \
     -l "AirPort network password" \
     -D "AirPort network password" \
     -T /usr/libexec/airportd \
     -T /usr/bin/networksetup \
     -U -w "<PSK>" \
     /Library/Keychains/System.keychain
```

* **`-T /usr/libexec/airportd` is the load-bearing flag.** Without it the item
  exists and `airportd` still gets `−25308`.
* The **System** keychain, not login — an auto-join happens before anyone logs in.
* `networksetup -addpreferredwirelessnetworkatindex` writes the *list* entry but
  on a machine that has never had a Wi-Fi service does **not** reliably create a
  readable secret. That is the trap.

Verify, then trigger a join (or just reboot — `airportd` will auto-join):

```sh
sudo security find-generic-password -a "<SSID>" -s "AirPort" /Library/Keychains/System.keychain >/dev/null \
  && echo "present"
```

---

## 3 · What "working" looks like

```
scan       SCAN_OK count=27 · real channel and RSSI (ch157 −34 dBm, ch1 −33 dBm)
associate  SCAN → AUTH → ASSOC → RUN · st=4 · en1 RUNNING
dhcp       192.168.1.206 · router 192.168.1.1
traffic    nc -z <host> 22 → rc=0 with `route get` confirming the route is on en1
```

`ioreg` is the fastest proof the driver is real: `IOMACAddress` should be the
hardware address and `IOModel` should be `iwlwifi-bz-b0-fm-c0-c106.ucode`.

---

## 4 · Known limits

* **SSIDs are nil to bare command-line scanners.** macOS 14+ reveals SSIDs only to
  apps with Location Services authorisation, and a CLI binary has no bundle
  identity to grant it. Channel, RSSI and count are real. This is not a scan fault.
* **Scan completion is bounded, not instant.** This HAL posts `SCAN_DONE` at the
  end of every band sweep; a request that joins a sweep already past the AP's
  channel used to return zero results. The collection window (5 s) fixes that at
  the cost of a slightly slower scan.
* **The underlying driver still has a device-internal TX-completion freeze**,
  ~3–7/hour, independent of this layer. It recovers; persistence has not been
  proven beyond ~150 s continuous in the VM.
* **A USB Ethernet dongle on the same machine is unrelated.** An ASIX AX88179
  has no macOS driver and will not pass traffic; that is not this kext.
