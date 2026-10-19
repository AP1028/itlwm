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
/*    $OpenBSD: if_iwxvar.h,v 1.13 2020/10/11 07:05:28 mpi Exp $    */

/*
 * Copyright (c) 2014 genua mbh <info@genua.de>
 * Copyright (c) 2014 Fixup Software Ltd.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*-
 * Based on BSD-licensed source modules in the Linux iwlwifi driver,
 * which were used as the reference documentation for this implementation.
 *
 ******************************************************************************
 *
 * This file is provided under a dual BSD/GPLv2 license.  When using or
 * redistributing this file, you may do so under either license.
 *
 * GPL LICENSE SUMMARY
 *
 * Copyright(c) 2017 Intel Deutschland GmbH
 * Copyright(c) 2018 - 2019 Intel Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * BSD LICENSE
 *
 * Copyright(c) 2017 Intel Deutschland GmbH
 * Copyright(c) 2018 - 2019 Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name Intel Corporation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************
 */

/*-
 * Copyright (c) 2007-2010 Damien Bergamini <damien.bergamini@free.fr>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <net80211/ieee80211_var.h>
#include <net80211/ieee80211_amrr.h>
#include <net80211/ieee80211_mira.h>
#include <net80211/ieee80211_radiotap.h>

#include <IOKit/network/IOMbufMemoryCursor.h>
#include <IOKit/IODMACommand.h>

#define IWL_CFG_ANY (~0)

#define IWL_CFG_MAC_TYPE_PU        0x31
#define IWL_CFG_MAC_TYPE_PNJ        0x32
#define IWL_CFG_MAC_TYPE_TH        0x32
#define IWL_CFG_MAC_TYPE_QU        0x33
#define IWL_CFG_MAC_TYPE_QUZ        0x35
#define IWL_CFG_MAC_TYPE_QNJ        0x36
#define IWL_CFG_MAC_TYPE_SO        0x37
#define IWL_CFG_MAC_TYPE_SNJ        0x42
#define IWL_CFG_MAC_TYPE_SOF        0x43
#define IWL_CFG_MAC_TYPE_MA        0x44
#define IWL_CFG_MAC_TYPE_BZ        0x46

#define IWL_CFG_RF_TYPE_TH        0x105
#define IWL_CFG_RF_TYPE_TH1        0x108
#define IWL_CFG_RF_TYPE_JF2        0x105
#define IWL_CFG_RF_TYPE_JF1        0x108
#define IWL_CFG_RF_TYPE_HR2        0x10A
#define IWL_CFG_RF_TYPE_HR1        0x10C
#define IWL_CFG_RF_TYPE_GF        0x10D
#define IWL_CFG_RF_TYPE_MR        0x110
#define IWL_CFG_RF_TYPE_FM        0x112

#define IWL_CFG_RF_ID_TH        0x1
#define IWL_CFG_RF_ID_TH1        0x1
#define IWL_CFG_RF_ID_JF        0x3
#define IWL_CFG_RF_ID_JF1        0x6
#define IWL_CFG_RF_ID_JF1_DIV        0xA
#define IWL_CFG_RF_ID_HR        0x7
#define IWL_CFG_RF_ID_HR1        0x4

#define IWL_CFG_NO_160            0x1
#define IWL_CFG_160            0x0

#define IWL_CFG_CORES_BT        0x0
#define IWL_CFG_CORES_BT_GNSS        0x5

#define IWL_CFG_NO_CDB            0x0
#define IWL_CFG_CDB            0x1

#define IWL_SUBDEVICE_RF_ID(subdevice)    ((u16)((subdevice) & 0x00F0) >> 4)
#define IWL_SUBDEVICE_NO_160(subdevice)    ((u16)((subdevice) & 0x0200) >> 9)
#define IWL_SUBDEVICE_CORES(subdevice)    ((u16)((subdevice) & 0x1C00) >> 10)

/* HW REV */
#define CSR_HW_REV_TYPE(_val)          (((_val) & 0x000FFF0) >> 4)

/* HW RFID */
#define CSR_HW_RFID_FLAVOR(_val)       (((_val) & 0x000000F) >> 0)
#define CSR_HW_RFID_DASH(_val)         (((_val) & 0x00000F0) >> 4)
#define CSR_HW_RFID_STEP(_val)         (((_val) & 0x0000F00) >> 8)
#define CSR_HW_RFID_TYPE(_val)         (((_val) & 0x0FFF000) >> 12)
#define CSR_HW_RFID_IS_CDB(_val)       (((_val) & 0x10000000) >> 28)
#define CSR_HW_RFID_IS_JACKET(_val)    (((_val) & 0x20000000) >> 29)

#define CSR_HW_REV_TYPE_MSK        (0x000FFF0)
#define CSR_HW_REV_TYPE_NONE        (0x00001F0)
#define CSR_HW_REV_TYPE_QNJ        (0x0000360)
#define CSR_HW_REV_TYPE_QNJ_B0        (0x0000364)
#define CSR_HW_REV_TYPE_QU_B0        (0x0000334)
#define CSR_HW_REV_TYPE_QU_C0        (0x0000338)
#define CSR_HW_REV_TYPE_QUZ        (0x0000354)
#define CSR_HW_REV_TYPE_HR_CDB        (0x0000340)
#define CSR_HW_REV_TYPE_SO        (0x0000370)
#define CSR_HW_REV_TYPE_TY        (0x0000420)

/* RF_ID value */
#define CSR_HW_RF_ID_TYPE_JF        (0x00105100)
#define CSR_HW_RF_ID_TYPE_HR        (0x0010A000)
#define CSR_HW_RF_ID_TYPE_HR1        (0x0010c100)
#define CSR_HW_RF_ID_TYPE_HRCDB        (0x00109F00)
#define CSR_HW_RF_ID_TYPE_GF        (0x0010D000)
#define CSR_HW_RF_ID_TYPE_GF4        (0x0010E000)

/* HW_RF CHIP ID  */
#define CSR_HW_RF_ID_TYPE_CHIP_ID(_val) (((_val) >> 12) & 0xFFF)

/* HW_RF CHIP STEP  */
#define CSR_HW_RF_STEP(_val) (((_val) >> 8) & 0xF)

