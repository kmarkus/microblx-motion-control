# microblx motion control

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Block models](#block-models)
    - [Generic Manipulator (`manipulator-dummy`)](#generic-manipulator-manipulator-dummy)
    - [Controllers](#controllers)
        - [PID (`pid`)](#pid-pid)
        - [Saturation (`saturation`)](#saturation-saturation)
    - [RML based joint space trajectory generator (`trajgen_rml`)](#rml-based-joint-space-trajectory-generator-trajgen_rml)
- [Compositions](#compositions)
    - [`mix_ptrig_nrt`](#mix_ptrig_nrt)
    - [`frag_app_pid_sat`](#frag_app_pid_sat)
    - [`app_jnt_pid_sat`](#app_jnt_pid_sat)
    - [`app_jnt_moveto`](#app_jnt_moveto)
- [References](#references)

<!-- markdown-toc end -->

This repo contains a set of generic, hardware independent microblx
block models and blocks as well as composable usc models. The [Block
models](#block-models) section describes relevant planned and
available blocks. Section [Compositions](#compositions) describes
generic and reusable composition models for building typical motion
control systems using the former blocks.

## Block models

The specified interfaces are the minimum required API. Additional
ports and configurations are of course permitted depending on the
block.


### Generic Manipulator (`manipulator-dummy`)

A **generic manipulator** should conform to the following model. If
one or more of the `ctrl_mode`s are not supported, the respective
pairs of ports can be omitted.

**Status**: skelleton block available

**Configs**

- ctrl_mode [`int`]: *initial control mode to use*
  - `0`: position
  - `1`: velocity
  - `2`: effort
  - `3`: current

**Ports**

- ctrl_mode [in, `int`]: *port to switch control modes at runtime*
- pos_msr [out, `double`]: *measured position [rad]*
- pos_cmd [in, `double`]: *commanded joint position [rad]*
- vel_msr [out, `double`]: *measured velocity [m/s]*
- vel_cmd [in, `double`]: *commanded jnt velocity [rad/s]*
- eff_msr [out, `double`]: *measured effort [N or Nm]*
- eff_cmd [in, `double`]: *commanded jnt effort [N or Nm]*
- cur_msr [out, `double`]: *measured current [A]*
- cur_cmd [in, `double`]: *commanded jnt current [A]*

### Controllers

#### PID (`pid`)

[PID](https://microblx.readthedocs.io/en/latest/block_index.html#module-pid)
is a generic microblx block that can be used for joinspace control.

*Status*: *available*

**Configs**

- data_len [long] length of input signal
- Kp [double]: *P-gain (def: 0)*
- Ki [double]: *I-gain (def: 0)*
- Kd [double]: *D-gain (def: 0)*

**Ports**
- msr [in, `double`]: *measured input signal*
- des [in, `double`]: *desired input signal*
- out [out, `double`]: *controller output*


#### Saturation (`saturation`)

*Status*: *available*

[saturation](https://microblx.readthedocs.io/en/latest/block_index.html#module-saturation-double)
is a generic microblx block that can be used to limit the output of a
controller such as the PID.

**Configs**
- `data_len` [`long`]: *data array length*
- `lower_limits` [`double`]: *saturation lower limits*
- `upper_limits` [`double`]: *saturation upper limits*
 
**Ports**
- `in` [in, `double`]: *input signal to saturate*
- `out` [out, `double`]: *saturated output signal*


### RML based joint space trajectory generator (`trajgen_rml`)

*Status* *available*

This block is based on the [reflexxes motion
library](https://github.com/kschwan/RMLTypeII) and provides a simple
to use block that is useful for free space motion in joint space.

**Configs**
- data_len [`long`]: *data array length*
- max_vel [`long`]: *maximum velocity*
- max_acc [`long`]: *maximum acceleration*

**Ports**
- pos_msr [in, `long`]: *measured position*
- vel_msr [in, `long`]: *measured velocity*
- des_pos [in, `long`]: *desired target position*
- des_vel [in, `long`]: *desired target velocity*
- pos_cmd [out, `long`]: *commanded position*
- vel_cmd [out, `long`]: *commanded velocity*
- acc_cmd [out, `long`]: *commanded acceleration*
- reached [out, `int`]: *the final state has been reached*

## Compositions



The naming scheme is as follows:

- `mix_` compositions are supporting compositions to be *mixed-in* on
  the command line. They are usually not launchable standalone.
- `frag_` compositions are reusable, launchable application fragments
  that can be used to build applications. They may or may not be
  launchable standalone.
- `app_` compositions compose `frag_` compositions to build
  applications. They are launchable standalone.

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

The [frag_jnt_vel](usrc/frag_jnt_vel.usc) composition extends
`frag_pid_sat` with a velocity controlled robot manipulator and a
schedule for executing the composition.

### `app_jnt_vel`

[app_jnt_vel](usc/app_jnt_vel.usc) a small composition based
on `pid_saturated` and the `manipulator-dummy` block.



### `app_jnt_moveto`

[app_jnt_moveto](app_jnt_moveto.usc) a small joint space "move-to"
composition using `trajgen_rml` and the `manipulator-dummy` block. The
desired target `pos` and `vel` ports are exported via mqueues and can
be sent from the command line using `ubx-mq`. See the comment in the
usc file.

**Configuration**

- global: `data_len` defaults to 7 and can be changed based on the
  robot used.


**Example**

Reuse the composition with a 5-DOf manipulator:

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
