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

#ifndef __S_SAMPLE_H__
#define __S_SAMPLE_H__

s_sample_t *S_LoadSample(const char *name);
void S_LoadSamples(void);

#ifdef __S_LOCAL_H__

void S_FreeSamples(void);
s_sample_t *S_LoadModelSample(entity_state_t *ent, const char *name);

#endif /* __S_LOCAL_H__ */

#endif /* __S_SAMPLE_H__ */