struct iwx_rx_radiotap_header {
	struct ieee80211_radiotap_header wr_ihdr;
	uint64_t	wr_tsft;
	uint8_t		wr_flags;
	uint8_t		wr_rate;
	uint16_t	wr_chan_freq;
	uint16_t	wr_chan_flags;
	int8_t		wr_dbm_antsignal;
	int8_t		wr_dbm_antnoise;
} __packed;

#define IWX_RX_RADIOTAP_PRESENT						\
	((1 << IEEE80211_RADIOTAP_TSFT) |				\
	 (1 << IEEE80211_RADIOTAP_FLAGS) |				\
	 (1 << IEEE80211_RADIOTAP_RATE) |				\
	 (1 << IEEE80211_RADIOTAP_CHANNEL) |				\
	 (1 << IEEE80211_RADIOTAP_DBM_ANTSIGNAL) |			\
	 (1 << IEEE80211_RADIOTAP_DBM_ANTNOISE))

struct iwx_tx_radiotap_header {
	struct ieee80211_radiotap_header wt_ihdr;
	uint8_t		wt_flags;
	uint8_t		wt_rate;
	uint16_t	wt_chan_freq;
	uint16_t	wt_chan_flags;
} __packed;

#define IWX_TX_RADIOTAP_PRESENT						\
	((1 << IEEE80211_RADIOTAP_FLAGS) |				\
	 (1 << IEEE80211_RADIOTAP_RATE) |				\
	 (1 << IEEE80211_RADIOTAP_CHANNEL))

/*
 * Bz firmware (iwlwifi-bz-b0-fm-c0-c106.ucode) carries SEVENTY-ONE
 * IWX_UCODE_TLV_SEC_RT sections. The AX210-era limit of 60 caused
 * iwx_firmware_store_section() to return EINVAL on the 61st, which
 * iwx_read_firmware propagates as its own error - so iwx_load_firmware aborts
 * with 'if (err) return err' and never reaches iwx_start_fw/ALIVE. Raised to
 * cover this image with headroom.
 */
#define IWX_UCODE_SECT_MAX 96

/*
 * fw_status is used to determine if we've already parsed the firmware file
 *
 * In addition to the following, status < 0 ==> -error
 */
#define IWX_FW_STATUS_NONE		0
#define IWX_FW_STATUS_INPROGRESS	1
#define IWX_FW_STATUS_DONE		2

enum iwx_ucode_type {
	IWX_UCODE_TYPE_REGULAR,
	IWX_UCODE_TYPE_INIT,
	IWX_UCODE_TYPE_WOW,
	IWX_UCODE_TYPE_REGULAR_USNIFFER,
	IWX_UCODE_TYPE_MAX
};

struct iwx_fw_onesect {
    void *fws_data;
    uint32_t fws_len;
    uint32_t fws_devoff;
};

struct iwx_fw_sects {
    struct iwx_fw_onesect fw_sect[IWX_UCODE_SECT_MAX];
    size_t fw_totlen;
    int fw_count;
};

struct iwx_fw_info {
	void *fw_rawdata;
	size_t fw_rawsize;
    void *pnvm_rawdata;
    size_t pnvm_rawsize;
	int fw_status;

	struct iwx_fw_sects fw_sects[IWX_UCODE_TYPE_MAX];

	/* FW debug data parsed for driver usage */
	int dbg_dest_tlv_init;
	uint8_t *dbg_dest_ver;
	uint8_t n_dest_reg;
	struct iwx_fw_dbg_dest_tlv_v1 *dbg_dest_tlv_v1;

	struct iwx_fw_dbg_conf_tlv *dbg_conf_tlv[IWX_FW_DBG_CONF_MAX];
	size_t dbg_conf_tlv_len[IWX_FW_DBG_CONF_MAX];
	struct iwx_fw_dbg_trigger_tlv *dbg_trigger_tlv[IWX_FW_DBG_TRIGGER_MAX];
	size_t dbg_trigger_tlv_len[IWX_FW_DBG_TRIGGER_MAX];
	struct iwx_fw_dbg_mem_seg_tlv *dbg_mem_tlv;
	size_t n_mem_tlv;
};

struct iwx_nvm_data {
	int n_hw_addrs;
	uint8_t hw_addr[ETHER_ADDR_LEN];

	int sku_cap_band_24GHz_enable;
	int sku_cap_band_52GHz_enable;
	int sku_cap_11n_enable;
    int sku_cap_11ac_enable;
    int sku_cap_11ax_enable;
	int sku_cap_amt_enable;
	int sku_cap_ipan_enable;
	int sku_cap_mimo_disable;
    int lar_enabled;
    bool vht160_supported;

	uint8_t valid_tx_ant, valid_rx_ant;

	uint16_t nvm_version;
};

/* max bufs per tfd the driver will use */
#define IWX_MAX_CMD_TBS_PER_TFD 2

struct iwx_host_cmd {
	const void *data[IWX_MAX_CMD_TBS_PER_TFD];
	struct iwx_rx_packet *resp_pkt;
	size_t resp_pkt_len;
	unsigned long _rx_page_addr;
	uint32_t _rx_page_order;
	int handler_status;

	uint32_t flags;
	uint16_t len[IWX_MAX_CMD_TBS_PER_TFD];
	uint8_t dataflags[IWX_MAX_CMD_TBS_PER_TFD];
	uint32_t id;
};

/*
 * DMA glue is from iwn
 */

struct iwx_dma_info {
	IOBufferMemoryDescriptor* buffer;
    bus_addr_t        paddr;
    void             *vaddr;
    bus_size_t        size;
    IOBufferMemoryDescriptor *bmd;
    IODMACommand *cmd;
};

struct iwx_tx_data {
	bus_dmamap_t	map;
	bus_addr_t	cmd_paddr;
	mbuf_t m;
	struct iwx_node *in;
    int flags;
#define IWX_TXDATA_FLAG_CMD_IS_NARROW  0x01
    uint8_t type;
};

