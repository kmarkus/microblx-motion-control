/*
 * Reflexxes based trajectory generator block.
 * Copyright (C) 2014-2020 Markus Klotzbuecher <mk@mkio.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#undef UBX_DEBUG
#include <ubx/ubx.h>

#include <RMLTypeII/ReflexxesAPI.h>
#include <RMLTypeII/RMLPositionFlags.h>
#include <RMLTypeII/RMLPositionInputParameters.h>
#include <RMLTypeII/RMLPositionOutputParameters.h>

const char rml_block_name[] = "mc/trajgen_rml";
const char rml_meta[] = "a reflexxes based trajectory generator";

ubx_proto_config_t rml_config[] = {
    { .name = "loglevel", .type_name = "int" },
    { .name = "data_len", .type_name = "long", .doc = "data array length" },
    { .name = "max_vel", .type_name = "double", .doc="maximum velocity" },
    { .name = "max_acc", .type_name = "double", .doc="maximum acceleration" },
    { .name = "cycle_time", .type_name = "double", .min=1, .max=1, .doc="cycle time [s]" },
    { 0 },
};

ubx_proto_port_t rml_ports[] = {
    { .name="pos_msr", .in_type_name="double", .doc="current measured position"  },
    { .name="pos_des", .in_type_name="double", .doc="desired target position"  },
    { .name="vel_msr", .in_type_name="double", .doc="current measured velocity"  },
    { .name="vel_des", .in_type_name="double", .doc="desired target velocity"  },
    { .name="pos_cmd", .out_type_name="double", .doc="new position (controller input)"  },
    { .name="vel_cmd", .out_type_name="double", .doc="new velocity (controller input)"  },
    { .name="acc_cmd", .out_type_name="double", .doc="new acceleration (controller input)"  },
    { .name="reached", .out_type_name="int", .doc="target state reached event"  },
    { 0 },
};

struct rml_ports {
    ubx_port_t* pos_msr;
    ubx_port_t* vel_msr;
    ubx_port_t* pos_des;
    ubx_port_t* vel_des;
    ubx_port_t* pos_cmd;
    ubx_port_t* vel_cmd;
    ubx_port_t* acc_cmd;
    ubx_port_t* reached;
};

void update_port_cache(ubx_block_t *b, struct rml_ports *ports)
{
    ports->pos_msr = ubx_port_get(b, "pos_msr");
    ports->vel_msr = ubx_port_get(b, "vel_msr");
    ports->pos_des = ubx_port_get(b, "pos_des");
    ports->vel_des = ubx_port_get(b, "vel_des");
    ports->pos_cmd = ubx_port_get(b, "pos_cmd");
    ports->vel_cmd = ubx_port_get(b, "vel_cmd");
    ports->acc_cmd = ubx_port_get(b, "acc_cmd");
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

    if (ubx_inport_resize(inf->ports.pos_msr, inf->data_len) ||
        ubx_inport_resize(inf->ports.vel_msr, inf->data_len) ||
        ubx_inport_resize(inf->ports.pos_des, inf->data_len) ||
        ubx_inport_resize(inf->ports.vel_des, inf->data_len) ||
        ubx_outport_resize(inf->ports.pos_cmd, inf->data_len) ||
        ubx_outport_resize(inf->ports.vel_cmd, inf->data_len) ||
        ubx_outport_resize(inf->ports.acc_cmd, inf->data_len) != 0) {
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
    double pos_cmd[inf->data_len];
    double vel_cmd[inf->data_len];
    double acc_cmd[inf->data_len];

    /* new target pos? */
    len = read_double_array(inf->ports.pos_des, tmparr, inf->data_len);

    if(len == inf->data_len) {
        write_int(inf->ports.reached, &reached);

        for (long i=0; i<inf->data_len; i++) {
            inf->IP->TargetPositionVector->VecData[i] = tmparr[i];
            ubx_info(b, "new pos_des[%lu] = %f", i, tmparr[i]);
        }
    }

    /* new target vel? */
    len = read_double_array(inf->ports.vel_des, tmparr, inf->data_len);

    if(len == inf->data_len) {
        write_int(inf->ports.reached, &reached);

        for (long i=0; i<inf->data_len; i++) {
            inf->IP->TargetVelocityVector->VecData[i] = tmparr[i];
            ubx_info(b, "new vel_des[%lu] = %f", i, tmparr[i]);
        }
    }

    /* new measured pos? */
    len = read_double_array(inf->ports.pos_msr, tmparr, inf->data_len);

    if(len == inf->data_len) {
        for (long i=0; i<inf->data_len; i++) {
            inf->IP->CurrentPositionVector->VecData[i] = tmparr[i];
            ubx_debug(b, "pos_msr[%lu] = %f", i, tmparr[i]);
        }
    }

    /* new measured vel? */
    len = read_double_array(inf->ports.vel_msr, tmparr, inf->data_len);

    if(len == inf->data_len) {
        for (long i=0; i<inf->data_len; i++) {
            inf->IP->CurrentVelocityVector->VecData[i] = tmparr[i];
            ubx_debug(b, "vel_msr[%lu] = %f", i, tmparr[i]);
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
        pos_cmd[i] = inf->OP->NewPositionVector->VecData[i];
        ubx_debug(b, "pos_cmd[%lu] = %f", i, pos_cmd[i]);
    }

    for (long i=0; i<inf->data_len; i++) {
        vel_cmd[i] = inf->OP->NewVelocityVector->VecData[i];
        ubx_debug(b, "vel_cmd[%lu] = %f", i, vel_cmd[i]);
    }

    for (long i=0; i<inf->data_len; i++) {
        acc_cmd[i] = inf->OP->NewAccelerationVector->VecData[i];
        ubx_debug(b, "acc_cmd[%lu] = %f", i, acc_cmd[i]);
    }

    write_double_array(inf->ports.pos_cmd, pos_cmd, inf->data_len);
    write_double_array(inf->ports.vel_cmd, vel_cmd, inf->data_len);
    write_double_array(inf->ports.acc_cmd, acc_cmd, inf->data_len);
}

ubx_proto_block_t rml_block = {
    .name = rml_block_name,
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
    ubx_block_unregister(ni, rml_block_name);
}

UBX_MODULE_LICENSE_SPDX(BSD-3-Clause)
UBX_MODULE_INIT(rml_mod_init)
UBX_MODULE_CLEANUP(rml_mod_cleanup)
