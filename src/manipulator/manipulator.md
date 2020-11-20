# Generic Manipulator (`manipulator`) block

A **manipulator** shall conform to the following model.

**Status**: skelleton block available

**Configs**

| Name      | Type  | Description                                                          |
|-----------|-------|----------------------------------------------------------------------|
| ctrl_mode | `int` | *initial control mode to use* (0: pos, 1: vel, 2: effort, 3: current |

**Ports**

| Name      | Direction | Type     | Description                               |
|-----------|-----------|----------|-------------------------------------------|
| ctrl_mode | in        | `int`    | *port to switch control modes at runtime* |
| jnt_pos_msr   | out       | `double` | *measured position [rad]*                 |
| jnt_pos_cmd   | in        | `double` | *commanded joint position [rad]*          |
| jnt_vel_msr   | out       | `double` | *measured velocity [m/s]*                 |
| jnt_vel_cmd   | in        | `double` | *commanded jnt velocity [rad/s]*          |
| jnt_eff_msr   | out       | `double` | *measured effort [N or Nm]*               |
| jnt_eff_cmd   | in        | `double` | *commanded jnt effort [N or Nm]*          |
| jnt_cur_msr   | out       | `double` | *measured current [A]*                    |
| jnt_cur_cmd   | in        | `double` | *commanded jnt current [A]*               |


**Notes** 

- If one or more of the `ctrl_mode`s are not supported, the respective
  pairs of ports can be omitted.
- The specified interfaces are the minimum required API. Additional
  ports and configurations are obviously permitted as required.