struct iwx_tx_ring {
	struct iwx_dma_info	desc_dma;
	struct iwx_dma_info	cmd_dma;
	struct iwx_dma_info	bc_tbl;
	/*
	 * Round S52: Linux's dedicated first-TB buffer.  Each TFD's TB0 points at
	 * a 64-byte-aligned slot in its own allocation and carries a COPY of the
	 * first IWX_FIRST_TB_SIZE bytes of the command; TB1 still points into the
	 * command buffer at +IWL_FIRST_TB_SIZE (iwl_txq_get_first_tb_dma(),
	 * txq->first_tb_bufs[], pcie/gen1_2/tx-gen2.c:1390-1412).  The port used
	 * the command ring entry itself for TB0, the one structural difference left
	 * between its command TFD and the reference's.
	 */
	struct iwx_dma_info	first_tb_dma;
	struct iwx_tfh_tfd	*desc;
	struct iwx_device_cmd	*cmd;
	struct iwx_tx_data	data[IWX_MIN_256_BA_QUEUE_SIZE_GEN3];
    unsigned int    ring_count;
    unsigned int    hi_mark;
    unsigned int    low_mark;
	int			qid;
	int			queued;
	/* S223: THIS QUEUE'S OWN DEADLINE, IN SECONDS (0 = disarmed).  A CLOCK, NOT THE
	 * EVIDENCE: at zero the fuse re-reads `queued` on this same ring.  DELETE BEFORE
	 * THE PR (0dp). */
	int			wd_timer;
	int			cur;
	int			tail;
};

/*
 * ==== FIX (round S10, Build B, uncommitted): RX RING COUNT FOR Bz ====
 * Was 512 (the 22000/AX210 *non-HE* value) for every family.  Linux sizes the
 * RX queue from the per-device config, not from the family:
 *
 *   pcie/rx.c:  if (trans->trans_cfg->mq_rx_supported)
 *                   rxq->queue_size = trans->cfg->num_rbds;
 *   cfg/bz.c:   #define IWL_NUM_RBDS_BZ_EHT (512 * 16)   ->  .num_rbds = 8192
 *
 * (AX210 by contrast is IWL_NUM_RBDS_AX210_HE = 4096 and the non-HE parts are
 * 512 - so 512 was the wrong axis to key off.)  The count feeds three things:
 * the RBD/completion/status ring allocations, the ring mask in iwx_rx_intr, and
 * ctxt_info_gen3->mcr_size, where Linux writes
 * RX_QUEUE_CB_SIZE(trans->cfg->num_rbds) = ilog2(8192) = 13 and the port was
 * writing 9.  That field is inside the handoff the ROM reads before the kick.
 *
 * All 12 uses of the constant were traced before changing it:
 *   if_iwxvar.h:365 (def), :380 (struct iwx_rx_data data[] - the only memory
 *   cost: +120 KB in the softc, which already carries a 512 KB
 *   sc_cmd_resp_pkt[]), ItlIwx.cpp:1253 (mcr_size), :2993 (free_desc_dma =
 *   8192 * 16 B = 128 KB), :3012 (used_desc_dma, see the Bz descriptor below),
 *   :3020/:3095/:12619/:12796 (loops), :12531 (mask 0x1ff -> 0x1fff),
 *   :12536 (wrap) and :12544 (back-step).  All derive from the constant.
 */
#define IWX_RX_MQ_RING_COUNT	8192
/* Linux driver optionally uses 8k buffer */
#define IWX_RBUF_SIZE		4096

struct iwx_rx_data {
	mbuf_t m;
	bus_dmamap_t	map;
};

struct iwx_rx_ring {
	struct iwx_dma_info	free_desc_dma;
	struct iwx_dma_info	stat_dma;
	struct iwx_dma_info	used_desc_dma;
	void			*desc;
	void	        *stat;
	struct iwx_rx_data	data[IWX_RX_MQ_RING_COUNT];
	int			cur;
};

#define IWX_FLAG_USE_ICT	0x01	/* using Interrupt Cause Table */
#define IWX_FLAG_RFKILL		0x02	/* radio kill switch is set */
#define IWX_FLAG_SCANNING	0x04	/* scan in progress */
#define IWX_FLAG_MAC_ACTIVE	0x08	/* MAC context added to firmware */
#define IWX_FLAG_BINDING_ACTIVE	0x10	/* MAC->PHY binding added to firmware */
#define IWX_FLAG_STA_ACTIVE	0x20	/* AP added to firmware station table */
#define IWX_FLAG_TE_ACTIVE	0x40	/* time event is scheduled */
#define IWX_FLAG_HW_ERR		0x80	/* hardware error occurred */
#define IWX_FLAG_SHUTDOWN	0x100	/* shutting down; new tasks forbidden */
#define IWX_FLAG_BGSCAN		0x200	/* background scan in progress */
#define IWX_FLAG_TXFLUSH    0x400   /* Tx queue flushing in progress */

struct iwx_ucode_status {
	uint32_t uc_lmac_error_event_table[2];
	uint32_t uc_umac_error_event_table;
	uint32_t uc_log_event_table;
	unsigned int error_event_table_tlv_status;

	int uc_ok;
	int uc_intr;
};

#define IWX_ERROR_EVENT_TABLE_LMAC1	(1 << 0)
#define IWX_ERROR_EVENT_TABLE_LMAC2	(1 << 1)
#define IWX_ERROR_EVENT_TABLE_UMAC	(1 << 2)

#define IWX_CMD_RESP_MAX PAGE_SIZE

/* lower blocks contain EEPROM image and calibration data */
#define IWX_OTP_LOW_IMAGE_SIZE_FAMILY_7000 	16384
#define IWX_OTP_LOW_IMAGE_SIZE_FAMILY_8000	32768

#define IWX_TE_SESSION_PROTECTION_MAX_TIME_MS 1000
#define IWX_TE_SESSION_PROTECTION_MIN_TIME_MS 400

