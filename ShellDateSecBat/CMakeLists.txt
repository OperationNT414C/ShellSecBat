cmake_minimum_required(VERSION 2.8)

add_definitions(
  -DENABLE_DATE
)

add_executable(shelldatesecbat
  ${CMAKE_SOURCE_DIR}/main.c
)

target_link_libraries(shelldatesecbat
  taihen_stub
  SceLibKernel_stub
  k
  gcc
  SceCtrl_stub
  SceRegistryMgr_stub
  SceRtc_stub
  ScePower_stub
  SceKernelModulemgr_stub
)

set_target_properties(shelldatesecbat
  PROPERTIES LINK_FLAGS "-nostdlib"
)

vita_create_self(shelldatesecbat.suprx shelldatesecbat
  UNSAFE
  CONFIG ${CMAKE_SOURCE_DIR}/export.yml
)
