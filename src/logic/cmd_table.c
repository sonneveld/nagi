#include "../agi.h"

#include "../logic/logic_base.h"
#include "../logic/logic_execute.h"
#include "../logic/cmd_table.h"
#include "../logic/arithmetic.h"
#include "../logic/logic_eval.h"

 
// pause, quit 
#include "../base.h" 
// block, unblock, ignoreblocks, observe blocks
#include "../view/obj_block.h"
// loadpic, drawpic, overlay, discard, set.upper.left
#include "../picture/pic_res.h"
// setscript.size   u171  u172
#include "../sys/script.h"
// animate.. unanimateall
#include "../view/obj_base.h"
// ignore observe distanc
#include "../view/obj_proximity.h"
// stop start force update
#include "../view/obj_update.h"
// load view/discard  set view  set cel set loop
#include "../view/view_base.h"
// draw erase
#include "../view/obj_drawerase.h"
// fix release loop
#include "../view/obj_loop.h"
// position + properties
#include "../view/obj_position.h"
// cycles
#include "../view/obj_cycle.h"
// view prop
#include "../view/obj_motion_cmd.h"
// view priorities
#include "../view/obj_priority.h"
// random
#include "../sys/rand.h"
// sound
#include "../sound/sound.h"
#include "../ui/agi_text.h"
#include "../ui/cmd_input.h"
#include "../ui/msg.h"
#include "../ui/parse.h"
#include "../ui/events.h"

// flags
#include "../flags.h"
// new room
#include "../new_room.h"
// get put
#include "../objects.h"

// status
#include "../ui/status.h"

#include "../ui/controller.h"

// getstring, set string
#include "../agi_string.h"
// menus
#include "../ui/menu.h"
// restart
#include "../state_io.h"
// add.to.pic
#include "../picture/pic_add.h"

// show object
#include "../obj_show.h"
#include "../cmd_misc.h"

// tracing!
#include "../trace.h"

u8 cmd_ret_false()
{
	return 0;
}

u8 *cmd_do_nothing(u8 *code)
{
	return(code);
}

FUNC eval_table[] = {
			{"return.false", cmd_ret_false, 0, 0},
			{"equal.n", cmd_equal_n, 2, 0x80},	// fix flags later
			{"equal.v", cmd_equal_v, 2, 0xC0},
			{"less.n", cmd_less_n, 2, 0x80},
			{"less.v", cmd_less_v, 2, 0xC0},
			{"greater.n", cmd_greater_n, 2, 0x80},
			{"greater.v", cmd_greater_v, 2, 0xC0},
			{"isset", cmd_isset, 1, 0},
			{"isset.v", cmd_isset_v, 1, 0x80},
			{"has", cmd_has, 1, 0},
			{"obj.in.room", cmd_obj_in_room, 2, 0x40},
			{"posn", cmd_posn, 5, 0},
			{"controller", cmd_controller, 1, 0},
			{"have.key", cmd_have_key, 0, 0},
			{"said", cmd_said, 0, 0},
			{"compare.strings", cmd_compare_strings, 2, 0},
			{"obj.in.box", cmd_obj_in_box, 5, 0},
			{"center.posn", cmd_center_posn, 5, 0},
			{"right.posn", cmd_right_posn, 5, 0}
	   	};
 