enum IWX_CMD_MODE {
	IWX_CMD_ASYNC		= (1 << 0),
	IWX_CMD_WANT_RESP	= (1 << 1),
	IWX_CMD_SEND_IN_RFKILL	= (1 << 2),
	/*
	 * Round S46: do NOT rewrite a narrow (group 0) command id to LONG_GROUP
	 * before sending it.  iwx_send_cmd() does that rewrite for every narrow id
	 * ("Firmware API versions >= 50 reject old-style commands in group 0",
	 * an OpenBSD-era workaround).  Linux does NOT rewrite: its host-command
	 * builder takes group_id = iwl_cmd_groupid(cmd->id) and writes the 8-byte
	 * wide header regardless, so a narrow id goes on the wire as group 0
	 * (pcie/gen1_2/tx-gen2.c:1206, :1305-1313).  S45 measured that this
	 * firmware ignores PHY_CONFIGURATION_CMD in every shape and size, and the
	 * group is the one identity field in which the port and Linux differ for
	 * that command.
	 */
	IWX_CMD_KEEP_NARROW_GROUP	= (1 << 3),
};
enum iwx_hcmd_dataflag {
	IWX_HCMD_DFL_NOCOPY     = (1 << 0),
	IWX_HCMD_DFL_DUP        = (1 << 1),
};

#define IWX_NUM_PAPD_CH_GROUPS	9
#define IWX_NUM_TXP_CH_GROUPS	9

struct iwx_phy_ctxt {
	uint16_t id;
	uint16_t color;
	uint32_t ref;
	struct ieee80211_channel *channel;
};

struct iwx_bf_data {
	int bf_enabled;		/* filtering	*/
	int ba_enabled;		/* abort	*/
	int ave_beacon_signal;
	int last_cqm_event;
};

/**
 * struct iwx_self_init_dram - dram data used by self init process
 * @fw: lmac and umac dram data
 * @lmac_cnt: number of lmac sections in fw image
 * @umac_cnt: number of umac sections in fw image
 * @paging: paging dram data
 * @paging_cnt: number of paging sections needed by fw image
 */
struct iwx_self_init_dram {
	struct iwx_dma_info *fw;
	int lmac_cnt;
    int umac_cnt;
	struct iwx_dma_info *paging;
	int paging_cnt;
};


#define IWX_MAX_TID_COUNT 8
#define IWX_INVALID_QUEUE 0xFFFF

struct iwx_tid_data {
    uint16_t seq_number;
    uint16_t next_reclaimed;
    uint16_t qid;
    uint16_t ssn;
};

#define    INFSLP    UINT64_MAX
#ifdef DELAY
#undef DELAY
#define DELAY IODelay
#endif

struct iwl_cfg_trans_params {
    int integrated;
    int device_family;
    int low_latency_xtal;
    int ltr_delay;
    int xtal_latency;
    int bisr_workaround;
};

struct iwl_cfg {
    struct iwl_cfg_trans_params trans;
    const char *name;
    const char *fwname;
    int device_family;
    int tx_with_siso_diversity;
    int uhb_supported;
    uint8_t max_tx_agg_size;
    uint16_t num_rbds;
};

/**
 * struct iwx_reorder_buffer - per ra/tid/queue reorder buffer
 * @head_sn: reorder window head sn
 * @num_stored: number of mpdus stored in the buffer
 * @buf_size: the reorder buffer size as set by the last addba request
 * @queue: queue of this reorder buffer
 * @last_amsdu: track last ASMDU SN for duplication detection
 * @last_sub_index: track ASMDU sub frame index for duplication detection
 * @reorder_timer: timer for frames are in the reorder buffer. For AMSDU
 *    it is the time of last received sub-frame
 * @removed: prevent timer re-arming
 * @valid: reordering is valid for this queue
 * @consec_oldsn_drops: consecutive drops due to old SN
 * @consec_oldsn_ampdu_gp2: A-MPDU GP2 timestamp to track
 *    when to apply old SN consecutive drop workaround
 * @consec_oldsn_prev_drop: track whether or not an MPDU
 *    that was single/part of the previous A-MPDU was
 *    dropped due to old SN
 */
struct iwx_reorder_buffer {
    uint16_t head_sn;
    uint16_t num_stored;
    uint16_t buf_size;
    uint16_t last_amsdu;
    uint8_t last_sub_index;
    CTimeout *reorder_timer;
    int removed;
    int valid;
    unsigned int consec_oldsn_drops;
    uint32_t consec_oldsn_ampdu_gp2;
    unsigned int consec_oldsn_prev_drop;
#define IWX_AMPDU_CONSEC_DROPS_DELBA    20
};

/**
 * struct iwx_reorder_buf_entry - reorder buffer entry per frame sequence number
 * @frames: list of mbufs stored (A-MSDU subframes share a sequence number)
 * @reorder_time: time the packet was stored in the reorder buffer
 */
struct iwx_reorder_buf_entry {
    struct mbuf_list frames;
    struct timeval reorder_time;
    uint32_t rx_pkt_status;
    int chanidx;
    int is_shortpre;
    uint32_t rate_n_flags;
    uint32_t device_timestamp;
    struct ieee80211_rxinfo rxi;
};

/**
 * struct iwx_rxba_data - BA session data
 * @sta_id: station id
 * @tid: tid of the session
 * @baid: baid of the session
 * @timeout: the timeout set in the addba request
 * @entries_per_queue: # of buffers per queue
 * @last_rx: last rx timestamp, updated only if timeout passed from last update
 * @session_timer: timer to check if BA session expired, runs at 2 * timeout
 * @sc: softc pointer, needed for timer context
 * @reorder_buf: reorder buffer
 * @reorder_buf_data: buffered frames, one entry per sequence number
 */
struct iwx_rxba_data {
    uint8_t sta_id;
    uint8_t tid;
    uint8_t baid;
    uint16_t timeout;
    uint16_t entries_per_queue;
    struct timeval last_rx;
    CTimeout *session_timer;
    struct iwx_softc *sc;
    struct iwx_reorder_buffer reorder_buf;
    struct iwx_reorder_buf_entry entries[IEEE80211_BA_MAX_WINSZ];
};

static inline struct iwx_rxba_data *
iwx_rxba_data_from_reorder_buf(struct iwx_reorder_buffer *buf)
{
    return (struct iwx_rxba_data *)((uint8_t *)buf -
            offsetof(struct iwx_rxba_data, reorder_buf));
}

