/*
* Copyright (C) 2020  钟先耀
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*/

#ifndef ItlHalService_hpp
#define ItlHalService_hpp

#include <libkern/c++/OSObject.h>
#include <libkern/c++/OSString.h>
#include <IOKit/IOLib.h>
#include <IOKit/IOService.h>
#include <IOKit/IOCommandGate.h>
#include <IOKit/IOWorkLoop.h>
#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/network/IOEthernetController.h>
#include <IOKit/network/IOEthernetInterface.h>

#include "ItlDriverInfo.hpp"
#include "ItlDriverController.hpp"

#include <net80211/ieee80211_var.h>

class ItlHalService : public OSObject {
    OSDeclareAbstractStructors(ItlHalService)
    
public:
    
    virtual bool attach(IOPCIDevice *device) = 0;
    
    virtual void detach(IOPCIDevice *device) = 0;
    
    virtual IOReturn enable(IONetworkInterface *interface) = 0;
    
    virtual IOReturn disable(IONetworkInterface *interface) = 0;
    
    virtual struct ieee80211com *get80211Controller() = 0;
#ifdef AIRPORT_WCL
    virtual IOReturn setScanHomeAwayTime(uint32_t) { return kIOReturnUnsupported; }
#endif
    
    virtual ItlDriverInfo *getDriverInfo() = 0;
    
    virtual ItlDriverController *getDriverController() = 0;
    
    /*
     * Read-only control channel (round S20/S21).  Returns a one-line status blob,
     * or NULL if this HAL has no control channel.
     *
     * Deliberately NOT pure virtual: hal_iwm and hal_iwn inherit this default and
     * are untouched, so only hal_iwx needs to implement it.  There is deliberately
     * no "up" command -- that would drive the path the tool calls rather than the
     * path the driver takes at boot, and would need a new state machine.
     *
     * CONTRACT for implementors, learned from this session's failures:
     *  - report the RETURN CODE of every device-memory read alongside its values
     *    (a zero-filled buffer that cannot distinguish "read zeros" from "read
     *    failed" is what made one earlier reading ambiguous);
     *  - PURE READ.  No device register writes.  If any read takes the nic lock
     *    and clears BZ_MAC_ACCESS_REQ (CSR_GP_CNTRL BIT(21)) -- as iwl_read_prph
     *    does -- perform it LAST and say so in the blob, so nobody samples
     *    GP_CNTRL after it and believes the cleared bit.
     */
    virtual OSString *getCtlStatus(void) { return NULL; }

    /* S217: the write-capable diagnostic channel (drain control, deterministic recovery).
     * Not pure: every HAL that does not implement it returns NULL and the caller publishes
     * "this HAL has no S217 channel". */
    virtual OSString *ctlS217(const char *cmd) { (void)cmd; return NULL; }
    
    virtual void free() override;

public:
    virtual bool initWithController(IOEthernetController *controller, IOWorkLoop *workloop, IOCommandGate *commandGate);
    
protected:
    
    int tsleep_nsec(void *ident, int priority, const char *wmesg, int timo);
    
    void wakeupOn(void* ident);
    
    IOEthernetController *getController();
    
    IOCommandGate *getMainCommandGate();
    
    IOWorkLoop *getMainWorkLoop();
    
private:
    IOEthernetController *controller;
    IOCommandGate *mainCommandGate;
    IOWorkLoop *mainWorkLoop;

    lck_grp_t *inner_gp;
    lck_grp_attr_t *inner_gp_attr;
    lck_attr_t *inner_attr;
    lck_mtx_t *inner_lock;
};

#endif /* ItlHalService_hpp */
