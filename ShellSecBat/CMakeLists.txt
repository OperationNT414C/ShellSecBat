cmake_minimum_required(VERSION 2.8)

add_executable(shellsecbat
  ${CMAKE_SOURCE_DIR}/main.c
)

target_link_libraries(shellsecbat
  taihen_stub
  SceLibKernel_stub
  k
  gcc
  SceRtc_stub
  ScePower_stub
  SceKernelModulemgr_stub
)

set_target_properties(shellsecbat
  PROPERTIES LINK_FLAGS "-nostdlib"
)

vita_create_self(shellsecbat.suprx shellsecbat
  UNSAFE
  CONFIG ${CMAKE_SOURCE_DIR}/export.yml
)
