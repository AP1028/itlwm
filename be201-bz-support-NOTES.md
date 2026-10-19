# Intel BE201 (Bz) support — what is in this branch, and what is not

**Read this first.** This branch adds device recognition and firmware plumbing for the Intel
BE201. It does **not** make the card work, and the driver has **never executed**. Everything below
is written to be checkable rather than reassuring.

## The card

| property | value |
|---|---|
| PCI | `8086:7740`, subsystem `8086:00e4`, class `028000` |
| Bus | CNVi (integrated) |
| MAC type | Bz, `0x46` (`IWL_CFG_MAC_TYPE_BZ`) |
| RF type | FM, `0x112` (`IWL_CFG_RF_TYPE_FM`), `rfid = 0x20112200` |
| Firmware | `iwlwifi-bz-b0-fm-c0-c106.ucode` (+ `.pnvm`) |
| Linux op-mode | `iwlmld` — the **MLD** path |

## What this branch changes

| file | change |
|---|---|
| `itlwm/hal_iwx/if_iwxvar.h` | `IWX_DEVICE_FAMILY_BZ` (only `22000` and `AX210` existed), with an audit of the 25 `>= AX210` sites it now inherits |
| `itlwm/hal_iwx/ItlIwx.cpp` | `iwl_bz_trans_cfg`; `iwl_cfg_bz_b0_fm_c0`; PCI id `0x7740` in **both** `iwx_devices[]` and `iwl_dev_info_table`; `iwl_bz_name[]` |
| `itlwm/firmware/` | the two Bz blobs |
| `oc-inject-itlwm.sh` | installs the kext into the OpenCore ESP (new tooling; see below) |
| `PR-DESCRIPTION.md`, `KEXT-LOADING-ON-MACOS15.md` | the write-ups |

`IWL_CFG_MAC_TYPE_BZ 0x46` and `IWL_CFG_RF_TYPE_FM 0x112` were already defined upstream but **never
referenced**; this branch is the first use.

## What is verified

- **Compiles and links.** 71 sources, `EXIT=0`, on a Command Line Tools-only macOS 15.8.1, via the
  `build-kext-clt.sh` added here. Re-verified after every edit.
- **Two real build-path defects found and fixed.** The direct-clang path does not synthesise
  `_kmod_info` (Xcode does), and `_start`/`_stop` were C++-mangled and, as C symbols, would have
  become `__start`/`__stop`. Either would break anyone building without full Xcode. Both fixed.
- **Loads.** With the kext in `EFI/OC/Kexts/` and a `Kernel > Add` entry, `kmutil showloaded` lists
  `com.zxystd.itlwm (2.4.0)`. Removing the entry stops it loading, which proves the injection path.
- **Firmware plumbing correct.** Both blobs are compressed into `include/FwBinary.cpp` by the
  existing `scripts/fw_gen.sh`; name resolution verified; TLV magic matches a supported image; the
  PNVM name derives correctly.
- **Device matching correct.** `CSR_HW_RFID_TYPE(0x20112200)` = `0x112` = `IWL_CFG_RF_TYPE_FM`, and
  the `_IWL_DEV_INFO` argument order was verified against the macro.
- **`iwx_match(0x7740)` returns 1, and `probe()` returns `this`.** Observed once, on one boot, via a
  temporary `panic()` diagnostic — see the caveat below.
- **The hardware exists and macOS sees it.** `system_profiler` reports `0x7740` / `0x00e4`.

## What is NOT verified — read this part

- **The driver has never attached.** `"itlwm"=0`, `"ItlIwx"=0`, `"ItlHalService"=0`.
- **No Bz code has ever executed.** Not one line. The failure precedes the HAL.
- **The `probe()` success is a single observation.** It was seen once (panic report, 08:10) and
  **never reproduced** across roughly eight subsequent attempts, including a clean rebuild, a
  verified byte-identical kext, a corrected config, and a clean shutdown + cold boot. Treat "probe
  works, attach fails" as *unconfirmed*.
- **MLO is absent.** Bz is an MLD/MLO part and the iwx HAL has **no** `link_id` / `deflink`
  concept. Linux drives this exact silicon with `op_mode iwlmld`. This is not a corner case — it is
  the only mode Linux uses for this device.
- **Bz transport geometry is inherited, not implemented.** `num_of_queues`, `max_tfd_queue_size`,
  `apmg_not_supported`, `shadow_reg_enable`, `pcie_l1_allowed` mostly cannot be expressed by
  itlwm's `iwl_cfg`, so the Bz config inherits AX210-path behaviour. Two values are at least
  consistent with Linux: `IWX_TFD_QUEUE_SIZE_MAX_GEN3` = 65536 = Linux `max_tfd_queue_size`, and
  `txq[]` is already sized 512 = Linux `num_of_queues`.
- **`mac_step` is `IWL_CFG_ANY`.** Linux reports PCI `rev=0x461`, whose low nibble suggests step B0
  (matching the `bz-b0` firmware name) — but the guest reports `rev=00`, so the step is not
  asserted anywhere.

## Why it does not attach — current understanding

The blocker is **not** in the Bz code, because no Bz code runs. Established by elimination:

- the device is present and its IOKit node is `registered, matched, active`;
- the personality is well-formed (`IOClass=itlwm`, `IOProviderClass=IOPCIDevice`,
  `IOPCIPrimaryMatch=0x00008086&0x0000ffff`, `IODefaultMatchCategory`);
- the device exposes **both** MSI (`0xd0`) and MSI-X (`0x80`), so `probe()` cannot be bailing at its
  `"No MSI cap"` check;
- `Kernel > Block` has no enabled entries; SIP has kext signing disabled; the kext-consent policy
  DB already records `com.zxystd.itlwm` with `allowed = 1`;
- a purpose-built minimal kext with the **same** matching configuration *does* attach to the card,
  which exonerates the environment, the toolchain and the injection path.

The contradiction is that `probe()` was seen running once and has not been seen since.

## Nothing here should be presented as working support

Suggested honest framing for a PR or issue: *"Adds Bz device recognition and firmware plumbing for
the BE201. The kext compiles, links and loads; the driver does not yet attach, and MLO — which this
part requires, and which Linux uses exclusively for it — is not implemented. No Bz code has
executed on hardware."*

## Licence note

The two firmware blobs are Intel's, redistributed under the same terms as the other blobs already
in `itlwm/firmware/`. Note the repo's `iwlwifi-firmware-license` is a file at the repo root
referenced only from an Xcode navigator group; adding these blobs adds ~2.3 MB to every target
because `FwBinary.cpp` is one shared array for itlwm plus the eight AirportItlwm targets. A
maintainer may prefer to gate them behind a build flag.
