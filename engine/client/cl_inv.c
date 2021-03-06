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
// cl_inv.c -- client inventory screen

#include "client.h"

/*
================
CL_ParseInventory
================
*/
void CL_ParseInventory (void)
{
	int		i;

	for (i=0 ; i<MAX_ITEMS ; i++)
		cl.inventory[i] = MSG_ReadShort (&net_message);
}


/*
================
Inv_DrawString
================
*/
void Inv_DrawString (int x, int y, char *string)
{
	while (*string)
	{
		re.DrawChar (x, y, *string);

		// jkrige - scale2d
		x += 8*VID_SCALEFACTOR;
		//x+=8;
		// jkrige - scale2d
		string++;
	}
}

void SetStringHighBit (char *s)
{
	while (*s)
		*s++ |= 128;
}

/*
================
CL_DrawInventory
================
*/
#define	DISPLAY_ITEMS	17

void CL_DrawInventory (void)
{
	int		i, j;
	int		num, selected_num, item;
	int		index[MAX_ITEMS];
	char	string[1024];
	int		x, y;
	char	binding[1024];
	char	*bind;
	int		selected;
	int		top;

	selected = cl.frame.playerstate.stats[STAT_SELECTED_ITEM];

	num = 0;
	selected_num = 0;
	for (i=0 ; i<MAX_ITEMS ; i++)
	{
		if (i==selected)
			selected_num = num;
		if (cl.inventory[i])
		{
			index[num] = i;
			num++;
		}
	}

	// determine scroll point
	top = selected_num - DISPLAY_ITEMS/2;
	if (num - top < DISPLAY_ITEMS)
		top = num - DISPLAY_ITEMS;
	if (top < 0)
		top = 0;

	// jkrige - scale2d
	x = (viddef.width-(256*VID_SCALEFACTOR))/2;
	y = (viddef.height-(240*VID_SCALEFACTOR))/2;
	//x = (viddef.width-256)/2;
	//y = (viddef.height-240)/2;
	// jkrige - scale2d

	// jkrige - dirty removal
	// repaint everything next frame
	//SCR_DirtyScreen ();
	// jkrige - dirty removal


	// jkrige - scale2d
	re.DrawPic (x, y+(8*VID_SCALEFACTOR), "inventory");
	//re.DrawPic (x, y+8, "inventory");

	y += 28*VID_SCALEFACTOR;
	x += 20*VID_SCALEFACTOR;
	Inv_DrawString (x, y, "hotkey ### item");
	Inv_DrawString (x, y+(8*VID_SCALEFACTOR), "------ --- ----");
	y += 16*VID_SCALEFACTOR;
	//y += 24;
	//x += 24;
	//Inv_DrawString (x, y, "hotkey ### item");
	//Inv_DrawString (x, y+8, "------ --- ----");
	//y += 16;
	// jkrige - scale2d

	for (i=top ; i<num && i < top+DISPLAY_ITEMS ; i++)
	{
		item = index[i];
		// search for a binding
		Com_sprintf (binding, sizeof(binding), "use %s", cl.configstrings[CS_ITEMS+item]);
		bind = "";
		for (j=0 ; j<256 ; j++)
			if (keybindings[j] && !Q_stricmp (keybindings[j], binding))
			{
				bind = Key_KeynumToString(j);
				break;
			}

		Com_sprintf (string, sizeof(string), "%6s %3i %s", bind, cl.inventory[item],
			cl.configstrings[CS_ITEMS+item] );
		if (item != selected)
			SetStringHighBit (string);
		else	// draw a blinky cursor by the selected item
		{
			if ( (int)(cls.realtime*10) & 1)
			{
				// jkrige - scale2d
				re.DrawChar (x-(8*VID_SCALEFACTOR), y, 15);
				//re.DrawChar (x-8, y, 15);
				// jkrige - scale2d
			}
		}
		Inv_DrawString (x, y, string);

		// jkrige - scale2d
		y += 8*VID_SCALEFACTOR;
		//y += 8;
		// jkrige - scale2d
	}
}


