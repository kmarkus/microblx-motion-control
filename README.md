# microblx motion control

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Block models](#block-models)
    - [Generic Manipulator (`manipulator`)](#generic-manipulator-manipulator)
    - [Controllers](#controllers)
        - [PID (`pid`)](#pid-pid)
        - [Saturation (`saturation`)](#saturation-saturation)
    - [RML based joint space trajectory generator (`trajgen_rml`)](#rml-based-joint-space-trajectory-generator-trajgen_rml)
- [Compositions](#compositions)
    - [`mix_ptrig_nrt`](#mix_ptrig_nrt)
    - [`frag_pid_sat`](#frag_pid_sat)
    - [`frag_jnt_vel`](#frag_jnt_vel)
    - [`app_jnt_vel`](#app_jnt_vel)
    - [`app_jnt_moveto`](#app_jnt_moveto)
- [References](#references)

<!-- markdown-toc end -->

This repo contains a set of generic, hardware independent microblx
block models and blocks as well as composable usc models. The [Block
models](#block-models) section describes relevant planned and
available blocks. Section [Compositions](#compositions) describes
generic and reusable composition models for building typical motion
control systems using the former blocks.

## Installation

Dependencies are 

- microblx v0.9 or newer
- for the `trajgen_rml` block the reflexxes library is required. It
  was tested with [this](https://github.com/kschwan/RMLTypeII)
  version.

## Blocks

| Name        | Description                             | Status              | Where                                                                                           |
|-------------|-----------------------------------------|---------------------|-------------------------------------------------------------------------------------------------|
| manipulator | a robotic manipulator dummy block       | skelleton available | [this repo](src/manipulator/manipulator.md)                                                     |
| pid         | pid controller block                    | available           | [microblx](https://microblx.readthedocs.io/en/latest/block_index.html#module-pid)               |
| saturation  | generic saturation block                | available           | [microblx](https://microblx.readthedocs.io/en/latest/block_index.html#module-saturation-double) |
| trajgen_rml | libreflexxes based trajectory generator | available           | [this repo](src/trajgen_rml/trajgen_rml.md)                                                     |


## Compositions

This sections describes a number of reusable usc compositions. The
follwing naming scheme is used:

- `mix_` compositions are intended to be *mixed-in* on the command
  line as additional arguments to `-c`. They are usually not
  launchable standalone.
- `frag_` compositions are reusable application fragments that can be
  used as building blocks in applications. They may or may not be
  launchable standalone.
- `app_` compositions compose `frag_` compositions to build
  applications. They are launchable standalone.

Note that these rules are not enforced in any way.

### `mix_ptrig_nrt`

[mix_ptrig_nrt](usc/mix_ptrig_nrt.usc) is a mixin model for late addition
of a pthread trigger to avoid cluttering the application with platform
specifics.

### `frag_pid_sat`

[frag_pid_sat](usc/frag_pid_sat.usc) is a small composition of a PID
controller and a saturation block with the purpose of constraining the
PID's output to safe values.

**Configuration**

- global: `data_len`: default 1, must be set to the required data
  dimension
  
- `sat`
   - `lower_limits`: saturation lower limits, array of `data_len`
   - `upper_limits`: saturation upper limits, array of `data_len`

### `frag_jnt_vel`

The [frag_jnt_vel](usc/frag_jnt_vel.usc) composition is based on
`frag_pid_sat` and adds a velocity controlled robot manipulator and a
schedule for executing the composition.

**Configuration**

- same as `frag_pid_sat`


### `app_jnt_vel`

[app_jnt_vel](usc/app_jnt_vel.usc) customizes `frag_jnt_vel` (e.g. the
saturation limits) and adds a manipulator (`manipulator`) block.


**Usage**

This composition can be launched as follows:

```bash
$ ubx-launch -webif -v -c app_jnt_vel.usc,mix_ptrig_nrt.usc
...
```

Note: the `ubx-log` output will repeatedly show the following error
messgage `pid ERROR: ENODATA: no data on port des`. This is because
the PID expects a desired value on it's `des` port every
cycle. As this port is exported via a message queue:

```bash
$ ubx-mq list
   mq id    type name  array len  type hash
1  sat.out  double     7          e8cd7da078a86726031ad64f35f5a6c0
2  pid.des  double     7          e8cd7da078a86726031ad64f35f5a6c0
```

a value can be sent (`-r 0.1` sends the value at 10Hz, which is the
configured ptrig period):

```
$ ubx-mq write pid.des '{0,0.1,0.1,0.1,0.1,0}'  -r 0.1
```

With this, the error messages will stop.

### `app_jnt_moveto`

[app_jnt_moveto](usc/app_jnt_moveto.usc) a small joint space "move-to"
composition using `trajgen_rml` and the `manipulator` block. The
desired target `pos` and `vel` ports are exported via mqueues and can
be sent from the command line using `ubx-mq`.

**Configuration**

- global: `data_len` defaults to 7 and can be changed based on the
  robot used.
  
**Usage**

Lauching the composition:

```bash
$ ubx-launch -v -c app_jnt_moveto.usc,ptrig_nrt.usc
...
```

Show the current output of the `trajgen` block

```bash
$ ubx-mq read trajgen.vel_cmd
{0,0,0,0,0,0,0}
{0,0,0,0,0,0,0}
...
```

Send a velocity or position setpoint:

```bash
$ ubx-mq write trajgen.pos_des '{1,1,1,1,1,1,1 }' 
```

... and `trajgen.vel_cmd` values should start changing.


**Customization Example**

To reuse the `app_jnt_moveto` composition with a 5-DOF manipulator:

```Lua
return bd.system {
   -- set the array dimension to 5
   node_configurations = {
      data_len = { type = "long", config = 5 },
   },

   -- merge the composition
   subsystems = {
      bd.load("app_jnt_moveto.usc")
   },

   -- override the arm block
   blocks = {
      { name = "arm", type="mc/robotXY" },
   }
   ...
}
```


References
----------

- https://github.com/ros-controls/ros_control
- http://wiki.ros.org/ros_control/Ideas