/**
 * struct iwx_rxq_dup_data - per station per rx queue data
 * @last_seq: last sequence per tid for duplicate packet detection
 * @last_sub_frame: last subframe packet
 */
struct iwx_rxq_dup_data {
    uint16_t last_seq[IWX_MAX_TID_COUNT + 1];
    uint8_t last_sub_frame[IWX_MAX_TID_COUNT + 1];
};

struct iwx_ba_task_data {
    uint32_t        start_tidmask;
    uint32_t        stop_tidmask;
};

/* upstream if_iwxvar.h:441-445, verbatim (S103ag B5b) */
struct iwx_setkey_task_arg {
    int sta_id;
    struct ieee80211_node *ni;
    struct ieee80211_key *k;
};

/*
 * S162: THE FIRMWARE RESET HANDSHAKE STATES, AND THE TIMEOUT.
 * One for one with Linux's `enum iwl_trans_state`-adjacent `FW_RESET_*`
 * (`pcie/gen1_2/trans-gen2.c:99-145`): `FW_RESET_IDLE` = 0, `FW_RESET_REQUESTED`,
 * `FW_RESET_OK`.  The reference's `FW_RESET_ERROR` is deliberately NOT defined:
 * it is set when an SW-error interrupt arrives during the handshake, and that
 * branch is outside this round's scope (the port's SW-error handler keeps its
 * existing behaviour; a handshake interrupted by a real firmware error simply
 * times out and is counted as such).
 * THE TIMEOUT IS THE REFERENCE'S NUMBER: `#define FW_RESET_TIMEOUT (HZ / 5)`
 * (`pcie/gen1_2/trans-gen2.c:13`) = 200 ms at HZ=1000, and it is used for the
 * `wait_event_timeout()` at `:118-120`.
 */
#define IWX_FW_RESET_TIMEOUT_MS	200
#define IWX_FW_RESET_IDLE	0
#define IWX_FW_RESET_REQUESTED	1
#define IWX_FW_RESET_OK		2

struct iwx_softc {
	struct device sc_dev;
	struct ieee80211com sc_ic;
	int (*sc_newstate)(struct ieee80211com *, enum ieee80211_state, int);
	int sc_newstate_pending;
    pci_intr_handle_t ih;

	struct task		init_task; /* NB: not reference-counted */
//	struct refcnt		task_refs;
	struct task newstate_task;
	enum ieee80211_state	ns_nstate;
	int			ns_arg;

	/* Task for firmware BlockAck setup/teardown and its arguments. */
	struct task	ba_task;

	/*
	 * ==== S103ag B5b: THE DEFERRED KEY INSTALL ===============================================
	 * upstream if_iwxvar.h:657, :665-668 (the reference's own declarations):
	 *     struct task                setkey_task;
	 *     struct iwx_setkey_task_arg setkey_arg[3];
	 *     int                        setkey_cur, setkey_tail, setkey_nkeys;
	 * WHY IT IS NOT OPTIONAL -- MEASURED ON THE PORT'S OWN LOG, not argued:
	 * every `iwx_cmd_done` completion in S103af ran on thread 0:674 (56 of 56), every
	 * `PROBE S48 rx_pkt` ran on thread 0:674 (69 of 69), and BOTH `(01.17)` sends were issued
	 * from thread 0:674 (S103af-port-log.txt:2279, :2283).  The reference's MLD install is
	 * SYNCHRONOUS (if_iwx.c:9380 passes 0, not IWX_CMD_ASYNC, when remove_key == 0), so issuing
	 * it inline from iwx_set_key() would tsleep_nsec() on the very thread that must deliver its
	 * completion: a guaranteed 1-second timeout per key.  The reference avoids exactly that by
	 * queueing the key on systq (if_iwx.c:9337-9347).  The port deleted that dispatch.
	 * =========================================================================================
	 */
	struct iwx_setkey_task_arg	setkey_arg[3];
	struct task			setkey_task;
	int				setkey_cur;
	int				setkey_tail;
	int				setkey_nkeys;
    
    struct iwx_ba_task_data    ba_rx;
    struct iwx_ba_task_data    ba_tx;
    
    struct iwx_tid_data sc_tid_data[IWX_MAX_TID_COUNT + 1];//per tid data + mgmt. Look at %iwx_tid_data.

    /* Task for ERP/HT prot/slot-time/EDCA updates. */
    struct task        mac_ctxt_task;
    struct task        chan_ctxt_task;

	bus_space_tag_t sc_st;
	bus_space_handle_t sc_sh;
	bus_size_t sc_sz;
	bus_dma_tag_t sc_dmat;
	pci_chipset_tag_t sc_pct;
	pcitag_t sc_pcitag;
	IOInterruptEventSource *sc_ih;
	int sc_msix;

	/* TX/RX rings. */
	struct iwx_tx_ring txq[IWX_MAX_TVQM_QUEUES];
	struct iwx_rx_ring rxq;
	int qfullmsk;
	/* S103y D4 -- upstream if_iwx.c uses it at :2191 (iwx_reset_tx_ring, immediately after the
	 * qfullmsk clear above), :2995 (iwx_enable_txq, on success), :3054 (iwx_disable_txq, after the
	 * firmware serves the REMOVE), :7213 (the teardown's "only disable what is enabled" predicate)
	 * and :3771/:3778 (iwx_addba).  THE INVARIANT: bit i is SET iff the firmware currently holds a
	 * queue allocated for IWX_STATION_ID.  This port READ it at ONE site only -- the teardown --
	 * because the :3771/:3778 readers are replaced here by TVQM (the firmware chooses the queue
	 * id) and the :7210-7223 AGG loop is not carryable (see iwx_rm_sta's banner).  A field with a
	 * reader is carried; a field without one would be dead state. */
	uint32_t qenablemsk;
    struct iwx_tx_ring sc_tvqm_ring;
    int first_data_qid;

	int sc_sf_state;

	/* ICT table. */
	struct iwx_dma_info	ict_dma;
	int			ict_cur;
    uint32_t sku_id[3];

