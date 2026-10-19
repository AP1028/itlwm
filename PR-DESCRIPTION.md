# Add initial Intel BE200/BE201 (Bz) device recognition and firmware plumbing

Branch: `be201-bz-support`
Base: `OpenIntelWireless/itlwm` @ `53c51c2` ("bump version to 2.4.0")
Subject device: Intel BE201, PCI `8086:7740`, CNVi, firmware family `bz-b0-fm-c0`

---

## Status: this does NOT make a BE201 work. Read this first.

This change makes the driver **recognise** a BE201 and gives it a firmware image and
firmware name. It does **not** bring the card up, and it cannot be claimed to: the
transport work Bz needs is not present, and no part of this has been compiled or run.

Concretely, what is and is not done:

| item | state |
|---|---|
| PCI id `0x7740` in `iwx_devices[]` | done |
| `iwx_devices[]` **and** `iwl_dev_info_table` entry (both are required) | done |
| Bz MAC type (`0x46`) and FM RF type (`0x112`) actually matched | done |
| `IWX_DEVICE_FAMILY_BZ` family constant | done |
| Bz `iwl_cfg_trans_params` (integrated, xtal_latency, ltr_delay, low_latency_xtal) | done, partial — see below |
| Firmware blobs bundled + name resolution verified | done |
| Firmware TLV header format compatible with the existing parser | **verified**, see below |
| Bz transport / queue / TFD configuration | **NOT DONE** — and see the inheritance warning below |
| Context-info (gen2) handling for Bz | **NOT DONE** (may reuse AX210 path; unverified) |
| MLO / multi-link | **NOT DONE** and not started |
| Compiled | **YES** — all 71 sources + link, on a CLT-only macOS 15.8.1 box. See "Build status" |
| Tested on hardware | **NO** |
| Anything proven to associate | **NO** |

Anyone reading this as "BE201 support" would be misled. It is device recognition plus the
firmware plumbing that any real port would need as step one.

---

## Why the change is shaped this way

### Both device tables must be touched, not just `iwx_devices[]`

`iwx_devices[]` only decides *whether* to attach. The actual per-device configuration is
selected by `iwx_attach()` walking `iwl_dev_info_table` and matching on
`device / subdevice / mac_type / mac_step / rf_type / cdb / rf_id / no_160 / cores`.
Adding the PCI id alone leaves `sc->sc_cfg` unset. So there are two entries.

The `iwl_dev_info_table` entry uses the explicit `_IWL_DEV_INFO` form rather than the
convenience `IWL_DEV_INFO()` macro, because that macro passes `IWL_CFG_ANY` for `mac_type`
and `rf_type` — with a `0x7740` device id it would then match on device id alone and could
capture unrelated silicon. `mac_type` and `rf_type` are therefore given their real values,
`IWL_CFG_MAC_TYPE_BZ` and `IWL_CFG_RF_TYPE_FM`, both of which already existed in
`if_iwxvar.h` but were never referenced anywhere.

`mac_step`, `rf_id`, `no_160` and `cores` are left `IWL_CFG_ANY` deliberately. Their values
for this part were not captured, and the struct fields are `uint8_t` while `IWL_CFG_ANY` is
`~0`, so inventing a number would both be a guess and risk a truncation warning.

### The firmware name is taken from the hardware, not from the docs

Linux's tree names its Bz firmware prefix `iwlwifi-bz-a0-fm-c0` (`cfg/bz.c`). This card is
**not** on `a0`. Running this exact BE201 under Linux `iwlwifi` on this machine reported:

```
loaded firmware version 106.c88d33af.0 bz-b0-fm-c0-c106.ucode
Detected RF FM, rfid=0x20112200
```

so the module requested `bz-b0-fm-c0` with core `106`, and that is what is used here. The
`-c` in `-c106` is not decoration: Linux's `FW_API_ARG` (`iwl-config.h:238`) emits `"c"%d`
when the number is a *core* number rather than an API number, which is why both `-106.ucode`
and `-c106.ucode` exist in linux-firmware. The `-c106` variant is what the silicon asked for.

`num_rbds` is left unset. Note this is **moot in either direction**: `num_rbds` is declared
and assigned but never read anywhere in `itlwm/` or `include/`, so the field is dead. An
earlier draft presented this as a deliberate caution; it has no effect.

### Firmware TLV header compatibility — checked, and it passes

Bundling firmware is pointless if the parser rejects it, so the header was compared against
a currently-supported image. `struct iwx_tlv_ucode_header` (`if_iwxreg.h:1612`) begins with
`uint32_t zero; uint32_t magic;`:

