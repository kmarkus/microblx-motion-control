/*
 * an empty 7-DOF robot manipulator function block
 *
 * Copyright (C) 2020 Markus Klotzbuecher <mk@mkio.de>
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * the purpose of this block is to specify a recommended driver
 * interface and to serve as a starting point for real robot drivers.
 */

#undef UBX_DEBUG

#include <ubx.h>

/* of course this could be made configurable, but for any real robot
 * that will not be the case, so we don't bother with it here */
#define NUM_JOINTS	6

const char manipulator_block_name[] = "mc/manipulator";
const char manipulator_meta[] = "a dummy 7-DOF robot manipulator block";

enum CTRL_MODE {
	POS = 0,
	VEL,
	EFF,
	CUR,
	__CTRL_MODE_LAST__
};

const char *ctrl_modes [] = {
	"position",
	"velocity",
	"effort",
	"current",
};

ubx_proto_config_t manipulator_config[] = {
	{ .name="ctrl_mode", .type_name = "int", .min=0, .max=1, .doc="initial ctrl_mode: 0: pos, 1: vel, 2: eff, 3: cur (def: 0)" },
	{ 0 },
};

ubx_proto_port_t manipulator_ports[] = {
	{ .name="ctrl_mode", .in_type_name="int", .in_data_len=1, .doc="port to switch control mode at runtime" },
	{ .name="jnt_pos_msr", .out_type_name="double", .out_data_len=NUM_JOINTS, .doc="measured position [rad]" },
	{ .name="jnt_pos_cmd", .in_type_name="double", .in_data_len=NUM_JOINTS, .doc="commanded joint position [rad]" },
	{ .name="jnt_vel_msr", .out_type_name="double", .out_data_len=NUM_JOINTS, .doc="measured velocity [m/s]" },
	{ .name="jnt_vel_cmd", .in_type_name="double", .in_data_len=NUM_JOINTS, .doc="commanded jnt velocity [rad/s]" },
	{ .name="jnt_eff_msr", .out_type_name="double", .out_data_len=NUM_JOINTS, .doc="measured effort [N or Nm]" },
	{ .name="jnt_eff_cmd", .in_type_name="double", .in_data_len=NUM_JOINTS, .doc="commanded jnt effort [N or Nm]" },
	{ .name="jnt_cur_msr", .out_type_name="double", .out_data_len=NUM_JOINTS, .doc="measured current [A]" },
	{ .name="jnt_cur_cmd", .in_type_name="double", .in_data_len=NUM_JOINTS, .doc="commanded jnt current [A]" },
	{ 0 },
};

struct manipulator_info
{
	int ctrl_mode;

	struct port_cache {
		ubx_port_t* ctrl_mode;
		ubx_port_t* jnt_pos_msr;
		ubx_port_t* jnt_pos_cmd;
		ubx_port_t* jnt_vel_msr;
		ubx_port_t* jnt_vel_cmd;
		ubx_port_t* jnt_eff_msr;
		ubx_port_t* jnt_eff_cmd;
		ubx_port_t* jnt_cur_msr;
		ubx_port_t* jnt_cur_cmd;
	} ports;

	double pos[NUM_JOINTS];
	double vel[NUM_JOINTS];
	double eff[NUM_JOINTS];
	double cur[NUM_JOINTS];
};

int manipulator_init(ubx_block_t *b)
{
	int ret;
	long len;
	const int *ctrl_mode;
	struct manipulator_info *inf;

	inf = calloc(1, sizeof(struct manipulator_info));

	if (inf == NULL) {
		ubx_err(b, "manipulator: failed to alloc memory");
		ret = EOUTOFMEM;
		goto out;
	}

	b->private_data = inf;

	/* ctrl_mode */
	len = cfg_getptr_int(b, "ctrl_mode", &ctrl_mode);
	inf->ctrl_mode = (len == 0) ? 0 : *ctrl_mode;

	if(inf->ctrl_mode < 0 || inf->ctrl_mode >= __CTRL_MODE_LAST__) {
		ubx_err(b, "invalid ctrl_mode %i", inf->ctrl_mode);
		ret = -1;
		goto out_free;
	}

	/* cache ports */
	inf->ports.ctrl_mode = ubx_port_get(b, "ctrl_mode");
	inf->ports.jnt_pos_msr = ubx_port_get(b, "jnt_pos_msr");
	inf->ports.jnt_pos_cmd = ubx_port_get(b, "jnt_pos_cmd");
	inf->ports.jnt_vel_msr = ubx_port_get(b, "jnt_vel_msr");
	inf->ports.jnt_vel_cmd = ubx_port_get(b, "jnt_vel_cmd");
	inf->ports.jnt_eff_msr = ubx_port_get(b, "jnt_eff_msr");
	inf->ports.jnt_eff_cmd = ubx_port_get(b, "jnt_eff_cmd");
	inf->ports.jnt_cur_msr = ubx_port_get(b, "jnt_cur_msr");
	inf->ports.jnt_cur_cmd = ubx_port_get(b, "jnt_cur_cmd");

	return 0;

out_free:
	free(inf);
out:
	return ret;
}

