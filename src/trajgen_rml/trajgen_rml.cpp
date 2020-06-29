/*
 * Reflexxes based trajectory generator block.
 */

#define UBX_DEBUG
#include <ubx/ubx.h>

#include <RMLTypeII/ReflexxesAPI.h>
#include <RMLTypeII/RMLPositionFlags.h>
#include <RMLTypeII/RMLPositionInputParameters.h>
#include <RMLTypeII/RMLPositionOutputParameters.h>

char rml_meta[] = "a reflexxes based trajectory generator";

ubx_proto_config_t rml_config[] = {
    { .name = "data_len", .type_name = "long", .doc = "data array length" },
    { .name = "max_vel", .type_name = "double", .doc="maximum velocity" },
    { .name = "max_acc", .type_name = "double", .doc="maximum acceleration" },
    { .name = "cycle_time", .type_name = "double", .min=1, .max=1, .doc="cycle time [s]" },
    { 0 },
};

ubx_proto_port_t rml_ports[] = {
    { .name="msr_pos", .in_type_name="double", .doc="current measured position"  },
    { .name="msr_vel", .in_type_name="double", .doc="current measured velocity"  },
    { .name="des_pos", .in_type_name="double", .doc="desired target position"  },
    { .name="des_vel", .in_type_name="double", .doc="desired target velocity"  },
    { .name="cmd_pos", .out_type_name="double", .doc="new position (controller input)"  },
    { .name="cmd_vel", .out_type_name="double", .doc="new velocity (controller input)"  },
    { .name="cmd_acc", .out_type_name="double", .doc="new acceleration (controller input)"  },
    { .name="reached", .out_type_name="int", .doc="the final state has been reached"  },
    { 0 },
};

struct rml_ports {
    ubx_port_t* msr_pos;
    ubx_port_t* msr_vel;
    ubx_port_t* des_pos;
    ubx_port_t* des_vel;
    ubx_port_t* cmd_pos;
    ubx_port_t* cmd_vel;
    ubx_port_t* cmd_acc;
    ubx_port_t* reached;
};

void update_port_cache(ubx_block_t *b, struct rml_ports *ports)
{
    ports->msr_pos = ubx_port_get(b, "msr_pos");
    ports->msr_vel = ubx_port_get(b, "msr_vel");
    ports->des_pos = ubx_port_get(b, "des_pos");
    ports->des_vel = ubx_port_get(b, "des_vel");
    ports->cmd_pos = ubx_port_get(b, "cmd_pos");
    ports->cmd_vel = ubx_port_get(b, "cmd_vel");
    ports->cmd_acc = ubx_port_get(b, "cmd_acc");
    ports->reached = ubx_port_get(b, "reached");
}

struct rml_info
{
    long data_len;

    ReflexxesAPI *RML;
    RMLPositionInputParameters *IP;
    RMLPositionOutputParameters *OP;
    RMLPositionFlags Flags;

    struct rml_ports ports;
};

int rml_init(ubx_block_t *b)
{
    long len;
    struct rml_info *inf;
    const double *cycle_time;
    const long *tlong;

    b->private_data = calloc(1, sizeof(struct rml_info));

    if (b->private_data == NULL) {
        ubx_err(b, "failed to alloc rml_info");
        return EOUTOFMEM;
    }

    inf = (struct rml_info*) b->private_data;

    /* data_len, default to 1 */
    len = cfg_getptr_long(b, "data_len", &tlong);
    assert(len >= 0);
    inf->data_len = (len > 0) ? *tlong : 1;

    /* cycle_time */
    len = cfg_getptr_double(b, "cycle_time", &cycle_time);
    assert(len>0);

    /* cache and resize ports */
    update_port_cache(b, &inf->ports);

    if (ubx_inport_resize(inf->ports.msr_pos, inf->data_len) ||
        ubx_inport_resize(inf->ports.msr_vel, inf->data_len) ||
        ubx_inport_resize(inf->ports.des_pos, inf->data_len) ||
        ubx_inport_resize(inf->ports.des_vel, inf->data_len) ||
        ubx_outport_resize(inf->ports.cmd_pos, inf->data_len) ||
        ubx_outport_resize(inf->ports.cmd_vel, inf->data_len) ||
        ubx_outport_resize(inf->ports.cmd_acc, inf->data_len) != 0) {
        return -1;
    }

    inf->RML = new ReflexxesAPI(inf->data_len, *cycle_time);
    inf->IP = new RMLPositionInputParameters(inf->data_len);
    inf->OP = new RMLPositionOutputParameters(inf->data_len);
    inf->Flags.SynchronizationBehavior = RMLPositionFlags::ONLY_TIME_SYNCHRONIZATION;

    return 0;
}