```
bz-b0-fm-c0-c106.ucode :  00 00 00 00  49 57 4c 0a   ("IWL\n")
ty-a0-gf-a0-68.ucode   :  00 00 00 00  49 57 4c 0a   ("IWL\n")
```

Identical leading layout, so `iwx_read_firmware()`'s magic check
(`IWX_TLV_UCODE_MAGIC 0x0a4c5749`) and TLV walk will accept the Bz image. That is a genuine
positive result — but it only means the image can be *parsed*, not that the contents are
understood or sufficient.

Name resolution was also verified rather than assumed: `scripts/zlib_compress_fw.py` mangles
a file's identifier but emits `IWL_FW("<original name>", ...)`, `IWL_FW` sets `.name` to that
literal (`FwData.h:29`), and `getFWDescByName()` matches with `strcmp`. The `.fwname` string
here is exactly the bundled basename, so lookup succeeds.

The PNVM sits beside the ucode as `iwlwifi-bz-b0-fm-c0.pnvm`, which matches how
`iwx_read_pnvm()` derives the name (truncate at the last `-`, append `.pnvm`).

### What Bz needs that is NOT here

Linux's `iwl_bz_mac_cfg` / `iwl_bz_base` (`cfg/bz.c`) specifies, among other things:

```
.num_of_queues        = 512
.max_tfd_queue_size   = 65536
.gen2                 = true
.integrated           = true
.apmg_not_supported   = true
.shadow_reg_enable    = true
.pcie_l1_allowed      = true
.umac_prph_offset     = 0x300000
.mac_addr_from_csr    = 0x30
.xtal_latency         = 12000
.ltr_delay            = IWL_CFG_TRANS_LTR_DELAY_2500US
```

`itlwm`'s `iwl_cfg_trans_params` has fields for only `integrated`, `low_latency_xtal`,
`xtal_latency` and `ltr_delay`. There is **no field** for queue count, TFD queue size,
`apmg_not_supported`, `shadow_reg_enable`, `pcie_l1_allowed`, `umac_prph_offset` or
`mac_addr_from_csr`. That is why the config here is partial by construction rather than by
oversight, and it is the main reason this cannot be expected to work: a 512-queue, 65536-entry
TFD part will not initialise on a driver that builds its rings and queue parameters for
AX210's geometry.

`iwx` also has **no MLO concept at all** — no `link_id`, no `deflink`, no per-link station or
key state. Bz is an MLD/MLO part, so this is not a gap that can be papered over at the edges.

### On the `device_family` choice, and what the inherited path actually gives Bz

`IWX_DEVICE_FAMILY_BZ` (value 3) is a new constant rather than reusing
`IWX_DEVICE_FAMILY_AX210`.

**An earlier draft claimed a new value means existing branches "take their `else` path, which is
at least visibly wrong rather than silently mistaken for AX210 behaviour". That was wrong.**
Because Bz is numbered *after* AX210, the 25 occurrences of

```c
sc->sc_device_family >= IWX_DEVICE_FAMILY_AX210
```

all evaluate **TRUE**, so Bz inherits the AX210/gen3 path at those sites. Only the 4 equality
comparisons fall through. It is a mix, and the inherited path is the silent one.

**But an audit of what that path actually provides Bz is more reassuring than the first reading
suggested** — checked against Linux's `iwl_bz_base`, not assumed:

| concern | finding |
|---|---|
| TFD queue size | `IWX_TFD_QUEUE_SIZE_MAX_GEN3` = 65536 = Linux `max_tfd_queue_size` for Bz. **Matches.** |
| Queue count / array sizing | `struct iwx_softc`'s `txq[]` is **already** dimensioned `IWX_MAX_TVQM_QUEUES` = 512 = Linux `num_of_queues` for Bz. The array is correctly sized without any Bz-specific code. |
| Context-info format | Linux **consolidated** this: a single `iwl_pcie_ctxt_info_init()` sets `version.version = 0` for all current devices including Bz, and `ctxt-info-gen3.c` no longer exists. Bz's context-info is expected to match the gen3 layout. |
| `IWX_MAX_QUEUES` = 31 | All four uses are **diagnostic dump loops** (`iwx_nic_error` / driver-status printing). They under-report for Bz but cannot cause a functional failure. |

