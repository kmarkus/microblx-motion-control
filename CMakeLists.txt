# -*- cmake -*-

CMAKE_MINIMUM_REQUIRED(VERSION 3.14)

project(microblx-motion-control)

find_package(PkgConfig REQUIRED)

# required:
pkg_check_modules(UBX REQUIRED ubx0>=0.9.0)
pkg_get_variable(UBX_MODDIR ubx0 UBX_MODDIR)
find_program(TOCARR ubx-tocarr)

pkg_check_modules(KDL orocos-kdl>=1.4.0)
pkg_check_modules(KDL_PARSER kdl_parser>=1.14.1)
pkg_check_modules(KDL_CONV kdl_conversions)
pkg_check_modules(UBXKDL ubxkdl)

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
find_path(RML_INC RMLTypeII/ReflexxesAPI.h)

if (RML_LIB AND RML_INC)
  message("found libReflexxesTypeII")
  set(LIBRML_FOUND TRUE)
else()
  message("libReflexxesTypeII NOT found, will not build trajgen_rml")
endif()

if ("${LIBRML_FOUND}")
  add_library(trajgen_rml SHARED src/trajgen_rml/trajgen_rml.cpp)
  set_target_properties(trajgen_rml PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE PREFIX "")
  target_compile_options(trajgen_rml PUBLIC
    ${UBX_CFLAGS}
    -I${RML_INC}/RMLTypeII
    -Werror -Wno-missing-field-initializers -fvisibility=hidden)
  target_link_libraries(trajgen_rml ${UBX_LDFLAGS} ${RML_LIB})
  install(TARGETS trajgen_rml DESTINATION ${UBX_MODDIR})
endif()

if (KDL_FOUND AND KDL_PARSER_FOUND AND KDL_CONV_FOUND AND UBXKDL_FOUND)
  message("building fwkin")
  add_library(fwkin SHARED src/fwkin/fwkin.cpp)
  target_link_libraries(fwkin ${UBX_LDFLAGS} ${KDL_LDFLAGS} ${KDL_PARSER_LDFLAGS})
  set_target_properties(fwkin PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE PREFIX "")
  target_compile_options(fwkin PUBLIC
    ${UBX_CFLAGS} ${KDL_CFLAGS}
    -Wno-missing-field-initializers
    -fvisibility=hidden)
  install(TARGETS fwkin DESTINATION ${UBX_MODDIR})
else()
  message("didn't find one or more of KDL, KDL_PARSER, KDL_CONV or UBXKDL")
endif()
