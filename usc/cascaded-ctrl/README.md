# Cascaded controllers

The following two compositions implement two variants of a position
controller. The first uses the manipulators built-in velocity
controller, whereas the second realizes the position controller as
cascaded position - velocity controller on top of the manipulators
built-in effort controller.

Both compositions use the dummy `manipulator` block and are launchable
(instruction below). For debugging and testing, the position
controllers *desired* port and the controller output is exported using
a message queue.

**Notes:**

- As both compositions only specify their schedule with a passive
`trig` blcok, it is necessary to mix-in an active trigger
(e.g. [mix_ptrig_nrt.usc](../mix_ptrig_nrt.usc)).
- Run `ubx-log` in a separate terminal to see log messages.
- For both compositions, the log will show repeating messages `ERROR:
  ENODATA: no data on port des`. This is because the pid controller is
  not receiving a desired setpoint. After sending setpoints as shown
  below using `ubx-mq`, the message will dissappear.


## `app-posctrl-builtin` ([app-posctrl-builtin.usc](app-posctrl-builtin.usc))

**Launching**

```sh
$ ubx-launch -v -c app-posctrl-builtin.usc,../mix_ptrig_nrt.usc
```

**Send a setpoint**

```sh
$ ubx-mq write posctrl.des '{1,1,1,1,1,1}' -r 0.1
```

**Read the position controller output**

```sh
$ ubx-mq read posctrl.out
```

## `app-posctrl-cascaded` ([app-posctrl-cascaded.usc](app-posctrl-cascaded.usc))

**Launching**

```sh
$ ubx-launch -v -c app-posctrl-cascaded.usc,../mix_ptrig_nrt.usc
```

**Send a setpoint**

```sh
$ ubx-mq write posctrl.des '{1,1,1,1,1,1}' -r 0.1
```

**Read the velocity controller output**

```sh
$ ubx-mq read velctrl.out
```