So the obvious failure — "Bz driven with AX210's smaller queue geometry" — **did not reproduce on
inspection**. What remains genuinely unverified is whether *every* one of the 25 sites is right;
`IWX_MIN_256_BA_QUEUE_SIZE_GEN3` and the RX descriptor sizing in particular have not been checked
against Bz. Read this as "the obvious failure was not found", not as a clean bill of health. None
of it is tested on hardware.

This audit is also recorded beside the constant in `if_iwxvar.h` so it is not rediscovered later.

### What Bz needs that is NOT here

Linux's `iwl_bz_mac_cfg` / `iwl_bz_base` (`cfg/bz.c`) specifies, among other things:

```
.num_of_queues        = 512
.max_tfd_queue_size   = 65536
.gen2                 = true
.integrated           = true
.apmg_not_supported   = true
.shadow_reg_enable    = true
.pcie_l1_allowed      = true
.umac_prph_offset     = 0x300000
.mac_addr_from_csr    = 0x30
.xtal_latency         = 12000
.ltr_delay            = IWL_CFG_TRANS_LTR_DELAY_2500US
```

`itlwm`'s `iwl_cfg_trans_params` has fields for only `integrated`, `low_latency_xtal`,
`xtal_latency` and `ltr_delay`. There is **no field** for queue count, TFD queue size,
`apmg_not_supported`, `shadow_reg_enable`, `pcie_l1_allowed`, `umac_prph_offset` or
`mac_addr_from_csr`. That is why the config here is partial by construction rather than by
oversight, and it is the main reason this cannot be expected to work: a 512-queue, 65536-entry
TFD part will not initialise on a driver that builds its rings and queue parameters for
AX210's geometry.

`iwx` also has **no MLO concept at all** — no `link_id`, no `deflink`, no per-link station or
key state. Bz is an MLD/MLO part, so this is not a gap that can be papered over at the edges.

### On the `device_family` choice — and a warning that must not be missed

`IWX_DEVICE_FAMILY_BZ` (value 3) is a new constant rather than reusing
`IWX_DEVICE_FAMILY_AX210`.

**An earlier draft of this document claimed that a new value means existing branches "take
their `else` path, which is at least visibly wrong rather than silently mistaken for AX210
behaviour". That was wrong, and the opposite is true.** Because Bz is numbered *after* AX210,
the 25 occurrences of

```c
sc->sc_device_family >= IWX_DEVICE_FAMILY_AX210
```

all evaluate **TRUE** for Bz. Bz therefore **silently inherits AX210/gen3 transport geometry**
at those sites — TFD queue size, RX descriptor format, `rb_status` size, BA queue size,
context-info selection. Only the 4 equality comparisons (`== AX210`, `== 22000`) fall through
to neither branch. The behaviour is a *mix*, and the inherited path is the dangerous one
precisely because it is silent.

Whether each inherited value is right for Bz is **unverified**. It is coincidentally correct in
at least one place — `IWX_TFD_QUEUE_SIZE_MAX_GEN3` is 65536, equal to Linux `iwl_bz_base`'s
`max_tfd_queue_size`. It is certainly wrong for `num_of_queues`, where Linux uses 512 and the
inherited path assumes AX210's smaller geometry.

This is the single most important reason the port is incomplete, and it is called out in the
header beside the constant so nobody has to rediscover it.

---

## Build status: COMPILES, LINKS, AND LOADS — via the aux kernel collection, NOT OpenCore

**Status as of the latest investigation.** An earlier revision of this document claimed the kext
loads because "OpenCore prelinks injected kexts at boot". **That was wrong**, and the error was
material — it sent the investigation after a non-existent personality defect for many rounds.

### What actually happens

OpenCore's `Kernel > Add` prelink on this VM maps the kext's code without registering its
`IOKitPersonalities`. The result is deceptive:

```
$ kmutil showloaded | grep itlwm
   70    0 0xffffff80040e6000  com.zxystd.itlwm (2.4.0) 7F6CA251-...
```

