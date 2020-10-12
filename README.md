# microblx motion control

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Installation](#installation)
- [Block Index](#block-index)
- [Compositions](#compositions)
    - [`mix_ptrig_nrt`](#mix_ptrig_nrt)
    - [`frag_pid_sat`](#frag_pid_sat)
    - [`frag_jnt_vel`](#frag_jnt_vel)
    - [`app_jnt_vel`](#app_jnt_vel)
    - [`app_jnt_moveto`](#app_jnt_moveto)
- [References](#references)

<!-- markdown-toc end -->

This repo contains a set of generic, hardware independent microblx
function blocks and composable usc models. The [Block
index](#block-index) section describes the currently available
blocks. Section [Compositions](#compositions) list generic and
reusable composition models for building typical motion control
systems using the former blocks.

## Installation

**Dependencies**

- microblx v0.9 or newer
- for the `trajgen_rml` block the reflexxes library is required. It
  was tested with [this](https://github.com/kschwan/RMLTypeII)
  version.
- for the kinematics blocks:
  - https://github.com/orocos/orocos_kinematics_dynamics
  - https://github.com/ros/kdl_parser
  - https://github.com/kmarkus/microblx-kdl-types

**Building**

```bash
$ mkdir build
$ cd build
$ cmake ..
 ...
$ make
$ sudo make install
```

## Block Index

| Name        | Description                                 | Status              | Where                                                                                           |
|-------------|---------------------------------------------|---------------------|-------------------------------------------------------------------------------------------------|
| manipulator | a robotic manipulator dummy block           | skelleton available | [this repo](src/manipulator/manipulator.md)                                                     |
| pid         | pid controller block                        | available           | [microblx](https://microblx.readthedocs.io/en/latest/block_index.html#module-pid)               |
| saturation  | generic saturation block                    | available           | [microblx](https://microblx.readthedocs.io/en/latest/block_index.html#module-saturation-double) |
| trajgen_rml | libreflexxes based trajectory generator     | available           | [this repo](src/trajgen_rml/trajgen_rml.md)                                                     |
| fwkin       | KDL based forward position kinematics block | available           | [this repo](src/fwkin)                                                                          |


## Compositions

- [mix_ptrig_nrt](usc/mix_ptrig_nrt.usc) is a generic mixin model for
  late addition of a pthread trigger to avoid cluttering the
  application with platform specifics. It is used by several of the
  following examples.
- [cascaded controllers](usc/cascaded-ctrl/README.md): two variants of
  position control compositions
- [jnt_moveto](usc/jnt_moveto/README.md): free-space trajectory following
- [jnt_vel](usc/jnt_vel/README.md): a multi-level composition of a
  jnt_space velocity controller.

## Tutorial

The 
