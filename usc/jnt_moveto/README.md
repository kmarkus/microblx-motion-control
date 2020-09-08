# `app_jnt_moveto`

[app_jnt_moveto](app_jnt_moveto.usc) is a small joint space "move-to"
composition using `trajgen_rml` and the `manipulator` block. The
desired target `pos` and `vel` ports are exported via mqueues and can
be sent from the command line using `ubx-mq`.

**Configuration**

- global: `data_len` defaults to 7 and can be changed based on the
  robot used.
  
**Usage**

Lauching the composition:

```bash
$ ubx-launch -v -c app_jnt_moveto.usc,../ptrig_nrt.usc
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

Note that `usc` being a declarative DSL, the order of declarations is
irrelevant.