#if 0 /* kept here as this block is to serve as	an example */
/* start */
int manipulator_start(ubx_block_t *b)
{
	struct manipulator_info *inf = (struct manipulator_info*) b->private_data;
	ubx_debug(b, "%s", __func__);
	int ret = 0;
	return ret;
}

/* stop */
void manipulator_stop(ubx_block_t *b)
{
	struct manipulator_info *inf = (struct manipulator_info*) b->private_data;
	ubx_debug(b, "%s", __func__);
}
#endif

/* cleanup */
void manipulator_cleanup(ubx_block_t *b)
{
	struct manipulator_info *inf = (struct manipulator_info*) b->private_data;
	ubx_debug(b, "%s", __func__);
	free(inf);
}

/* step */
void manipulator_step(ubx_block_t *b)
{
	int ctrl_mode;
	long len;

	struct manipulator_info *inf = (struct manipulator_info*) b->private_data;

	/* check whether a mode switch has been detected */
	len = read_int(inf->ports.ctrl_mode, &ctrl_mode);

	if (len > 0) {
		if (ctrl_mode < 0 || ctrl_mode >= __CTRL_MODE_LAST__) {
			ubx_err(b, "invalid ctrl_mode %i requested - ignoring", ctrl_mode);
			goto cont;

		}
		if (ctrl_mode != inf->ctrl_mode) {
			ubx_info(b, "switching ctrl_mode to %s", ctrl_modes[ctrl_mode]);
			inf->ctrl_mode = ctrl_mode;
		}
	}
cont:
	/* ideal manipulator: cmd is msr */
	read_double_array(inf->ports.jnt_pos_cmd, inf->pos, NUM_JOINTS);
	write_double_array(inf->ports.jnt_pos_msr, inf->pos, NUM_JOINTS);

	read_double_array(inf->ports.jnt_vel_cmd, inf->vel, NUM_JOINTS);
	write_double_array(inf->ports.jnt_vel_msr, inf->vel, NUM_JOINTS);

	read_double_array(inf->ports.jnt_eff_cmd, inf->eff, NUM_JOINTS);
	write_double_array(inf->ports.jnt_eff_msr, inf->eff, NUM_JOINTS);

	read_double_array(inf->ports.jnt_cur_cmd, inf->cur, NUM_JOINTS);
	write_double_array(inf->ports.jnt_cur_msr, inf->cur, NUM_JOINTS);

	return;
}

ubx_proto_block_t manipulator_block = {
	.name = manipulator_block_name,
	.type = BLOCK_TYPE_COMPUTATION,
	.meta_data = manipulator_meta,
	.configs = manipulator_config,
	.ports = manipulator_ports,

	.init = manipulator_init,
	/* uncomment if used */
	/* .start = manipulator_start, */
	/* .stop = manipulator_stop, */
	.cleanup = manipulator_cleanup,
	.step = manipulator_step,
};

int manipulator_mod_init(ubx_node_t* nd)
{
	return ubx_block_register(nd, &manipulator_block);
}

void manipulator_mod_cleanup(ubx_node_t *nd)
{
	ubx_block_unregister(nd, manipulator_block_name);
}

UBX_MODULE_INIT(manipulator_mod_init);
UBX_MODULE_CLEANUP(manipulator_mod_cleanup);
UBX_MODULE_LICENSE_SPDX(BSD-3-Clause);
