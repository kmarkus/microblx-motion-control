# -*- cmake -*-

CMAKE_MINIMUM_REQUIRED(VERSION 3.14)

project(microblx-motion-control)

find_package(PkgConfig REQUIRED)

# required:
pkg_check_modules(UBX REQUIRED ubx0>=0.9.0)
pkg_get_variable(UBX_MODDIR ubx0 UBX_MODDIR)
find_program(TOCARR ubx-tocarr)

# manipulator
add_library(manipulator SHARED src/manipulator/manipulator.c)
set_target_properties(manipulator PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE PREFIX "")

target_compile_options(manipulator PUBLIC
  ${UBX_CFLAGS}
  -Wall -Wextra -Werror
  -Wno-missing-field-initializers
  -fvisibility=hidden
  )

target_link_libraries(manipulator ${UBX_LDFLAGS})
install(TARGETS manipulator DESTINATION ${UBX_MODDIR})