	int sc_hw_rev;
#define IWX_SILICON_A_STEP	0
#define IWX_SILICON_B_STEP	1
#define IWX_SILICON_C_STEP	2
#define IWX_SILICON_D_STEP	3
    int sc_hw_rf_id;
	int sc_hw_id;
    const struct iwl_cfg_trans_params *sc_cfg_params;
    const struct iwl_cfg *sc_cfg;
	int sc_device_family;
    /*
     * ==== SUP-P05 (offline triage, STRUCT/MECHANISM fix): THE OP-MODE SELECTOR ====
     * Upstream's field, verbatim (`if_iwxvar.h:748`): `int sc_use_mld_api;`
     * Set once, at `if_iwx.c:4576-4583`:
     *     if (isset(sc->sc_enabled_capa, IWX_UCODE_TLV_CAPA_MLD_API_SUPPORT) &&
     *         (IWX_CSR_HW_REV_TYPE(sc->sc_hw_rev) == IWX_CFG_MAC_TYPE_MA ||
     *          sc->sc_device_family == IWX_DEVICE_FAMILY_BZ))
     *             sc->sc_use_mld_api = 1;
     * and read at six sites: `:5586`, `:6547`, `:6835-6836`, `:7171-7172`, `:8077-8078`, `:4583`.
     * The port has NO counterpart (0 hits for `sc_use_mld_api` in all three delivering files), so
     * it cannot express the distinction that decides which MAC-context command goes on the wire.
     * Bz satisfies the gate on this device: `IWX_UCODE_TLV_CAPA_MLD_API_SUPPORT` is bit 110 and
     * the shipped Bz image sets it (census: `07-scripts/sup-tlv48.py`, caps word idx3 bit 14).
     * ==============================================================================
     */
    int sc_use_mld_api;
#define IWX_DEVICE_FAMILY_22000	1
#define IWX_DEVICE_FAMILY_AX210	2
/*
 * Bz (Wi-Fi 7: BE200/BE201 and relatives). Added for BE201 support.
 *
 * This is a genuinely different generation from 22000/AX210, not a variation:
 * Linux's iwl_bz_base specifies num_of_queues 512, max_tfd_queue_size 65536,
 * gen2 context-info, integrated (CNVi) operation, apmg_not_supported,
 * shadow_reg_enable, pcie_l1_allowed, and low-latency xtal at 12000.
 *
 * READ THIS BEFORE TRUSTING ANY Bz CODE PATH.
 *
 * The value 3 is ORDERED AFTER the existing families, and that has a
 * consequence that is easy to get wrong: the 25 occurrences of
 *
 *     sc->sc_device_family >= IWX_DEVICE_FAMILY_AX210
 *
 * evaluate TRUE for Bz, so Bz SILENTLY INHERITS the AX210/gen3 transport
 * geometry at those sites (TFD queue size, RX descriptor format, rb_status
 * size, BA queue size, context-info selection, ...). Only the 4 equality
 * comparisons (== AX210, == 22000) fall through to neither branch.
 *
 * So the behaviour is a MIX, not "everything takes the else path".
 *
 * AUDIT of what the inherited gen3 path actually gives Bz (checked against
 * Linux's iwl_bz_base, not assumed):
 *
 *   - IWX_TFD_QUEUE_SIZE_MAX_GEN3 = 65536 == Linux max_tfd_queue_size (Bz). OK.
 *   - struct iwx_softc's txq[] is ALREADY dimensioned IWX_MAX_TVQM_QUEUES = 512,
 *     which equals Linux num_of_queues (Bz). So the queue ARRAY is correctly
 *     sized for Bz even though no Bz-specific code exists. OK.
 *   - Context-info format: Linux consolidated this. There is a single
 *     iwl_pcie_ctxt_info_init() setting version.version = 0 for all current
 *     devices including Bz; the old ctxt-info-gen3.c no longer exists. Bz's
 *     context-info is therefore expected to match the gen3 layout, so the
 *     inherited path is plausibly right here too.
 *   - The four uses of IWX_MAX_QUEUES (= 31) are DIAGNOSTIC dump loops only
 *     (iwx_nic_error / driver-status printing). They under-report for Bz but
 *     cannot cause a functional failure.
 *
 * Net: the inherited geometry is closer to correct for Bz than a naive reading
 * suggests. What remains unverified is whether every one of the 25 sites is
 * right -- in particular IWX_MIN_256_BA_QUEUE_SIZE_GEN3 and the RX descriptor
 * sizing have not been checked against Bz -- and NONE of it is tested on
 * hardware. Do not read this audit as a clean bill of health; read it as
 * "the obvious failure did not reproduce on inspection".
 *
 * ROUND 30/31 CORRECTION — the context-info bullet above is now MEASURED WRONG
 * and must not be relied on. The claim "Bz's context-info is therefore expected
 * to match the gen3 layout" was an inference, and the firmware does not agree:
 * with the inherited gen3 path, iwx_load_firmware's ALIVE handshake never
 * completes. Measured on the device (rounds 28-30):
 *
 *     A sb1=0x00000000 sb2=0x03010801 upc=0xc00c040e lpc=0xc00c040c uc_ok=0 t=35
 *     B upc=0xc00c040e lpc=0xc00c040c sb1=0x00000000 moved=NO-stalled
 *     uc_intr=0 uc_ok=0
 *
 * i.e. the image is transferred, the CPU is kicked (UREG_CPU_INIT_RUN written
 * through the correct 0x300000-offset window), the uCode begins executing and
 * then HALTS both CPUs almost immediately (pc unchanged over 500ms, both parked
 * in the 0xc00c040x region), and the ALIVE interrupt never fires. An immediate
 * double-CPU halt on the first instructions is the signature of a fault during
 * the uCode's own early init, and the context-info layout is the prime suspect:
 * Linux's iwl_bz_mac_cfg sets .gen2 = true while this port selects
 * iwx_ctxt_info_gen3_init for family >= AX210, which Bz satisfies.
 *
 * Also note Linux still has iwl-context-info-gen3.h and a gen3 init path, so the
 * "Linux consolidated this" reasoning above was itself inaccurate.
 */
#define IWX_DEVICE_FAMILY_BZ	3

