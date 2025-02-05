cmake_minimum_required(VERSION 3.19)
project(HDMIProgram)

include (FetchContent)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "nope" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "nope" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "nope" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "nope" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "nope" FORCE)
SET(CMAKE_DEFAULT_STARTUP_PROJECT HDMIProgram)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3.10
)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.77
)

FetchContent_Declare(
  nativefiledialog
  GIT_REPOSITORY https://github.com/mlabbe/nativefiledialog.git
  GIT_TAG        release_116
)

FetchContent_MakeAvailable(glfw imgui nativefiledialog)

add_executable(
				HDMIProgram WIN32
				main.cpp 
				PNPID.cpp 
				PNPID.h 
				Programmer.cpp 
				Programmer.h 
				I2C_FTDI_D2XX.cpp 
				I2C_FTDI_D2XX.h 
				stdafx.cpp 
				stdafx.h 
				icon.rc
				"${FETCHCONTENT_BASE_DIR}/imgui-src/imgui.cpp" 
				"${FETCHCONTENT_BASE_DIR}/imgui-src/imgui_draw.cpp" 
				"${FETCHCONTENT_BASE_DIR}/imgui-src/imgui_widgets.cpp" 
				"${FETCHCONTENT_BASE_DIR}/imgui-src/examples/imgui_impl_glfw.cpp"  
				"${FETCHCONTENT_BASE_DIR}/imgui-src/examples/imgui_impl_opengl2.cpp" 
				"${FETCHCONTENT_BASE_DIR}/imgui-src/imgui.h"  
				"${FETCHCONTENT_BASE_DIR}/imgui-src/imgui_internal.h"
				"${FETCHCONTENT_BASE_DIR}/nativefiledialog-src/src/nfd_common.c"
				"${FETCHCONTENT_BASE_DIR}/nativefiledialog-src/src/nfd_win.cpp"
				"${FETCHCONTENT_BASE_DIR}/nativefiledialog-src/src/include/nfd.h"
				)

target_link_libraries(HDMIProgram PRIVATE glfw)
target_link_libraries(HDMIProgram PUBLIC opengl32)
target_include_directories(HDMIProgram PRIVATE "${FETCHCONTENT_BASE_DIR}/imgui-src")
target_include_directories(HDMIProgram PRIVATE "${FETCHCONTENT_BASE_DIR}/imgui-src/examples/")
target_include_directories(HDMIProgram PRIVATE "${FETCHCONTENT_BASE_DIR}/nativefiledialog-src/src")
target_include_directories(HDMIProgram PRIVATE "${FETCHCONTENT_BASE_DIR}/nativefiledialog-src/src/include")
