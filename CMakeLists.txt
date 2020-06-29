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
  -Werror
  -Wno-missing-field-initializers
  -fvisibility=hidden)

target_link_libraries(manipulator ${UBX_LDFLAGS})
install(TARGETS manipulator DESTINATION ${UBX_MODDIR})

# trajgen_rml
find_library(RML_LIB NAMES ReflexxesTypeII libReflexxesTypeII)

if ("${RML_LIB_NOTFOUND}" OR "${RML_INC_NOTFOUND}")
  message("libReflexxesTypeII NOT found, will not build trajgen_rml")
else()
  message("found libReflexxesTypeII")
  set(LIBRML_FOUND TRUE)
endif()

if ("${LIBRML_FOUND}")
  add_library(trajgen_rml SHARED src/trajgen_rml/trajgen_rml.cpp)
  set_target_properties(trajgen_rml PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE PREFIX "")
  target_compile_options(trajgen_rml PUBLIC
    ${UBX_CFLAGS}
    -I${RML_INC}
    -Werror -Wno-missing-field-initializers -fvisibility=hidden)
  target_link_libraries(trajgen_rml ${UBX_LDFLAGS} ${RML_LIB})
  install(TARGETS trajgen_rml DESTINATION ${UBX_MODDIR})
endif()