The kext appears **loaded** — and `kextutil`/`kmutil load` even refuses to unload it ("kext is in
use or retained") — but **no personality is ever registered**, so `probe()` is never called and the
instance counts stay at zero. *Loaded code is not a registered personality*, and conflating the two
cost a great deal of time.

**The path that actually works is the macOS auxiliary kernel collection.** Installing to
`/Library/Extensions` and building the collection explicitly:

```
kmutil create -z -n aux ... -B ... -S ... -A ... -r /Library/Extensions
```

With that, the personality fires: `iwx_match(0x7740)` returns 1 and `probe()` returns `this`. The
kext then loads from `0xffffff7f...` and `start()` runs. Requires SIP reduced
(`csr-active-config=430A0000`); `aux-install-itlwm.sh` does the install and verifies the kext is
present in the built collection.

### The driver attaches to the device and runs — but does not come up

This is real progress over earlier revisions of this document, which said the driver "has never been
exercised on the device". It has now been exercised extensively, with this verified chain:

| stage | result |
|---|---|
| kext loads via aux KC | yes |
| personality fires | yes — `iwx_match(0x7740)` = 1 |
| `probe()` | returns `this` |
| `start()` | passes **all** setup, reaches `initWithController` |
| `ItlIwx::attach` → `iwx_attach` → `iwx_preinit` | reached |
| `iwx_start_hw` | **FAILS** — `FAIL-apm_init` |
| `iwx_apm_init` | **FAILS** — `MAC_STATUS` poll times out, returns `ETIMEDOUT` |

So bring-up stops in power-management initialisation. Firmware is never loaded, no rings are set up,
no scan is possible. **The driver attaches but does not complete bring-up.**

### What was fixed to get this far

The BE201 is a Bz-generation part with a substantially different register layout, and the fork now
implements the Bz-specific controls identified from Linux's `iwlwifi`:

- access request via `BZ_MAC_ACCESS_REQ` (GP_CNTRL bit 21), status via `MAC_STATUS` (bit 20) —
  the legacy bits are bit 3 / bit 0 and Bz does not respond to them;
- MAC bring-up via `MAC_INIT` (bit 6);
- soft reset via GP_CNTRL bit 31, with reset ordered **before** `prepare_card_hw`;
- bus-master stop via bits 29/28.

All four are confirmed to take effect on hardware: `GP_CNTRL` moves from an immovable `0x09000000`
to `0x09200000` and `0x09200040`, and the soft reset is demonstrably performed (`GP_CNTRL` reads the
`0xa5a5a5a2` bus test pattern while the device is held in reset).

### The unresolved problem, stated precisely

The port now issues the same commands, in the same order, with the same bit positions as Linux, and
the device accepts and executes each one — **yet `MAC_STATUS` never asserts**. Linux, on this same
card in this same VM, loads firmware, detects the RF, reads the MAC address and associates. The
remaining difference is therefore *not* another register bit in this sequence; it appears to lie
outside it (host-platform/ACPI differences, or the unapplied `umac_prph_offset = 0x300000` access
window).

**Nothing here is proven working on the device.** The honest summary is: *compiles, links, loads
(via the aux KC), attaches, and runs — but bring-up does not complete, and no Bz code has finished
initialising.*

`KEXT-LOADING-ON-MACOS15.md` is the full investigation log, including every hypothesis tested and
every conclusion subsequently retracted.

## (earlier) Build verification — CLT-only macOS 15.8.1

**This was compiled.** The earlier draft stated it could not be built in this environment,
because the authoring host is Linux with no `xcodebuild`. That was true of *that* host but
wrong as a conclusion about the code. It was built on a macOS 15.8.1 (24H32) x86_64 VM with
**only the Command Line Tools** — no full Xcode.

```
71 sources compiled, 0 failed, 0 diagnostics           EXIT=0
build/itlwm.kext/Contents/MacOS/itlwm:
    Mach-O 64-bit kext bundle x86_64   (16,803,168 bytes)
otool -hv -> MH_MAGIC_64 X86_64 KEXTBUNDLE ... flags NOUNDEFS DYLDLINK TWOLEVEL
```

`itlwm/hal_iwx/ItlIwx.cpp` — the file carrying all the new Bz code — compiled with **exit 0 and
an empty log**, both at `-O0` and at `-O1` (Xcode's Release level). The Bz symbols
(`iwl_bz_trans_cfg`, `iwl_cfg_bz_b0_fm_c0`, `iwl_bz_name`) are present in the object, and both
Bz firmware-name strings appear in the linked binary. A full clean rebuild (deleting `build/`
*and* the generated `FwBinary.cpp`) reproduces this in ~24 s.

`xcodebuild` alone is not enough on a CLT-only box. Two project settings must be reproduced by
hand, and getting them wrong looks like 22 source errors when it is not:

1. **`GCC_INPUT_FILETYPE = sourcecode.cpp.cpp`** — Xcode compiles *every* source as C++,
   including the `.c` files. Compiled as C, `itl80211/openbsd/net80211/ieee80211.c` dies with
   `unknownown type name 'class'` (via `sys/timeout.h` -> `<sys/CTimeout.hpp>` -> IOKit C++
   headers) and `itl80211/zutil.c` with `expected identifier or '('` (it has a bare
   `extern "C" {`). Fix: `clang++ -std=gnu++14 -x c++`.
2. **Project-level defines** `ITLWM_VERSION="2.4.0" GIT_COMMIT="" IEEE80211_STA_ONLY` —
   without them `itlwm/ItlNetworkUserClient.cpp:103` fails on undeclared `ITLWM_VERSION`
   and `GIT_COMMIT`.

A reusable build script capturing all of this is included as `build-kext-clt.sh`
(it reads the source list from the Xcode project's Sources phase: 28 `.cpp` + 43 `.c`).

### What this does NOT establish

- **It has not been loaded, and not run on hardware.** The build VM has no BE201 passed
  through, so there is no claim of loadability, and none of the runtime paths are exercised.
  Compiling is not working.
- `kextlibs` reports 588 unresolved symbols; that is a CLT tooling limitation (it cannot find a
  kernel collection to resolve against), not a link failure — the link completed with NOUNDEFS
  and the symbols are ordinary kernel KPI imports.
- The Bz transport geometry is still wrong in 25 places (see the inheritance warning above).
  A clean compile says nothing about whether the device would initialise.

### Independent checks that passed

- `CSR_HW_RFID_TYPE(0x20112200)` = `0x112` = `IWL_CFG_RF_TYPE_FM`, so the new `dev_info` entry's
  `rf_type` genuinely matches the rfid this part reported.
- `CSR_HW_RFID_IS_CDB(0x20112200)` = 0, matching the entry's `IWL_CFG_NO_CDB`.
- `_IWL_DEV_INFO` argument mapping verified arg-for-arg; the `uint8_t` `ANY` fields truncate to
  `0xFF` while the compared values are bounded (step 0-3, rf_id 0-15, no_160 0-1, cores 0-7),
  so `ANY` semantics survive and no collision is possible.
- PNVM name derivation lands exactly on `iwlwifi-bz-b0-fm-c0.pnvm`, which is in the table.
- TLV header `0000 0000 4957 4c0a` matches `IWX_TLV_UCODE_MAGIC` and is layout-identical to a
  supported image.
- `iwl_bz_trans_cfg`'s fields are genuinely consumed (copied into the softc and read for
  `ltr_delay` / `low_latency_xtal` / `xtal_latency`), so the struct is not inert.

## Firmware licence

The two blobs added under `itlwm/firmware/` are Intel's, taken from `linux-firmware` at the
same version the Linux driver on this machine loads. `itlwm` already ships 35 firmware images
under `iwlwifi-firmware-license` (a 2046-byte notice at the repo root, referenced only in an
Xcode navigator group), which states *"Redistribution and use in binary form, without
modification, are permitted"* and *"No reverse engineering, decompilation, or disassembly of
this software is permitted."* — so redistributing these unmodified is consistent with how the
project already handles firmware, but the maintainers should confirm that judgement.

---

## Reproducing the build

```sh
# on macOS with Command Line Tools (no full Xcode needed)
cd itlwm-be201
git clone --depth=1 https://github.com/acidanthera/MacKernelSDK.git
python3 scripts/zlib_compress_fw.py include/FwBinary.cpp itlwm/firmware/
./build-kext-clt.sh          # -> build/itlwm.kext
```

`build-kext-clt.sh` is new in this branch and exists because `xcodebuild` refuses to run on a
CLT-only install. It is not a substitute for the Xcode project -- it is a way to compile-check
the tree on a machine that has no Xcode.

## Suggested review order

1. `itlwm/hal_iwx/if_iwxvar.h` — the new family constant and what it does/does not carry.
2. `itlwm/hal_iwx/ItlIwx.cpp` — `iwl_bz_trans_cfg`, `iwl_cfg_bz_b0_fm_c0`, the `iwx_devices[]`
   and `iwl_dev_info_table` entries, and `iwl_bz_name`.
3. `itlwm/firmware/iwlwifi-bz-b0-fm-c0-c106.ucode` (+ `.pnvm`) — binaries.

## What would actually be needed to finish

1. Port Bz's transport geometry (queue count, TFD size, the missing trans flags) into
   `iwl_cfg`/`iwl_cfg_trans_params` — a struct change, not a table entry.
2. Determine whether Bz's context-info loading can reuse the gen2 path or needs its own.
3. Implement MLO/multi-link state, which the current iwx HAL has no representation for.
4. Build on macOS with full Xcode; load on real hardware; iterate.
