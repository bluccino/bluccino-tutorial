//==============================================================================
// bl_dcomp.h
// device composition for multi model mesh demo based wireless (mesh) core
//
// Created by Hugo Pristauz on 2022-Jan-02
// Copyright © 2022 Bluccino. All rights reserved.
//==============================================================================
// Bluetooth: Mesh Generic OnOff, Generic Level, Lighting & Vendor Models
// Copyright (c) 2018 Vikrant More
// SPDX-License-Identifier: Apache-2.0
//==============================================================================

#ifndef __BL_DCOMP_H__
#define __BL_DCOMP_H__

  #include "bluccino.h"

//================================================================================================
// retrieving a model pointer
//================================================================================================

  #define CONFIG_SRV0   0       // config server
  #define HEALTH_SRV0   1       // health server

  #define GONOFF_SRV0   2       // generic onoff server
  #define GONOFF_CLI0   3       // generic onoff client
  #define GLEVEL_SRV0   4       // generic level server
  #define GLEVEL_CLI0   5       // generic level client

  #define GDEFTT_SRV0   6       // generic deftt server
  #define GDEFTT_CLI0   7       // generic deftt client

  #define GPOWER_SRV0   8       // generic power onoff server
  #define GSETUP_SRV0   9       // generic power onoff setup server
  #define GPOWER_CLI0  10       // generic power onoff client

  #define LLIGHT_SRV0  11       // light lightness server 0
  #define LLSETUP_SRV0 12       // light lightness setup server 0
  #define LLIGHT_CLI0  13       // light lightness client 0

  #define LCTLT_SRV0   14       // light CTL temperature server 0
  #define LCSETUP_SRV0 15       // light CTL setup server 0
  #define LCTLT_CLI0   16       // light CTL temperature client 0

  #define HEALTH_CLI0  17       // health client

  #define VMONI_SRV0   18       // vendor monitor server

  #define GLEVEL_SRV1  19       // generic level server
  #define GLEVEL_CLI1  20       // generic level client

  #define LCTLT_SRV1   21       // light CTL temperature server 1

  #define VSIMP_SRV0   22       // vendor simple server

  struct bt_mesh_model *mesh_model_pointer(uint8_t element, uint8_t iid);

//================================================================================================
// other defines
//================================================================================================

#define CID_ZEPHYR 0x0002

#define STATE_OFF       0x00
#define STATE_ON        0x01
#define STATE_DEFAULT   0x01
#define STATE_RESTORE   0x02

/* Following 4 values are as per Mesh Model specification */
#define LIGHTNESS_MIN   0x0001
#define LIGHTNESS_MAX   0xFFFF
#define TEMP_MIN	0x0320
#define TEMP_MAX	0x4E20
#define DELTA_UV_DEF	0x0000

/* Refer 7.2 of Mesh Model Specification */
#define RANGE_SUCCESSFULLY_UPDATED      0x00
#define CANNOT_SET_RANGE_MIN            0x01
#define CANNOT_SET_RANGE_MAX            0x02

struct vendor_state {
	int current;
	uint32_t response;
	uint8_t last_tid;
	uint16_t last_src_addr;
	uint16_t last_dst_addr;
	int64_t last_msg_timestamp;
};

struct lightness {
	uint16_t current;
	uint16_t target;

	uint16_t def;
	uint16_t last;

	uint8_t status_code;
	uint16_t range_min;
	uint16_t range_max;
	uint32_t range;

	int delta;
};

struct temperature {
	uint16_t current;
	uint16_t target;

	uint16_t def;

	uint8_t status_code;
	uint16_t range_min;
	uint16_t range_max;
	uint32_t range;

	int delta;
};


struct delta_uv {
	int16_t current;
	int16_t target;

	int16_t def;

	int delta;
};

struct light_ctl_state {
	struct lightness *light;
	struct temperature *temp;
	struct delta_uv *duv;

	uint8_t onpowerup, tt;

	uint8_t last_tid;
	uint16_t last_src_addr;
	uint16_t last_dst_addr;
	int64_t last_msg_timestamp;

	struct transition *transition;
};

extern struct vendor_state vnd_user_data;
extern struct light_ctl_state *const ctl;

extern struct bt_mesh_model root_models[];
extern struct bt_mesh_model vnd_models[];
extern struct bt_mesh_model s0_models[];

extern const struct bt_mesh_comp comp;

void gen_onoff_publish(struct bt_mesh_model *model);
void gen_level_publish(struct bt_mesh_model *model);
void light_lightness_publish(struct bt_mesh_model *model);
void light_lightness_linear_publish(struct bt_mesh_model *model);
void light_ctl_publish(struct bt_mesh_model *model);
void light_ctl_temp_publish(struct bt_mesh_model *model);
void gen_level_publish_temp(struct bt_mesh_model *model);

//==============================================================================
// public module interface
//==============================================================================
//
// (C) := (BL_CORE)
//                  +--------------------+
//                  |     BL_DCOMP       | device composition
//                  +--------------------+
//                  |        SYS:        | SYS: interface
// (C)->     INIT ->|       <out>        | init module, store <out> callback
//                  +--------------------+
//                  |       GOOSRV:      | GOOSRV: interface (generic onoff srv)
// (C)<-      STS <-|   @ix,<data>,val   | status [GOOSRV:STS @ix,<data>,val]
//                  +--------------------+
//
//==============================================================================

  int bl_dcomp(BL_ob *o, int val);

#endif // __BL_DCOMP_H__
