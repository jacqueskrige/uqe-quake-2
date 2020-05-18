/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// vid.h -- video driver defs

typedef struct vrect_s
{
	int				x,y,width,height;
} vrect_t;

// jkrige - scale2d
typedef struct
{
	qboolean	scaled;
	float		factor;
} vidscale2d_t;
// jkrige - scale2d

typedef struct
{
	unsigned		width, height;			// coordinates from main game

	// jkrige - scale2d
	vidscale2d_t	scale2d;
	// jkrige - scale2d
} viddef_t;

extern	viddef_t	viddef;				// global video state

// jkrige - scale2d
#define VID_SCALECHAR (8 * viddef.scale2d.factor)
#define VID_SCALECHAR16 (16 * viddef.scale2d.factor)
#define VID_SCALEFACTOR viddef.scale2d.factor
// jkrige - scale2d

// Video module initialisation etc
void	VID_Init (void);
void	VID_Shutdown (void);
void	VID_CheckChanges (void);

void	VID_MenuInit( void );
void	VID_MenuDraw( void );
const char *VID_MenuKey( int );