int rml_start(ubx_block_t *b)
{
    long len;
    const double *max_vel, *max_acc;

    struct rml_info *inf = (struct rml_info*) b->private_data;

    /* max_vel */
    len = cfg_getptr_double(b, "max_vel", &max_vel);
    assert(len >= 0);

    if (len != inf->data_len) {
        ubx_err(b, "EINVALID_CONFIG_LEN: max_vel is %lu but data_len is %lu", len, inf->data_len);
        return -1;
    }

    for (long i=0; i<inf->data_len; i++) {
        inf->IP->MaxVelocityVector->VecData[i] = max_vel[i];
        ubx_debug(b, "max_vel[%lu] = %f", i, max_vel[i]);
    }

    /* max_acc */
    len = cfg_getptr_double(b, "max_acc", &max_acc);
    assert(len >= 0);

    if (len != inf->data_len) {
        ubx_err(b, "EINVALID_CONFIG_LEN: max_acc is %lu but data_len is %lu", len, inf->data_len);
        return -1;
    }

    for (long i=0; i<inf->data_len; i++) {
        inf->IP->MaxAccelerationVector->VecData[i] = max_acc[i];
        ubx_debug(b, "max_acc[%lu] = %f", i, max_acc[i]);
    }

    /* initialize max_jerk and current pos */
    for (long i=0; i<inf->data_len; i++) {
        inf->IP->CurrentPositionVector->VecData[i] = 0;
        inf->IP->CurrentVelocityVector->VecData[i] = 0;
        inf->IP->CurrentAccelerationVector->VecData[i] = 0;

        /* not support in LGPL version */
        inf->IP->MaxJerkVector->VecData[i] = 100.0;
    }

    return 0;
}

void rml_cleanup(ubx_block_t *b)
{
    struct rml_info *inf;
    inf = (struct rml_info*) b->private_data;

    delete inf->RML;
    delete inf->IP;
    delete inf->OP;

    free(inf);
}

