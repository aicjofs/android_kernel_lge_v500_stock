/* pv951.h - Keytable for pv951 Remote Controller
 *
 * keymap imported from ir-keymaps.c
 *
 * Copyright (c) 2010 by Mauro Carvalho Chehab <mchehab@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <media/rc-map.h>
#include <linux/module.h>

/*                             */

static struct rc_map_table pv951[] = {
	{ 0x00, KEY_0 },
	{ 0x01, KEY_1 },
	{ 0x02, KEY_2 },
	{ 0x03, KEY_3 },
	{ 0x04, KEY_4 },
	{ 0x05, KEY_5 },
	{ 0x06, KEY_6 },
	{ 0x07, KEY_7 },
	{ 0x08, KEY_8 },
	{ 0x09, KEY_9 },

	{ 0x12, KEY_POWER },
	{ 0x10, KEY_MUTE },
	{ 0x1f, KEY_VOLUMEDOWN },
	{ 0x1b, KEY_VOLUMEUP },
	{ 0x1a, KEY_CHANNELUP },
	{ 0x1e, KEY_CHANNELDOWN },
	{ 0x0e, KEY_PAGEUP },
	{ 0x1d, KEY_PAGEDOWN },
	{ 0x13, KEY_SOUND },

	{ 0x18, KEY_KPPLUSMINUS },	/*        */
	{ 0x16, KEY_SUBTITLE },		/*    */
	{ 0x0d, KEY_TEXT },		/*     */
	{ 0x0b, KEY_TV },		/*         */
	{ 0x11, KEY_PC },		/*       */
	{ 0x17, KEY_OK },		/*        */
	{ 0x19, KEY_MODE },		/*      */
	{ 0x0c, KEY_SEARCH },		/*          */

	/*                                      */
	{ 0x0f, KEY_VIDEO },		/*        */
	{ 0x0a, KEY_KPPLUS },		/*      */
	{ 0x14, KEY_EQUAL },		/*      */
	{ 0x1c, KEY_TV },		/*       */
};

static struct rc_map_list pv951_map = {
	.map = {
		.scan    = pv951,
		.size    = ARRAY_SIZE(pv951),
		.rc_type = RC_TYPE_UNKNOWN,	/*                */
		.name    = RC_MAP_PV951,
	}
};

static int __init init_rc_map_pv951(void)
{
	return rc_map_register(&pv951_map);
}

static void __exit exit_rc_map_pv951(void)
{
	rc_map_unregister(&pv951_map);
}

module_init(init_rc_map_pv951)
module_exit(exit_rc_map_pv951)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mauro Carvalho Chehab <mchehab@redhat.com>");
