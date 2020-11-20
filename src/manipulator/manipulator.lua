return block 
{
      name = "mc/manipulator-dummy",
      license = "BSD-3-Clause",
      meta_data = "a generic, empty dummy manipulator block",

      port_cache = true,

      configurations = {
	 { name = "ctrl_mode", type_name = "int", min=0, max=1, doc="initial ctrl_mode: 0: pos (def), 1: vel, 2: eff, 3: cur" },
      },

      ports = {
	 { name = "ctrl_mode", in_type_name = "int", doc="port to switch control modes at runtime" },
	 { name = "jnt_pos_msr", out_type_name = "double", doc="measured position [rad]" },
	 { name = "jnt_pos_cmd", in_type_name = "double", doc = "commanded joint position [rad]" },
	 { name = "jnt_vel_msr", out_type_name = "double", doc = "measured velocity [m/s]" },
	 { name = "jnt_vel_cmd", in_type_name = "double",  doc = "commanded jnt velocity [rad/s]" },
 	 { name = "jnt_eff_msr", out_type_name = "double", doc = "measured effort [N or Nm]" },
	 { name = "jnt_eff_cmd", in_type_name = "double",  doc = "commanded jnt effort [N or Nm]" },
	 { name = "jnt_cur_msr", out_type_name =  "double", doc="measured current [A]" },
	 { name = "jnt_cur_cmd", in_type_name = "double",  doc="commanded jnt current [A]" },
      },

      operations = { start=true, stop=true, step=true }
}