FUNC cmd_table[CMD_MAX + 1] = {
			// 0
			{"cmd.return", cmd_do_nothing, 0, 0},

                        // 1
			{"cmd.increment", cmd_increment, 1, 0x80},
			{"cmd.decrement", cmd_decrement, 1, 0x80},
			{"cmd.assignn", cmd_assignn, 2, 0x80},
			{"cmd.assignv", cmd_assignv, 2, 0xC0},
			{"cmd.addn", cmd_addn, 2, 0x80},
			{"cmd.addv", cmd_addv, 2, 0xC0},
			{"cmd.subn", cmd_subn, 2, 0x80},
			{"cmd.subv", cmd_subv, 2, 0xC0},
			{"cmd.lindirectv", cmd_lindirectv, 2, 0xC0},
			{"cmd.rindirect", cmd_rindirect, 2, 0xC0},
			{"cmd.lindirectn", cmd_lindirectn, 2, 0x80},

			// 12
			{"cmd.set", cmd_set, 1, 0},
			{"cmd.reset", cmd_reset, 1, 0},
			{"cmd.toggle", cmd_toggle, 1, 0},
			{"cmd.set.v", cmd_set_v, 1, 0x80},
			{"cmd.reset.v", cmd_reset_v, 1, 0x80},
			{"cmd.toggle.v", cmd_toggle_v, 1, 0x80},

                        // 18
			{"cmd.new.room", cmd_new_room, 1, 0},
			{"cmd.new.room.v", cmd_new_room_v, 1, 0x80},
			
			// 20
			{"cmd.load.logics", cmd_load_logics, 1, 0},
			{"cmd.load.logics.v", cmd_load_logics_v, 1, 0x80},
			{"cmd.call", cmd_call, 1, 0},
			{"cmd.call.v", cmd_call_v, 1, 0x80},
			
			// 24
			{"cmd.load.pic", cmd_load_pic, 1, 0x80},	// actually at 24
			{"cmd.draw.pic", cmd_draw_pic, 1, 0x80},
			{"cmd.show.pic", cmd_show_pic, 0, 0},
			{"cmd.discard.pic", cmd_discard_pic, 1, 0x80},
			{"cmd.overlay.pic", cmd_overlay_pic, 1, 0x80},
			{"cmd.show.priority.screen (incomplete)", cmd_show_pri, 0, 0},
			
			// 30
			{"cmd.load.view", cmd_load_view, 1, 0},
			{"cmd.load.view.v", cmd_load_view_v, 1, 0x80},
			{"cmd.discard.view", cmd_discard_view, 1, 0},
			{"cmd.animate.object", cmd_animate_obj, 1, 0},
			{"cmd.unanimate.all", cmd_unanimate_all, 0, 0},
			{"cmd.draw", cmd_draw, 1, 0},
			{"cmd.erase", cmd_erase, 1, 0},
			
			// 37
			{"cmd.position", cmd_position, 3, 0},
			{"cmd.position.v", cmd_position_v, 3, 0x60},
			{"cmd.get.position", cmd_get_position, 3, 0x60},
			{"cmd.reposition", cmd_reposition, 3, 0x60},
			
			// 41
			{"cmd.set.view", cmd_set_view, 2, 0},
			{"cmd.set.view.v", cmd_set_view_v, 2, 0x40},
			{"cmd.set.loop", cmd_set_loop, 2, 0},
			{"cmd.set.loop.v", cmd_set_loop_v, 2, 0x40},
			{"cmd.fix.loop", cmd_fix_loop, 1, 0},
			{"cmd.release.loop", cmd_release_loop,1, 0},
			{"cmd.set.cel", cmd_set_cel, 2, 0},
			{"cmd.set.cel.v", cmd_set_cel_v, 2, 0x40},
			{"cmd.last.cel", cmd_last_cel, 2, 0x40},
			{"cmd.current.cel", cmd_current_cel, 2, 0x40},
			{"cmd.current.loop", cmd_current_loop, 2, 0x40},
			{"cmd.current.view", cmd_current_view, 2, 0x40},
			{"cmd.num.of.loops", cmd_number_of_loops, 2, 0x40},
			
			// 54
			{"cmd.set.priority", cmd_set_priority, 2, 0},
			{"cmd.set.priority.v", cmd_set_priority_v, 2, 0x40},
			{"cmd.release.priority", cmd_release_priority, 1, 0},
			{"cmd.get.priority", cmd_get_priority, 2, 0x40},

                        // 58
			{"cmd.stop.update", cmd_stop_update, 1, 0},
			{"cmd.start.update", cmd_start_update, 1, 0},
			{"cmd.force.update", cmd_force_update, 1, 0},
			
			//61
			{"cmd.ignore.horizon", cmd_ignore_horizon, 1, 0},
			{"cmd.observe.horizon", cmd_observe_horizon, 1, 0},
			{"cmd.set.horizon", cmd_set_horizon, 1, 0},
			{"cmd.obj.on.water", cmd_obj_on_water, 1, 0},
			{"cmd.obj.on.land", cmd_obj_on_land, 1, 0},
			{"cmd.obj.on.anything", cmd_obj_on_anything, 1, 0},
			
			//67
			{"cmd.ignore.objects", cmd_ignore_objects, 1, 0},
			{"cmd.observe.objects", cmd_observe_objects, 1, 0},
			{"cmd.distance", cmd_distance, 3, 0x20},
			// 70
			{"cmd.stop.cycling", cmd_stop_cycling, 1, 0},
			{"cmd.start.cycling", cmd_start_cycling, 1, 0},
			{"cmd.normal.cycle", cmd_normal_cycle, 1, 0},
			{"cmd.end.of.loop", cmd_end_of_loop, 2, 0},
			{"cmd.reverse.cycle", cmd_reverse_cycle, 1, 0},
			{"cmd.reverse.loop", cmd_reverse_loop, 2, 0},
			{"cmd.cycle.time", cmd_cycle_time, 2, 0x40},
			
                        // 77
			{"cmd.stop.motion", cmd_stop_motion, 1, 0},
			{"cmd.start.motion", cmd_start_motion, 1, 0},
			{"cmd.step.size", cmd_step_size, 2, 0x40},
			{"cmd.step.time", cmd_step_time, 2, 0x40},
			{"cmd.move.obj", cmd_move_obj, 5, 0},
			{"cmd.move.obj.v", cmd_move_obj_v, 5, 0x70},
			{"cmd.follow.ego", cmd_follow_ego, 3, 0},
			{"cmd.wander", cmd_wander, 1, 0},
			{"cmd.normal.motion", cmd_normal_motion, 1, 0},
			{"cmd.set.dir", cmd_set_dir, 2, 0x40},
			{"cmd.get.dir", cmd_get_dir, 2, 0x40},

                        // 88
			{"cmd.ignore.blocks", cmd_ignore_blocks, 1, 0},
			{"cmd.observe.blocks", cmd_observe_blocks, 1, 0},
			{"cmd.block", cmd_block, 4, 0},
			{"cmd.unblock", cmd_unblock, 0, 0},
			
			// 92
			{"cmd.get", cmd_get, 1, 0},
			{"cmd.get.v", cmd_get_v, 1, 0x80},
			{"cmd.drop", cmd_drop, 1, 0},
			{"cmd.put", cmd_put, 2, 0},
			{"cmd.put.v", cmd_put_v, 2, 0x40},
			{"cmd.get.room.v", cmd_get_room_v, 2, 0xC0},
 
                        // 98
			{"cmd.load.sound",cmd_load_sound , 1, 0}, //cmd_load_sound
			{"cmd.sound", cmd_sound, 2, 0},//cmd_sound
			{"cmd.stop.sound",cmd_stop_sound , 0, 0},	//cmd_stop_sound
			{"cmd.print", cmd_print, 1, 0},
			{"cmd.print.v", cmd_print_v, 1, 0x80},
			{"cmd.display", cmd_display, 3, 0},
			{"cmd.display.v", cmd_display_v, 3, 0xE0},
			{"cmd.clear.lines", cmd_clear_lines, 3, 0},
			{"cmd.text.screen", cmd_text_screen, 0, 0},
			{"cmd.graphics", cmd_graphics, 0, 0},

                        // 108
			{"cmd.set.cursor.char", cmd_set_cursor_char, 1, 0},
			{"cmd.set.text.attrib", cmd_set_text_attribute, 2, 0},
			{"cmd.shake.screen (incomplete)", cmd_do_nothing, 1, 0},
			{"cmd.config.screen", cmd_config_screen, 3, 0},
			{"cmd.status.line.on", cmd_status_line_on, 0, 0},
			{"cmd.status.line.off", cmd_status_line_off, 0, 0},
			{"cmd.set.string", cmd_set_string, 2, 0},
			{"cmd.get.string", cmd_get_string, 5, 0},
			{"cmd.word.to.string", cmd_word_to_string, 2, 0},
			{"cmd.parse", cmd_parse, 1, 0},

                        // 118
			{"cmd.get.num", cmd_get_num, 2, 0x40},
			{"cmd.prevent.input", cmd_prevent_input, 0, 0},
			{"cmd.accept.input", cmd_accept_input, 0, 0},
			{"cmd.set.key", cmd_set_key, 3, 0},
			{"cmd.add.to.pic", cmd_add_to_pic, 7, 0},
			{"cmd.add.to.pic.v", cmd_add_to_pic_v, 7, 0xFE},
			{"cmd.status", cmd_status, 0, 0},
			{"cmd.save.game (incomplete)", cmd_save_game, 0, 0},
			{"cmd.restore.game (incomplete)", cmd_restore_game, 0, 0},
			{"cmd.init.disk (incomplete)", cmd_do_nothing, 0, 0},

                        // 128
			{"cmd.restart.game", cmd_restart_game, 0, 0},
			{"cmd.show.obj", cmd_show_obj, 1, 0},
			{"cmd.random", cmd_random, 3, 0x20},
			{"cmd.program.control", cmd_program_control, 0, 0},
			{"cmd.player.control", cmd_player_control, 0, 0},
			{"cmd.obj.status.v (incomplete)", cmd_do_nothing, 1, 0x80},
			{"cmd.quit", cmd_quit, 1, 0},
			{"cmd.show.mem (incomplete)", cmd_do_nothing, 0, 0},
			{"cmd.pause", cmd_pause, 0, 0},
			{"cmd.echo.line", cmd_echo_line, 0, 0},

                        // 138
			{"cmd.cancel.line", cmd_cancel_line, 0, 0},
			{"cmd.init.joy (incomplete)", cmd_do_nothing, 0, 0},
			{"cmd.toggle.monitor", cmd_toggle_monitor, 0, 0},
			{"cmd.version", cmd_version, 0, 0},
			{"cmd.script.size", cmd_script_size, 1, 0},
			{"cmd.set.game.id", cmd_set_game_id, 1, 0},
			{"cmd.log (incomplete)", cmd_do_nothing, 1, 0},
			{"cmd.set.scan.start", cmd_set_scan_start, 0, 0},
			{"cmd.reset.scan.start", cmd_reset_scan_start, 0, 0},
			
			// 147
			{"cmd.reposition.to", cmd_reposition_to, 3, 0},
			{"cmd.reposition.to.v", cmd_reposition_to_v, 3, 0x60},
			
			// 149
			{"cmd.trace.on", cmd_trace_on, 0, 0},
			{"cmd.trace.info", cmd_trace_info, 3, 0},
			{"cmd.print.at", cmd_print_at, 4, 0},
			{"cmd.print.at.v", cmd_print_at_v, 4, 0x80},
			{"cmd.discard.view.v", cmd_discard_view_v, 1, 0x80},
			{"cmd.clear.text.rect", cmd_clear_text_rect, 5, 0},
			{"cmd.set.upper.left", cmd_set_upper_left, 2, 0},
			
			// 156
			{"cmd.set.menu", cmd_set_menu, 1, 0},
			{"cmd.set.menu.item", cmd_set_menu_item, 2, 0},
			{"cmd.submit.menu", cmd_submit_menu, 0, 0},
			{"cmd.enable.item", cmd_enable_item, 1, 0},
			{"cmd.disable.item", cmd_disable_item, 1, 0},
			{"cmd.menu.input", cmd_menu_input, 0, 0},
			
			// 162
			{"cmd.show.obj.v", cmd_show_obj_v, 1, 1},
			{"cmd.open.dialogue", cmd_open_dialogue, 0, 0},
			{"cmd.close.dialogue", cmd_close_dialogue, 0, 0},

                        // 165
			{"cmd.mult.n", cmd_multn, 2, 0x80},
			{"cmd.mult.v", cmd_multv, 2, 0xC0},
			{"cmd.div.n", cmd_divn, 2, 0x80},
			{"cmd.div.v", cmd_divv, 2, 0xC0},

                        // 169
			{"cmd.close.window", cmd_close_window, 0, 0},
			
			// 170
			{"cmd.set.simple (incomplete)", cmd_do_nothing, 1, 0},
			{"cmd.push.script", cmd_unknown_171, 0, 0},
			{"cmd.pop.script", cmd_unknown_172, 0, 0},
			{"cmd.hold.key", cmd_unknown_173, 0, 0},
			{"cmd.set.pri.base", cmd_unknown_174, 1, 0},
			{"cmd.discard.sound (incomplete)", cmd_do_nothing, 1, 0},
			{"cmd.hide.mouse (incomplete)", cmd_do_nothing, 0, 0},
			{"cmd.allow.menu", cmd_unknown_177, 1, 0},
			{"cmd.show.mouse (incomplete)", cmd_do_nothing, 0, 0},
			{"cmd.fence.mouse (incomplete)", cmd_do_nothing, 4, 0},
			// 180
			{"cmd.mouse.posn (incomplete)", cmd_do_nothing, 2, 0xC0},
			{"cmd.release.key", cmd_unknown_181, 0, 0},
			{"cmd.adj.ego.move.to.x.y (incomplete)", cmd_do_nothing, 0, 0}
	   	};

