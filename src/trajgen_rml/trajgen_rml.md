# RML based joint space trajectory generator (`trajgen_rml`)

*Status* *available*

This block is based on the reflexxes library and provides a simple to
use block that is useful for free space motion in joint space.

**Configs**

| Name     | Type   | Description            |
|----------|--------|------------------------|
| data_len | `long` | *data array length*    |
| max_vel  | `long` | *maximum velocity*     |
| max_acc  | `long` | *maximum acceleration* |


**Ports**

| Name    | Direction | Type   | Description                        |
|---------|-----------|--------|------------------------------------|
| pos_msr | in        | `long` | *measured position*                |
| vel_msr | in        | `long` | *measured velocity*                |
| des_pos | in        | `long` | *desired target position*          |
| des_vel | in        | `long` | *desired target velocity*          |
| pos_cmd | out       | `long` | *commanded position*               |
| vel_cmd | out       | `long` | *commanded velocity*               |
| acc_cmd | out       | `long` | *commanded acceleration*           |
| reached | out       | `int`  | *the final state has been reached* |

