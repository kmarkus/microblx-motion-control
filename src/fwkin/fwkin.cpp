/*
 * a simple KDL based forward kinematics block
 *
 * Copyright (C) 2020 Markus Klotzbuecher <mk@mkio.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#undef UBX_DEBUG

#include <ubx/ubx.h>
#include <ubxkdl.hpp>

#include <kdl_parser/kdl_parser.hpp>
#include <kdl/chainfksolverpos_recursive.hpp>

#define FWKIN_BLOCK_NAME "fwkin"

// block meta information
char fwkin_meta[] =
    "{ doc='A KDL based forward kinematics block', realtime=false }";

ubx_proto_config_t fwkin_config[] = {
    { .name="urdf_file", .type_name = "char", .min=1, .doc="robot urdf model" },
    { .name="base_link", .type_name = "char", .min=1, .doc="base link of the chain" },
    { .name="tip_link", .type_name = "char", .min=1, .doc="tip link of the chain" },    
    { 0 },
};

ubx_proto_port_t fwkin_ports[] = {
    { .name="jnt_pos", .in_type_name="double", .in_data_len=1, .doc="joint position" },
    { .name="pose", .out_type_name="struct kdl_frame", .out_data_len=1, .doc="pose" },
    { 0 },
};

struct fwkin_info
{
    KDL::Chain chain;
    long num_joints;

    std::unique_ptr<KDL::ChainFkSolverPos_recursive> fkrec;
    std::shared_ptr<KDL::JntArray> jntarr;

    KDL::Frame frame;

    struct port_cache {
        ubx_port_t* jnt_pos;
        ubx_port_t* pose;
    } ports;
};

int fwkin_init(ubx_block_t *b)
{
    long len;
    const char *urdf_file, *base_link, *tip_link;
    struct fwkin_info *inf;
    KDL::Tree tree;

    try {
        inf = new fwkin_info();
    }
    catch (const std::bad_alloc& e) {
        ubx_err(b, "ubxros: failed to alloc block state");
        return EOUTOFMEM;
    }

    b->private_data = inf;

    /* urdf_file */
    len = cfg_getptr_char(b, "urdf_file", &urdf_file);
    assert(len>0);

    /* base_link */
    len = cfg_getptr_char(b, "base_link", &base_link);
    assert(len>0);

    /* tip_link */        
    len = cfg_getptr_char(b, "tip_link", &tip_link);
    assert(len>0);

    if (!kdl_parser::treeFromFile(urdf_file, tree)){
        ubx_err(b, "failed to construct kdl tree from file %s", urdf_file);
        return -1;
    }

    /* extract chain */
    if (!tree.getChain(base_link, tip_link, inf->chain)) {
        ubx_err(b, "failed to extract chain %s -> %s", base_link, tip_link);
        return -1;
    }

    inf->num_joints = inf->chain.getNrOfJoints();
    ubx_info(b, "found chain %s->%s with %lu joints",
             base_link, tip_link, inf->num_joints);

    /* create jnt array and  solver */
    inf->fkrec.reset(new KDL::ChainFkSolverPos_recursive(inf->chain));
    inf->jntarr.reset(new KDL::JntArray(inf->num_joints));

    /* cache ports */
    inf->ports.jnt_pos = ubx_port_get(b, "jnt_pos");
    inf->ports.pose = ubx_port_get(b, "pose");

    /* resize ports */
    if (ubx_inport_resize(inf->ports.jnt_pos, inf->num_joints)) {
        ubx_err(b, "failed to resize ports to jointsize %lu", inf->num_joints);
        return -1;
    }

    return 0;
}

/* start */
int fwkin_start(ubx_block_t *b)
{
    /* struct fwkin_info *inf = (struct fwkin_info*) b->private_data; */
    ubx_info(b, "%s", __func__);
    int ret = 0;
    return ret;
}

/* stop */
void fwkin_stop(ubx_block_t *b)
{
    /* struct fwkin_info *inf = (struct fwkin_info*) b->private_data; */
    ubx_info(b, "%s", __func__);
}

/* cleanup */
void fwkin_cleanup(ubx_block_t *b)
{
    struct fwkin_info *inf = (struct fwkin_info*) b->private_data;
    delete(inf);
    b->private_data = NULL;
}

void fwkin_step(ubx_block_t *b)
{
    long len;
    struct fwkin_info *inf = (struct fwkin_info*) b->private_data;

    double jnt_pos[inf->num_joints];

    len = read_double_array(inf->ports.jnt_pos, jnt_pos, inf->num_joints);

    if (len == 0) {
        ubx_err(b, "ENODATA: on port jnt_pos");
        return;
    }

    if (len != inf->num_joints) {
        ubx_err(b, "EINVALID_DATA_LEN: jnt_pos, is %lu, expected %lu", len, inf->num_joints);
        return;
    }

    for (int i=0; i<inf->num_joints; i++)
        inf->jntarr->data[i] = jnt_pos[i];

    if (inf->fkrec->JntToCart(*inf->jntarr.get(), inf->frame) < 0) {
        ubx_err(b, "FK calculation failed");
        return;
    }

    portWrite(inf->ports.pose, &inf->frame);
    return;
}

ubx_proto_block_t fwkin_block = {
    .name = FWKIN_BLOCK_NAME,
    .meta_data = fwkin_meta,
    .type = BLOCK_TYPE_COMPUTATION,
    .configs = fwkin_config,
    .ports = fwkin_ports,

    .init = fwkin_init,
    .start = fwkin_start,
    .stop = fwkin_stop,
    .cleanup = fwkin_cleanup,
    .step = fwkin_step,
};

int fwkin_mod_init(ubx_node_t* nd)
{
    return ubx_block_register(nd, &fwkin_block);
}

void fwkin_mod_cleanup(ubx_node_t *nd)
{
    ubx_block_unregister(nd, FWKIN_BLOCK_NAME);
}

UBX_MODULE_INIT(fwkin_mod_init);
UBX_MODULE_CLEANUP(fwkin_mod_cleanup);
UBX_MODULE_LICENSE_SPDX(BSD-3-Clause);
