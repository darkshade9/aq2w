/*
 * Copyright(c) 1997-2001 Id Software, Inc.
 * Copyright(c) 2002 The Quakeforge Project.
 * Copyright(c) 2006 Quake2World.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "ui_local.h"

/*
 * Ui_Player
 */
TwBar *Ui_Player(void) {

	TwBar *bar = TwNewBar("Player");

	Ui_CvarText(bar, "Name", name, NULL);
	Ui_CvarText(bar, "Model", skin, NULL);
	Ui_CvarText(bar, "Effects color", color, NULL);

	TwDefine(
			"Player size='300 90' valueswidth=150 alpha=200 iconifiable=false visible=false");

	return bar;
}