/* step */
void rml_step(ubx_block_t *b)
{
    long len;
    int res, reached = 0;

    struct rml_info *inf = (struct rml_info*) b->private_data;

    double tmparr[inf->data_len];
    double cmd_pos[inf->data_len];
    double cmd_vel[inf->data_len];
    double cmd_acc[inf->data_len];

    /* new target pos? */
    len = read_double_array(inf->ports.des_pos, tmparr, inf->data_len);

    if(len == inf->data_len) {
        write_int(inf->ports.reached, &reached);

        for (long i=0; i<inf->data_len; i++) {
            inf->IP->TargetPositionVector->VecData[i] = tmparr[i];
            ubx_debug(b, "new des_pos[%lu] = %f", i, tmparr[i]);
        }
    }

    /* new target vel? */
    len = read_double_array(inf->ports.des_vel, tmparr, inf->data_len);

    if(len == inf->data_len) {
        write_int(inf->ports.reached, &reached);

        for (long i=0; i<inf->data_len; i++) {
            inf->IP->TargetVelocityVector->VecData[i] = tmparr[i];
            ubx_debug(b, "new des_vel[%lu] = %f", i, tmparr[i]);
        }
    }

    /* new measured pos? */
    len = read_double_array(inf->ports.msr_pos, tmparr, inf->data_len);

    if(len == inf->data_len) {
        for (long i=0; i<inf->data_len; i++) {
            inf->IP->CurrentPositionVector->VecData[i] = tmparr[i];
            ubx_debug(b, "msr_pos[%lu] = %f", i, tmparr[i]);
        }
    }

    /* new measured vel? */
    len = read_double_array(inf->ports.msr_vel, tmparr, inf->data_len);

    if(len == inf->data_len) {
        for (long i=0; i<inf->data_len; i++) {
            inf->IP->CurrentVelocityVector->VecData[i] = tmparr[i];
            ubx_debug(b, "msr_vel[%lu] = %f", i, tmparr[i]);
        }
    }

    /* select all DOF's */
    for (long i=0; i<inf->data_len; i++)
        inf->IP->SelectionVector->VecData[i] = true;

    ubx_debug(b, "CheckForValidity: %d", inf->IP->CheckForValidity());

    /* update */
    res = inf->RML->RMLPosition(*inf->IP, inf->OP, inf->Flags);

    switch (res) {
    case ReflexxesAPI::RML_WORKING:
        break;
    case ReflexxesAPI::RML_FINAL_STATE_REACHED:
        reached = 1;
        write_int(inf->ports.reached, &reached);
        break;
    case ReflexxesAPI::RML_ERROR_INVALID_INPUT_VALUES:
        ubx_err(b, "RML_ERROR_INVALID_INPUT_VALUES");
        return;
    case ReflexxesAPI::RML_ERROR_EXECUTION_TIME_CALCULATION:
        ubx_err(b, "RML_ERROR_EXECUTION_TIME_CALCULATION");
        return;
    case ReflexxesAPI::RML_ERROR_SYNCHRONIZATION:
        ubx_err(b, "RML_ERROR_SYNCHRONIZATION");
        return;
    case ReflexxesAPI::RML_ERROR_NUMBER_OF_DOFS:
        ubx_err(b, "RML_ERROR_NUMBER_OF_DOFS");
        return;
    case ReflexxesAPI::RML_ERROR_NO_PHASE_SYNCHRONIZATION:
        ubx_err(b, "RML_ERROR_NO_PHASE_SYNCHRONIZATION");
        return;
    case ReflexxesAPI::RML_ERROR_NULL_POINTER:
        ubx_err(b, "RML_ERROR_NULL_POINTER");
        return;
    case ReflexxesAPI::RML_ERROR_EXECUTION_TIME_TOO_BIG:
        ubx_err(b, "RML_ERROR_EXECUTION_TIME_TOO_BIG");
        return;
    default:
        ubx_err(b, "unkown error");
        return;
    }

    for (long i=0; i<inf->data_len; i++) {
        cmd_pos[i] = inf->OP->NewPositionVector->VecData[i];
        ubx_debug(b, "cmd_pos[%lu] = %f", i, cmd_pos[i]);
    }

    for (long i=0; i<inf->data_len; i++) {
        cmd_vel[i] = inf->OP->NewVelocityVector->VecData[i];
        ubx_debug(b, "cmd_vel[%lu] = %f", i, cmd_vel[i]);
    }

    for (long i=0; i<inf->data_len; i++) {
        cmd_acc[i] = inf->OP->NewAccelerationVector->VecData[i];
        ubx_debug(b, "cmd_acc[%lu] = %f", i, cmd_acc[i]);
    }

    write_double_array(inf->ports.cmd_pos, cmd_pos, inf->data_len);
    write_double_array(inf->ports.cmd_vel, cmd_vel, inf->data_len);
    write_double_array(inf->ports.cmd_acc, cmd_acc, inf->data_len);
}


ubx_proto_block_t rml_block = {
    .name = "trajgen_rml",
    .meta_data = rml_meta,
    .type = BLOCK_TYPE_COMPUTATION,
    .configs = rml_config,
    .ports = rml_ports,

    .init = rml_init,
    .start = rml_start,
    .cleanup = rml_cleanup,
    .step = rml_step,
};

int rml_mod_init(ubx_node_t* ni)
{
    return ubx_block_register(ni, &rml_block);
}

void rml_mod_cleanup(ubx_node_t *ni)
{
    ubx_block_unregister(ni, "trajgen_rml");
}

UBX_MODULE_LICENSE_SPDX(BSD-3-Clause)
UBX_MODULE_INIT(rml_mod_init)
UBX_MODULE_CLEANUP(rml_mod_cleanup)
