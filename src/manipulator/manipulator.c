/*
 * a robot manipulator skelleton function block
 */
#define UBX_DEBUG

#include <ubx.h>

/* compile time constant */
static const long NUM_JOINTS = 7;

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
	{ .name="ctrl_mode", .type_name = "int", .max=1, .doc="initial ctrl_mode: 0: pos (def), 1: vel, 2: eff, 3: cur" },
	{ 0 },
};

const char manipulator_meta[] = "a empty skelleton manipulator interface block";

ubx_proto_port_t manipulator_ports[] = {
	{ .name="ctrl_mode", .in_type_name="int", .in_data_len=1, .doc="port to switch control mode at runtime" },
	{ .name="pos_msr", .out_type_name="double", .out_data_len=NUM_JOINTS, .doc="measured position [rad]" },
	{ .name="pos_cmd", .in_type_name="double", .in_data_len=NUM_JOINTS, .doc="commanded joint position [rad]" },
	{ .name="vel_msr", .out_type_name="double", .out_data_len=NUM_JOINTS, .doc="measured velocity [m/s]" },
	{ .name="vel_cmd", .in_type_name="double", .in_data_len=NUM_JOINTS, .doc="commanded jnt velocity [rad/s]" },
	{ .name="eff_msr", .out_type_name="double", .out_data_len=NUM_JOINTS, .doc="measured effort [N or Nm]" },
	{ .name="eff_cmd", .in_type_name="double", .in_data_len=NUM_JOINTS, .doc="commanded jnt effort [N or Nm]" },
	{ .name="cur_msr", .out_type_name="double", .out_data_len=NUM_JOINTS, .doc="measured current [A]" },
	{ .name="cur_cmd", .in_type_name="double", .in_data_len=NUM_JOINTS, .doc="commanded jnt current [A]" },
	{ 0 },
};

struct manipulator_info
{
	int ctrl_mode;

	struct port_cache {
		ubx_port_t* ctrl_mode;
		ubx_port_t* pos_msr;
		ubx_port_t* pos_cmd;
		ubx_port_t* vel_msr;
		ubx_port_t* vel_cmd;
		ubx_port_t* eff_msr;
		ubx_port_t* eff_cmd;
		ubx_port_t* cur_msr;
		ubx_port_t* cur_cmd;
	} ports;
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
	assert(len>=0);

	if (len > 0) {
		if(*ctrl_mode < 0 || *ctrl_mode >= __CTRL_MODE_LAST__) {
			ubx_err(b, "invalid ctrl_mode %i", *ctrl_mode);
			ret = -1;
			goto out_free;
		}
		inf->ctrl_mode = *ctrl_mode;
	} else {
		inf->ctrl_mode = 0;
	}

	/* cache ports */
	inf->ports.ctrl_mode = ubx_port_get(b, "ctrl_mode");
	inf->ports.pos_msr = ubx_port_get(b, "pos_msr");
	inf->ports.pos_cmd = ubx_port_get(b, "pos_cmd");
	inf->ports.vel_msr = ubx_port_get(b, "vel_msr");
	inf->ports.vel_cmd = ubx_port_get(b, "vel_cmd");
	inf->ports.eff_msr = ubx_port_get(b, "eff_msr");
	inf->ports.eff_cmd = ubx_port_get(b, "eff_cmd");
	inf->ports.cur_msr = ubx_port_get(b, "cur_msr");
	inf->ports.cur_cmd = ubx_port_get(b, "cur_cmd");

	return 0;

out_free:
	free(inf);
out:
	return ret;
}

/* start */
int manipulator_start(ubx_block_t *b)
{
	/* struct manipulator_info *inf = (struct manipulator_info*) b->private_data; */
        ubx_info(b, "%s", __func__);
	int ret = 0;
	return ret;
}

/* stop */
void manipulator_stop(ubx_block_t *b)
{
	/* struct manipulator_info *inf = (struct manipulator_info*) b->private_data; */
        ubx_info(b, "%s", __func__);
}

/* cleanup */
void manipulator_cleanup(ubx_block_t *b)
{
	/* struct manipulator_info *inf = (struct manipulator_info*) b->private_data; */
        ubx_info(b, "%s", __func__);
	free(b->private_data);
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
			ubx_err(b, "invalid ctrl_mode %i requested", ctrl_mode);
			goto cont;

		}
		if (ctrl_mode != inf->ctrl_mode) {
			ubx_info(b, "switching ctrl_mode to %s", ctrl_modes[ctrl_mode]);
			inf->ctrl_mode = ctrl_mode;
		}
	}
cont:
	/* read data based on ctrl_mode and do something */
	return;
}

ubx_proto_block_t manipulator_block = {
	.name = "manipulator-skel",
	.type = BLOCK_TYPE_COMPUTATION,
	.meta_data = manipulator_meta,
	.configs = manipulator_config,
	.ports = manipulator_ports,

	/* ops */
	.init = manipulator_init,
	.start = manipulator_start,
	.stop = manipulator_stop,
	.cleanup = manipulator_cleanup,
	.step = manipulator_step,
};



/* manipulator module init and cleanup functions */
int manipulator_mod_init(ubx_node_t* nd)
{
	return ubx_block_register(nd, &manipulator_block);
}

void manipulator_mod_cleanup(ubx_node_t *nd)
{
	ubx_block_unregister(nd, "manipulator-skel");
}

UBX_MODULE_INIT(manipulator_mod_init)
UBX_MODULE_CLEANUP(manipulator_mod_cleanup)
UBX_MODULE_LICENSE_SPDX(BSD-3-Clause)
