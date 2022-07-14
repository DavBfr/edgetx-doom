
set(SRC ${SRC}
  doom/doomdef.c doom/hu_lib.c doom/i_timer.c doom/m_fixed.c doom/p_lights.c doom/p_switch.c doom/r_sky.c doom/w_checksum.c
  doom/am_map.c doom/hu_stuff.c doom/i_video.c doom/m_menu.c doom/p_map.c doom/p_telept.c doom/r_things.c doom/w_file.c
  doom/i_cdmus.c doom/icon.c doom/m_misc.c doom/p_maputl.c doom/p_tick.c doom/s_sound.c doom/w_main.c
  doom/d_event.c doom/doomstat.c doom/i_endoom.c doom/include/ doom/m_random.c doom/p_mobj.c doom/p_user.c doom/sha1.c doom/w_wad.c
  doom/d_items.c doom/dstrings.c doom/i_input.c doom/info.c doom/memio.c doom/p_plats.c doom/r_bsp.c doom/sounds.c doom/wi_stuff.c
  doom/d_iwad.c doom/dummy.c doom/i_joystick.c doom/m_argv.c doom/p_ceilng.c doom/p_pspr.c doom/r_data.c doom/st_lib.c doom/z_zone.c
  doom/d_loop.c doom/f_finale.c doom/i_main.c doom/m_bbox.c doom/p_doors.c doom/p_saveg.c doom/r_draw.c doom/st_stuff.c
  doom/d_main.c doom/f_wipe.c doom/i_scale.c doom/m_cheat.c doom/p_enemy.c doom/p_setup.c doom/r_main.c doom/statdump.c
  doom/d_mode.c doom/g_game.c doom/i_sound.c doom/m_config.c doom/p_floor.c doom/p_sight.c doom/r_plane.c doom/tables.c
  doom/d_net.c doom/gusconf.c doom/i_system.c doom/m_controls.c doom/p_inter.c doom/p_spec.c doom/r_segs.c doom/v_video.c doom/edgetx/display.cpp doom/edgetx/doomgeneric.cpp
)

message(STATUS "Build DOOM firmware")

include_directories(doom/include doom/edgetx)
add_compile_definitions(WITH_DOOM)
