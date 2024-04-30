add_executable(lab9.elf main.c renderer.c angles.c error.h renderer_fp.h error.h)
target_link_libraries(lab9.elf ${330_LIBS} buttons_switches intervalTimer interrupts touchscreen)
set_target_properties(lab9.elf PROPERTIES LINKER_LANGUAGE CXX)