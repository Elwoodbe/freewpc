/*
 * Copyright 2006, 2007, 2008 by Brian Dominy <brian@oddchange.com>
 *
 * This file is part of FreeWPC.
 *
 * FreeWPC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * FreeWPC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with FreeWPC; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/* Spiral Award - 3 second timer is started by either inlane (logic in inlane.c)
 * Lamps are lit at start and turned off after being awarded
 * Use the lamp state to check whether a award has been previously collected */

/* TODO: Fix bug with leff, doesn't show sometimes */
#include <freewpc.h>
#include <eb.h>

U8 spiralaward;
__local__ U8 spiralawards_collected; 

extern __local__ U8 mpf_enable_count;

const char *spiralaward_names[] = {
	"2 MILLION",
	"BATTLE THE POWER",
	"4 MILLION",
	"BATTLE THE POWER",
	"10 MILLION",
	"LIGHT EXTRA BALL"
};

const lampnum_t spiralaward_lamps[] = {
	LM_SPIRAL_2M,
	LM_SPIRAL_3M,
	LM_SPIRAL_4M,
	LM_SPIRAL_5M,
	LM_SPIRAL_10M,
	LM_SPIRAL_EB
};

void start_spiralaward_timer (void)
{	
	if (!multi_ball_play ())
	{
		free_timer_restart (TIM_SPIRALAWARD, TIME_3S);
		leff_start (LEFF_SPIRAL_AWARD_ACTIVE);

	}
}

void flash_spiralaward_lamp (void)
{
	lamp_tristate_flash (spiralaward_lamps[spiralaward]);
	task_sleep_sec (3);
	/* Turn off lamp, award is collected */
	lamp_tristate_off (spiralaward_lamps[spiralaward]);
	task_exit ();
}

void award_spiralaward (void)
{	
	/* Used for bonus */
	spiralawards_collected++;
	
	/* Pick a random award, random_scaled returns N-1 */
	spiralaward = random_scaled (6);
	/* Check to see if it's been previously awarded */
	while (!lamp_test(spiralaward_lamps[spiralaward]))
		spiralaward = random_scaled (6);
	/* Don't award extra ball until the last two */
//	while (spiralaward = 5 && spiralawards_collected < 4)	
//		spiralaward = random_scaled (6);
	
	switch (spiralaward)
	{
		case 0:
			score (SC_2M);
			break;
		case 1:
			mpf_enable_count++;
			break;
		case 2:
			score (SC_4M);
			break;
		case 3:
			mpf_enable_count++;
			break;
		case 4:
			score (SC_10M);
			break;
		case 5:
			sound_send (SND_GET_THE_EXTRA_BALL);
			light_easy_extra_ball ();
			break;
	}
	deff_start (DEFF_SPIRALAWARD_COLLECTED);
	
	/* Run lamp flash as task so it can run in parallel */
	task_recreate_gid (GID_FLASH_SPIRALAWARD_LAMP, flash_spiralaward_lamp);
	/* reset lamps after all 6 have been collected */
	if (spiralawards_collected > 5)
	{	
		/* Wait until lamp flash has finished */
		while (task_find_gid (GID_FLASH_SPIRALAWARD_LAMP))
			task_sleep (TIME_500MS);
		lamplist_apply (LAMPLIST_SPIRAL_AWARDS, lamp_on);
	}
}

void spiralaward_collected_deff (void)
{
	dmd_alloc_low_clean ();
	 (spiralawards_collected < 5)
	{
		font_render_string_center (&font_var5, 64, 20, spiralaward_names[spiralaward]);
		font_render_string_center (&font_fixed6, 64, 5, "SPIRAL AWARD");
	}
	else 
	{
		font_render_string_center (&font_mono5, 64, 5, "SPIRALAWARD COMPLETED");
		sprintf ("20 MILLION");
		font_render_string_center (&font_term6, 64, 15, sprintf_buffer);
		font_render_string_center (&font_term6, 64, 25, spiralaward_names[spiralaward]);
	
	}
	dmd_show_low ();
	task_sleep_sec (2);
	deff_exit ();
}

void spiralaward_right_loop_completed (void)
{
	if (free_timer_test (TIM_SPIRALAWARD))
	{
		sound_send (SND_SLOT_PAYOUT);
		free_timer_stop (TIM_SPIRALAWARD);
		award_spiralaward ();
		//task_sleep (TIME_500MS);
	}
}
/* Cancel if player misses loop */
CALLSET_ENTRY (spiralaward, lamp_update)
{
	if (!free_timer_test (TIM_SPIRALAWARD) && leff_running_p (LEFF_SPIRAL_AWARD_ACTIVE))
	{
		leff_stop (LEFF_SPIRAL_AWARD_ACTIVE);
		task_sleep (TIME_200MS);
	}
}	

CALLSET_ENTRY (spiralaward, start_player)
{
	lamplist_apply (LAMPLIST_SPIRAL_AWARDS, lamp_on);
	spiralawards_collected = 0;
}
