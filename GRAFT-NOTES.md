# S244 — AirportItlwm/WCL graft onto the shipped BE201 keeper

**Host-only tree. No card, no boot, no install, no virsh was used to make it.**

```
BASE      itlwm-be201-live @ fc88528 (our Bz tree, 0 dirty)
PATCH     wifi_work/10-STASH-itlwm-be201/patches/0001-be201-shipped-fixes.patch
          (115 hunks, one file)  ->  ItlIwx.cpp md5 924ced6a9c19606977a8181fdb9b0d05, 31,898 lines
HEADERS   wifi_work/10-STASH-itlwm-be201/source/{if_iwxvar.h,if_iwxreg.h}
          md5 adbcf3c20ba08178912cd038d15bfacf / c204fe31e60d37dc2c830df929336899
          (1,089 / 9,479 lines — the pair the shipped cpp actually compiles against;
           the live-tree headers LACK the five fixes' defines)
TARGET    wifi_work/05-artifacts/s103bo/mw2 @ 82f0ba2, AirportItlwm/ 22 files
          + WCL/WCL.{cpp,hpp}, macOS15/macOS26 headers, ItlDma, ItlTx
```

## What is in this tree

* the shipped keeper source, byte-identical for its first 31,898 lines except the
  scan-SSID/timing re-expression described below;
* all **19** `AIRPORT_WCL` regions from `mw2/itlwm/hal_iwx/ItlIwx.cpp`: **17 are
  byte-identical** to theirs; 2 are re-expressed because our file carries state
  theirs does not:
  * `iwx_start` — theirs calls `runAction(_iwx_start_task, &ifp)`; ours keeps the
    S202 witness, so it calls `runAction(_iwx_start_task, &ifp, &ran202)`;
  * `iwx_init_task` — theirs replaces the whole itlwm branch; ours keeps the
    shipped **#5a re-arm byte-identical inside `#else`** and uses their
    airport-native recovery body on the `AIRPORT_WCL` side. **#5a is not carried
    on the WCL path** (the recorded non-port); it is not dead code either, because
    the `#else` still compiles it for the itlwm target.
* the scan builders were re-expressed to support WCL directed scans:
  3 locals (`scanSSIDLength`, `scanSSID`, `scanActive`) + guarded `maxOut`.
  Under `!AIRPORT_WCL` every substitution is value-identical.
* the additive WCL surface from the merged base:
  `ieee80211com.ic_wcl_*` fields + `ieee80211_wcl_ac()` + `IEEE80211_EVT_DRIVER_*`
  (4 purely additive hunks), `ItlHalService::setScanHomeAwayTime`,
  `ItlIwx::setScanHomeAwayTime`, WCL counter init, and the 22-file
  `AirportItlwm/` target with its 10 schemes.

## Personalities

`0x77408086` was added to the three legacy plists (`AirportItlwm/Info.plist`,
`AirportItlwm-Monterey-Info.plist`, `AirportItlwm-Sonoma-Info.plist`): 87 -> 88 IDs,
matching the two WCL plists.  `grep 0x7740` is 1/1/1/1/1 across the five plists.

## Stage-1 build setting

The Xcode project is theirs, with **`AIRPORT_VTD` removed from the four
Sequoia/Tahoe preprocessor-definition lists**.  Reason: the VTD layer is a
separate 29-region DMA/transport subsystem (`ItlDma` arena, `bus_dma_tag_t`
retyping, 4 `compat.cpp` hunks).  WCL does not need it, and our legacy DMA path
is the one the keeper actually measured.  The ItlDma files are present so stage 2
is a define flip + 29 guarded regions, not a new port.

## Five fixes

The graft diff against the shipped cpp is **additions only, except 34 lines of
scan-SSID/timing substitution**.  No line of #1 (strip), #2 (handshake gate),
#3 (token), #4 (per-queue fuse) or #5a is changed; #5a is wrapped, not edited.

## Compile status

**NOT COMPILED HERE. The host has no macOS SDK.**  An honest attempt with
Linux clang 21 + MacKernelSDK fails before reaching our code: MacKernelSDK lacks
the macOS SDK's `OSClassLoadInformation` and its IOKit headers are not a kext
SDK.  What it takes is in `RESULT-S244.md` §7.

## Hygiene

`itlwm/Info.plist` and `build/itlwm.kext/Contents/Info.plist` carried a
`WiFiConfig` password (our tree's, not mw2's).  Both values were replaced with
`WIFI-PASSWORD-REMOVED-S244-HYGIENE`; **no value was read out or printed**.
The AirportItlwm plists never contained one.