	struct iwx_dma_info ctxt_info_dma;
	struct iwx_self_init_dram init_dram;
    /* For gen3 */
    struct iwx_dma_info prph_scratch_dma;
    struct iwx_dma_info prph_info_dma;
    struct iwx_dma_info iml_dma;
    struct iwx_dma_info pnvm_dram;

	int sc_fw_chunk_done;
	int sc_init_complete;
#define IWX_INIT_COMPLETE	0x01
#define IWX_CALIB_COMPLETE	0x02

	struct iwx_ucode_status sc_uc;
	char sc_fwver[32];

	int sc_capaflags;
	int sc_capa_max_probe_len;
	int sc_capa_n_scan_channels;
    int sc_capa_num_stations;
	uint8_t sc_ucode_api[howmany(IWX_NUM_UCODE_TLV_API, NBBY)];
	uint8_t sc_enabled_capa[howmany(IWX_NUM_UCODE_TLV_CAPA, NBBY)];
#define IWX_MAX_FW_CMD_VERSIONS	800
	struct iwx_fw_cmd_version cmd_versions[IWX_MAX_FW_CMD_VERSIONS];
	int n_cmd_versions;
    char sc_fw_mcc[3];
    uint16_t sc_fw_mcc_int;
#define IWX_IML_MAX_LEN 2048*10
    uint32_t sc_iml_len;
    uint8_t sc_iml[IWX_IML_MAX_LEN];

	int sc_intmask;
	int sc_flags;

	uint32_t sc_fh_init_mask;
	uint32_t sc_hw_init_mask;
	uint32_t sc_fh_mask;
	uint32_t sc_hw_mask;

	/*
	 * S162: THE FIRMWARE RESET HANDSHAKE -- THE TWO FIELDS LINUX HAS AS
	 * `fw_reset_state` + `fw_reset_waitq` (`iwl-trans.h:1046-1049`;
	 * `pcie/gen1_2/trans-gen2.c:99-145`).
	 *
	 * `sc_fw_reset_state` is Linux's `fw_reset_state`: IDLE before and after a
	 * handshake, REQUESTED from the instant the doorbell is rung until the
	 * bit-2 interrupt handler acknowledges it, OK once acknowledged.  Linux
	 * additionally has ERROR for "an SW error interrupt arrived during the
	 * handshake"; this port does not (see the note at the sender).
	 *
	 * `sc_fw_reset_wait` IS the wait channel, and it is a field rather than
	 * `&sc_fw_reset_state` for one reason: the value of the state CHANGES at
	 * exactly the moment the waiter must stop sleeping, and a channel whose
	 * contents are also the condition makes every future reader re-derive
	 * whether that is safe.  It is NEVER read or written as a value: its
	 * ADDRESS is the sleep/wakeup identity, which is the port's own idiom
	 * (`tsleep_nsec(desc, ...)` in iwx_send_cmd, and every `wakeupOn(&x)`
	 * site).  A separate lock is not introduced: the only concurrent writer is
	 * the interrupt action, every transition is one aligned int store, and the
	 * wakeup is issued AFTER the store that makes it unnecessary to sleep
	 * again -- the lost-wakeup rule the S43/S66 notes in ItlIwx.cpp state.
	 */
	int sc_fw_reset_state;		/* S162: IDLE / REQUESTED / OK              */
	int sc_fw_reset_wait;		/* S162: the handshake's wait channel       */
	/* S216: LINUX'S `trans->state >= IWL_TRANS_FW_STARTED` FOR THE HANDSHAKE.
	 * 1 from the firmware's ALIVE until the driver DECLARES the firmware dead
	 * (watchdog timeout, SW/HW error, CT kill) or SW_RESETs it.  The S162 gate
	 * used `uc_ok`, which never clears, so every recovery stop handshook a
	 * firmware that had already failed.  See iwx_stop_device(). */
	int sc_fw_started;
	/* S216: IWL_UCODE_MAJOR from the image's FW_VERSION TLV.  The TLV header's
	 * `ver` field is NOT the major on this image (it reads 0x6a = API 106); the
	 * firmware's own TLV says 106.c88d33af.0, which is what the Linux log prints
	 * on this silicon.  The MVM version guard (0eaa1f24) uses this number. */
	uint32_t sc_ucode_major;

	int sc_generation;

//	struct rwlock ioctl_rwl;

	int sc_cap_off; /* PCIe caps */

	const char *sc_fwname;
	struct iwx_fw_info sc_fw;
	struct iwx_dma_info fw_mon;
	int sc_fw_phy_config;
	struct iwx_tlv_calib_ctrl sc_default_calib[IWX_UCODE_TYPE_MAX];

	struct iwx_nvm_data sc_nvm;
	struct iwx_bf_data sc_bf;

	int sc_tx_timer;
	int sc_rx_ba_sessions;

	int sc_scan_last_antenna;

	int sc_fixed_ridx;
    
    uint8_t sc_mgmt_last_antenna_idx; /* for MGMT frames using*/

	int sc_staid;
	int sc_nodecolor;

	uint8_t *sc_cmd_resp_pkt[IWX_TFD_QUEUE_SIZE_MAX_GEN3];	
	size_t sc_cmd_resp_len[IWX_TFD_QUEUE_SIZE_MAX_GEN3];
	int sc_nic_locks;

	struct taskq *sc_nswq;

	struct iwx_rx_phy_info sc_last_phy_info;
    int sc_ampdu_ref;
#define IWX_MAX_BAID    32
    struct iwx_rxba_data sc_rxba_data[IWX_MAX_BAID];

	uint32_t sc_time_event_uid;

	/* phy contexts.  we only use the first one */
	struct iwx_phy_ctxt sc_phyctxt[IWX_NUM_PHY_CTX];

	int sc_noise;

    int sc_pm_support;
	int sc_ltr_enabled;

	int sc_integrated;
	int sc_tx_with_siso_diversity;
	int sc_max_tfd_queue_size;
    int sc_ltr_delay;
    int sc_xtal_latency;
    int sc_low_latency_xtal;
    
    int sc_uhb_supported;

#if NBPFILTER > 0
	caddr_t			sc_drvbpf;

