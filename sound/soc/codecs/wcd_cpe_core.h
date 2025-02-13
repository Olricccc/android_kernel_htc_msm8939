/* Copyright (c) 2013-2015, 2018, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <soc/qcom/ramdump.h>
#include <linux/dma-mapping.h>
#include "wcd_cpe_services.h"

#define WCD_CPE_LAB_MAX_LATENCY 250
#define WCD_CPE_MAD_SLIM_CHANNEL 140

#define WCD_CPE_BLK_READY  (1 << 0)
#define WCD_CPE_BUS_READY (1 << 1)

#define WCD_CPE_READY_TO_DLOAD	\
	(WCD_CPE_BLK_READY | WCD_CPE_BUS_READY)

#define WCD_CPE_LOAD_IMEM (1 << 0)
#define WCD_CPE_LOAD_DATA (1 << 1)
#define WCD_CPE_LOAD_ALL \
	(WCD_CPE_LOAD_IMEM | WCD_CPE_LOAD_DATA)

enum {
	WCD_CPE_LSM_CAL_AFE = 0,
	WCD_CPE_LSM_CAL_LSM,
	WCD_CPE_LSM_CAL_MAX,
};

enum cpe_err_irq_cntl_type {
	CPE_ERR_IRQ_MASK = 0,
	CPE_ERR_IRQ_UNMASK,
	CPE_ERR_IRQ_CLEAR,
	CPE_ERR_IRQ_STATUS,
};

struct wcd_cpe_cdc_cb {
	
	int (*cdc_clk_en) (struct snd_soc_codec *, bool);

	
	int (*cpe_clk_en) (struct snd_soc_codec *, bool);
	int (*cdc_ext_clk)(struct snd_soc_codec *codec, int enable, bool dapm);
	int (*lab_cdc_ch_ctl)(struct snd_soc_codec *codec, u8 event);
	int (*bus_vote_bw)(struct snd_soc_codec *codec,
			   bool vote);

	
	int (*cpe_err_irq_control)(struct snd_soc_codec *codec,
				    enum cpe_err_irq_cntl_type cntl_type,
				    u8 *status);
};

enum wcd_cpe_ssr_state_event {
	
	WCD_CPE_INITIALIZED = 0,
	
	WCD_CPE_IMEM_DOWNLOADED,
	
	WCD_CPE_ENABLED,
	
	WCD_CPE_ACTIVE,
	
	WCD_CPE_BUS_DOWN_EVENT,
	
	WCD_CPE_SSR_EVENT,
	
	WCD_CPE_BUS_UP_EVENT,
};

struct wcd_cpe_ssr_entry {
	int offline;
	u32 offline_change;
	wait_queue_head_t offline_poll_wait;
	struct snd_info_entry *entry;
};

struct wcd_cpe_irq_info {
	int cpe_engine_irq;
	int cpe_err_irq;
	u8 cpe_fatal_irqs;
};

struct wcd_cpe_hw_info {
	u32 dram_offset;
	size_t dram_size;
	u32 iram_offset;
	size_t iram_size;
};

struct wcd_cpe_core {
	
	void *cpe_handle;

	
	void *cpe_reg_handle;

	
	void *cmi_afe_handle;

	
	struct snd_soc_codec *codec;

	
	struct device *dev;

	
	char fname[64];

	
	struct cpe_svc_codec_info_v1 cdc_info;

	
	struct work_struct load_fw_work;

	
	int cpe_debug_mode;

	
	const struct wcd_cpe_cdc_cb *cpe_cdc_cb;

	
	struct work_struct ssr_work;

	
	struct pm_qos_request pm_qos_req;

	
	struct completion offline_compl;

	
	struct wcd_cpe_ssr_entry ssr_entry;

	struct completion ready_compl;

	
	u8 ready_status;

	
	enum wcd_cpe_ssr_state_event ssr_type;

	
	struct mutex ssr_lock;

	struct mutex session_lock;

	struct cal_type_data *cal_data[WCD_CPE_LSM_CAL_MAX];

	
	struct completion online_compl;

	
	u8 cpe_users;

	
	void *cpe_ramdump_dev;
	struct ramdump_segment cpe_ramdump_seg;
	dma_addr_t cpe_dump_addr;
	void *cpe_dump_v_addr;

	
	u32 sfr_buf_addr;
	size_t sfr_buf_size;

	
	struct wcd_cpe_irq_info irq_info;

	
	struct wcd_cpe_hw_info hw_info;
};

struct wcd_cpe_params {
	struct snd_soc_codec *codec;
	struct wcd_cpe_core * (*get_cpe_core) (
				struct snd_soc_codec *);
	const struct wcd_cpe_cdc_cb *cdc_cb;
	int dbg_mode;
	u16 cdc_major_ver;
	u16 cdc_minor_ver;
	u32 cdc_id;

	struct wcd_cpe_irq_info cdc_irq_info;

	struct cpe_svc_init_param *cpe_svc_params;
};

int wcd_cpe_ssr_event(void *core_handle,
		      enum wcd_cpe_ssr_state_event event);
struct wcd_cpe_core *wcd_cpe_init(const char *,
struct snd_soc_codec *, struct wcd_cpe_params *params);
