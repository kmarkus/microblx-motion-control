# multi-level composition of a saturated joint space velocity controller

## `frag_pid_sat`

[frag_pid_sat](frag_pid_sat.usc) is a small composition of a PID
controller and a saturation block with the purpose of constraining the
PID's output to safe values.

**Configuration**

- global: `data_len`: default 1, must be set to the required data
  dimension
  
- `sat`
   - `lower_limits`: saturation lower limits, array of `data_len`
   - `upper_limits`: saturation upper limits, array of `data_len`


## `frag_jnt_vel`

The [frag_jnt_vel](frag_jnt_vel.usc) composition is based on
`frag_pid_sat` and adds a velocity controlled robot manipulator and a
schedule for executing the composition.

**Configuration**

- same as `frag_pid_sat`


## `app_jnt_vel`

[app_jnt_vel](app_jnt_vel.usc) customizes `frag_jnt_vel` (e.g. the
saturation limits) and adds a manipulator (`manipulator`) block.

**Usage**

This composition can be launched as follows:

```bash
$ ubx-launch -webif -v -c app_jnt_vel.usc,../mix_ptrig_nrt.usc
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
$ ubx-mq write pid.des '{0,0.1,0.1,0.1,0.1,0}' -r 0.1
```

With this, the error messages will stop.