	union {
		struct iwx_rx_radiotap_header th;
		uint8_t	pad[IEEE80211_RADIOTAP_HDRLEN];
	} sc_rxtapu;
#define sc_rxtap	sc_rxtapu.th
	int			sc_rxtap_len;

	union {
		struct iwx_tx_radiotap_header th;
		uint8_t	pad[IEEE80211_RADIOTAP_HDRLEN];
	} sc_txtapu;
#define sc_txtap	sc_txtapu.th
	int			sc_txtap_len;
#endif

    /*
     * S103u: the receipt latch for the WIDE system-statistics request.  Upstream `if_iwx.c:8194`
     * (cleared immediately before the send), `:8201-8206` (the bounded wait) and `:11356-11357`
     * (the ONLY writer, the (0x02,0xfd) END-notification rx case).  APPENDED at the END of struct
     * iwx_softc so that NO existing field offset moves -- a deliberate attribution choice, and the
     * placement is the port's own because upstream's if_iwxvar.h is not in the reference set.
     */
    int sc_system_stats_cleared;

    /*
     * ==== S195: THE RESET-PATH DIRECTED SCAN =============================================
     * APPENDED at the END of struct iwx_softc, as S103u's field above, so that NO existing
     * field offset moves.  `sc_s195_reload` is set by `iwx_watchdog`'s deadline (the fire)
     * and consumed by the `iwx_stop()` that the queued `init_task` runs; that stop is the
     * ONLY one allowed to capture the known BSS.  `sc_s195_arm`/`sc_s195_chan`/
     * `sc_s195_bssid` are that capture, consumed by the FIRST scan request after the reload
     * (`iwx_scan()` clears the arm even when the request declined it).  A userspace down, a
     * media change or an ioctl stop clears them, so a fresh association always starts with
     * a full sweep.  See `iwx_umac_scan_fill_channels()`.
     */
    int sc_s195_reload;                     /* 1 = the next iwx_stop() is a reset reload */
    int sc_s195_arm;                        /* 1 = the next scan may use one channel */
    int sc_s195_chan;                       /* the known BSS's IEEE channel (0 = none) */
    uint8_t sc_s195_bssid[ETHER_ADDR_LEN];  /* the known BSS's BSSID */

    /*
     * ==== S196: THE RUN TRANSITION'S CAPTURE (the S195 capture read the wrong instant) ======
     * S195 captured the directed scan's channel at `iwx_stop()`'s teardown, from
     * `ic_bss->ni_chan`.  That field is MOVED BY A SCAN -- `ieee80211_next_scan()` writes the
     * first scan channel into the BSS node BEFORE the driver's `ic_state` becomes SCAN, and a
     * background scan never changes the state at all -- so `st == RUN` at the teardown does NOT
     * make `ni_chan` the RUN channel.  MEASURED (S195 control boot): one reload captured
     * channel 1 while the BSSID/ESSID matched.  These fields are written at `iwx_run()`, the
     * RUN transition itself, where the node is the BSS being handed to the firmware and
     * `sc_phyctxt[0].channel` is the channel the firmware was just configured on; the teardown
     * only COPIES them, and refuses unless the same BSS is still RUN on the same PHY channel.
     * APPENDED at the END of struct iwx_softc, as S195's fields were, so no offset moves.
     */
    unsigned int sc_s196_run_seq;           /* RUN transitions observed (provenance) */
    int sc_s196_run_chan;                   /* the RUN BSS's channel, at iwx_run() */
    int sc_s196_run_esslen;
    uint8_t sc_s196_run_bssid[ETHER_ADDR_LEN];
    uint8_t sc_s196_run_essid[IEEE80211_NWID_LEN];

    /*
     * ==== S199: THE RETAINED ESS/CHANNEL SURVIVES THE DESELECT ==========================
     * `ieee80211_deselect_ess()` zeroes `ic_des_essid`/`ic_des_esslen`, and the S195/S196
     * scan site needs `n_ssids != 0` plus the captured BSS node still being the target, so
     * an error re-init after a deselect scanned PASSIVE over 41 channels (measured S198
     * out:#2: 4.6769 s = 85.5 % of the recovery).  The S196 RUN capture itself already
     * survives `sc_newstate(INIT)` and the deselect (it lives in THIS struct); this field
     * only records that the next `iwx_scan()` may spend it even though the state machine
     * is no longer RUN and may be trying another candidate of the same ESS.  ONE SHOT:
     * `iwx_scan()` consumes it whether or not the request used it, so the full sweep
     * remains the structural fallback (`0rt`).  See `iwxabS199Arm()`.
     */
    int sc_s199_direct;                     /* 1 = next scan may use the retained RUN ESS/chan */
};

struct iwx_node {
	struct ieee80211_node in_ni;
	struct iwx_phy_ctxt *in_phyctxt;
    uint8_t in_macaddr[ETHER_ADDR_LEN];

	uint16_t in_id;
	uint16_t in_color;
    
    struct iwx_rxq_dup_data dup_data;

	/*
	 * ==== S103ah B5c: THE KEY-STATE BOOKKEEPING =================================================
	 * upstream iwx if_iwxvar.h:846-849, verbatim (05-artifacts/research/
	 * FETCHED-upstream-if_iwxvar.h.v1.50).  The port's `struct iwx_node` is that struct with these
	 * four lines deleted and nothing else; MEASURED this round, not asserted.
	 * The port had no `in_flags` and no IWX_NODE_FLAG_HAVE_* anywhere, which is why S103ag could not
	 * carry the reference's completion tail (if_iwx.c:9518-9560) and DECLARED it instead.
	 * =========================================================================================
	 */
	int in_flags;
#define IWX_NODE_FLAG_HAVE_PAIRWISE_KEY		0x01
#define IWX_NODE_FLAG_HAVE_GROUP_KEY		0x02
#define IWX_NODE_FLAG_HAVE_INTEGRITY_GROUP_KEY	0x04
};
#define IWX_STATION_ID 0
#define IWX_AUX_STA_ID 1
#define IWX_MONITOR_STA_ID 2

#define IWX_ICT_SIZE		4096
#define IWX_ICT_COUNT		(IWX_ICT_SIZE / sizeof (uint32_t))
#define IWX_ICT_PADDR_SHIFT	12
