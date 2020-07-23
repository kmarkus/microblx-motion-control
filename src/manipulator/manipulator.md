# Generic Manipulator (`manipulator`) block

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


The specified interfaces are the minimum required API. Additional
ports and configurations are obviously permitted as required.
