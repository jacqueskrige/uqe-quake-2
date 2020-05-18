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
#include "../client/client.h"
#include "../client/qmenu.h"

extern cvar_t *vid_ref;
extern cvar_t *vid_fullscreen;
extern cvar_t *vid_gamma;

// jkrige - viewsize removal
//extern cvar_t *scr_viewsize;
// jkrige - viewsize removal

static cvar_t *gl_mode;
static cvar_t *gl_driver;

// jkrige - texture quality
//static cvar_t *gl_picmip;
// jkrige - texture quality


// jkrige - palette removal
//static cvar_t *gl_ext_palettedtexture;
// jkrige - palette removal

static cvar_t *gl_finish;

extern void M_ForceMenuOff( void );

/*
====================================================================

MENU INTERACTION

====================================================================
*/
static menuframework_s	s_opengl_menu;
static menuframework_s *s_current_menu;

static menulist_s		s_mode_list;

// jkrige - viewsize removal
//static menuslider_s		s_screensize_slider;
// jkrige - viewsize removal

static menuslider_s		s_brightness_slider;
static menulist_s  		s_fs_box;

// jkrige - anisotropic filtering
static menulist_s  		s_texturemode_box;
// jkrige - anisotropic filtering


// jkrige - texture quality
//static menuslider_s		s_tq_slider;
// jkrige - texture quality

// jkrige - palette removal
//static menulist_s  		s_paletted_texture_box;
// jkrige - palette removal

static menulist_s  		s_finish_box;
static menuaction_s		s_defaults_action;
static menuaction_s		s_cancel_action;


// jkrige - anisotropic filtering
static float ClampCvar( float min, float max, float value )
{
	if ( value < min ) return min;
	if ( value > max ) return max;
	return value;
}
// jkrige - anisotropic filtering


static void DriverCallback( void *unused )
{
	s_current_menu = &s_opengl_menu;
}

// jkrige - viewsize removal
/*static void ScreenSizeCallback( void *s )
{
	menuslider_s *slider = ( menuslider_s * ) s;

	Cvar_SetValue( "viewsize", slider->curvalue * 10 );
}*/
// jkrige - viewsize removal

static void BrightnessCallback( void *s )
{
	menuslider_s *slider = ( menuslider_s * ) s;

	//if ( s_current_menu_index == SOFTWARE_MENU )
	//	s_brightness_slider[1].curvalue = s_brightness_slider[0].curvalue;
	//else
	//	s_brightness_slider[0].curvalue = s_brightness_slider[1].curvalue;

	// jkrige - gamma
	/*if ( stricmp( vid_ref->string, "soft" ) == 0 )
	{
		float gamma = ( 0.8 - ( slider->curvalue/10.0 - 0.5 ) ) + 0.5;
		Cvar_SetValue( "vid_gamma", gamma );
	}*/
	if (stricmp(vid_ref->string, "gl") == 0)
	{
		float gamma = (1.7 - (slider->curvalue / 10.0 - 1.0)) + 1.0;
		Cvar_SetValue("vid_gamma", gamma);
	}
	// jkrige - gamma
}

// jkrige - anisotropic filtering
static void TextureModeCallback( void *unused )
{
	s_texturemode_box.curvalue = ClampCvar( 0, 2 + re.GetMaxAnisotropic(), s_texturemode_box.curvalue);
	Cvar_SetValue( "gl_texturemode", s_texturemode_box.curvalue );
}
// jkrige - anisotropic filtering

static void ResetDefaults( void *unused )
{
	VID_MenuInit();
}

static void ApplyChanges( void *unused )
{
	float gamma;

	/*
	** invert sense so greater = brighter, and scale to a range of 0.5 to 1.3
	*/
	// jkrige - gamma
	//gamma = ( 0.8 - ( s_brightness_slider.curvalue / 10.0 - 0.5 ) ) + 0.5;
	gamma = (1.7 - (s_brightness_slider.curvalue / 10.0 - 1.0)) + 1.0;
	// jkrige - gamma

	Cvar_SetValue( "vid_gamma", gamma );
	Cvar_SetValue( "vid_fullscreen", s_fs_box.curvalue );

	// jkrige - texture quality
	//Cvar_SetValue( "gl_picmip", 3 - s_tq_slider.curvalue );
	// jkrige - texture quality

	// jkrige - palette removal
	//Cvar_SetValue( "gl_ext_palettedtexture", s_paletted_texture_box.curvalue );
	// jkrige - palette removal

	Cvar_SetValue( "gl_finish", s_finish_box.curvalue );
	Cvar_SetValue( "gl_mode", s_mode_list.curvalue );

	Cvar_Set( "vid_ref", "gl" );
	Cvar_Set( "gl_driver", "opengl32" );

	/*
	** update appropriate stuff if we're running OpenGL and gamma
	** has been modified
	*/
	if ( stricmp( vid_ref->string, "gl" ) == 0 )
	{
		if ( vid_gamma->modified )
		{
			vid_ref->modified = true;
			/*if ( stricmp( gl_driver->string, "3dfxgl" ) == 0 )
			{
				char envbuffer[1024];
				float g;

				vid_ref->modified = true;

				g = 2.00 * ( 0.8 - ( vid_gamma->value - 0.5 ) ) + 1.0F;
				Com_sprintf( envbuffer, sizeof(envbuffer), "SSTV2_GAMMA=%f", g );
				putenv( envbuffer );
				Com_sprintf( envbuffer, sizeof(envbuffer), "SST_GAMMA=%f", g );
				putenv( envbuffer );

				vid_gamma->modified = false;
			}*/
		}

		if ( gl_driver->modified )
			vid_ref->modified = true;
	}

	M_ForceMenuOff();
}

static void CancelChanges( void *unused )
{
	extern void M_PopMenu( void );

	M_PopMenu();
}

/*
** VID_MenuInit
*/
void VID_MenuInit( void )
{
	int i;

	static const char *resolutions[] = 
	{
		// jkrige - video resolutions
		"[320 240  ]",
		"[640 480  ]",
		"[1024 768 ]",
		"[1280 720 ]",
		"[1366 768 ]",
		"[1600 900 ]",
		"[1440 900 ]",
		"[1680 1050]",
		"[1920 1080]",
		"[3840 2160]",
		0

		/*"[320 240  ]",
		"[640 480  ]",
		"[800 600  ]",
		"[1024 768 ]",
		"[1152 864 ]",
		"[1280 720 ]",
		"[1280 960 ]",
		"[1600 900 ]",
		"[1600 1200]",
		"[1920 1080]",
		0*/
		// jkrige - video resolutions
	};

	static const char *yesno_names[] =
	{
		"no",
		"yes",
		0
	};

	static const char *texturemode_names[] =
	{
		"point",
		"bilinear",
		"trilinear",
		"anisotropic 2x",
		"anisotropic 4x",
		"anisotropic 8x",
		"anisotropic 16x",
		0
	};

	if ( !gl_driver )
		gl_driver = Cvar_Get( "gl_driver", "opengl32", 0 );

	// jkrige - texture quality
	//if ( !gl_picmip )
	//	gl_picmip = Cvar_Get( "gl_picmip", "0", 0 );
	// jkrige - texture quality

	// jkrige - video resolutions
	if ( !gl_mode )
	{
		gl_mode = Cvar_Get( "gl_mode", "2", 0 );
		//gl_mode = Cvar_Get( "gl_mode", "3", 0 );
	}
	// jkrige - video resolutions


	// jkrige - palette removal
	//if ( !gl_ext_palettedtexture )
	//	gl_ext_palettedtexture = Cvar_Get( "gl_ext_palettedtexture", "1", CVAR_ARCHIVE );
	// jkrige - palette removal

	if ( !gl_finish )
		gl_finish = Cvar_Get( "gl_finish", "0", CVAR_ARCHIVE );

	s_mode_list.curvalue = gl_mode->value;

	// jkrige - viewsize removal
	//if ( !scr_viewsize )
	//	scr_viewsize = Cvar_Get ("viewsize", "100", CVAR_ARCHIVE);
	//s_screensize_slider.curvalue = scr_viewsize->value / 10;
	// jkrige - viewsize removal

	s_opengl_menu.x = viddef.width * 0.50;
	s_opengl_menu.nitems = 0;
	

	s_mode_list.generic.type = MTYPE_SPINCONTROL;
	s_mode_list.generic.name = "video mode";
	s_mode_list.generic.x = 0;
	s_mode_list.generic.y = 0;
	s_mode_list.itemnames = resolutions;

	// jkrige - viewsize removal
	/*s_screensize_slider.generic.type	= MTYPE_SLIDER;
	s_screensize_slider.generic.x = 0;
	// jkrige - scale2d
	s_screensize_slider.generic.y = 10*VID_SCALEFACTOR;
	//s_screensize_slider.generic.y = 10;
	// jkrige - scale2d
	s_screensize_slider.generic.name = "screen size";
	s_screensize_slider.minvalue = 3;
	s_screensize_slider.maxvalue = 12;
	s_screensize_slider.generic.callback = ScreenSizeCallback;*/
	// jkrige - viewsize removal

	s_brightness_slider.generic.type	= MTYPE_SLIDER;
	s_brightness_slider.generic.x = 0;
	// jkrige - scale2d
	s_brightness_slider.generic.y = 20*VID_SCALEFACTOR;
	//s_brightness_slider.generic.y = 20;
	// jkrige - scale2d
	s_brightness_slider.generic.name = "brightness";
	s_brightness_slider.generic.callback = BrightnessCallback;
	// jkrige - gamma
	//s_brightness_slider.minvalue = 5;
	//s_brightness_slider.maxvalue = 13;
	//s_brightness_slider.curvalue = (1.3 - vid_gamma->value + 0.5) * 10;
	s_brightness_slider.minvalue = 10;
	s_brightness_slider.maxvalue = 27;
	s_brightness_slider.curvalue = (2.7 - vid_gamma->value + 1.0) * 10;
	// jkrige - gamma


	// jkrige - anisotropic filtering
	s_texturemode_box.generic.type = MTYPE_SPINCONTROL;
	s_texturemode_box.generic.x = 0;
	s_texturemode_box.generic.y = 30*VID_SCALEFACTOR;
	s_texturemode_box.generic.name = "texture mode";
	s_texturemode_box.generic.callback = TextureModeCallback;
	s_texturemode_box.curvalue = Cvar_VariableValue( "gl_texturemode" );
	s_texturemode_box.itemnames = texturemode_names;
	// jkrige - anisotropic filtering


	s_fs_box.generic.type = MTYPE_SPINCONTROL;
	s_fs_box.generic.x = 0;
	// jkrige - scale2d
	s_fs_box.generic.y = 40*VID_SCALEFACTOR;
	//s_fs_box.generic.y = 30;
	// jkrige - scale2d
	s_fs_box.generic.name = "fullscreen";
	s_fs_box.itemnames = yesno_names;
	s_fs_box.curvalue = vid_fullscreen->value;


	// jkrige - texture quality
	/*s_tq_slider.generic.type = MTYPE_SLIDER;
	s_tq_slider.generic.x = 0;
	// jkrige - scale2d
	s_tq_slider.generic.y = 50*VID_SCALEFACTOR;
	//s_tq_slider.generic.y = 50;
	// jkrige - scale2d
	s_tq_slider.generic.name = "texture quality";
	s_tq_slider.minvalue = 0;
	s_tq_slider.maxvalue = 3;
	s_tq_slider.curvalue = 3-gl_picmip->value;*/
	// jkrige - texture quality


	// jkrige - palette removal
	//s_paletted_texture_box.generic.type = MTYPE_SPINCONTROL;
	//s_paletted_texture_box.generic.x = 0;
	//s_paletted_texture_box.generic.y = 60;
	//s_paletted_texture_box.generic.name	= "8-bit textures";
	//s_paletted_texture_box.itemnames = yesno_names;
	//s_paletted_texture_box.curvalue = gl_ext_palettedtexture->value;
	// jkrige - palette removal

	s_finish_box.generic.type = MTYPE_SPINCONTROL;
	s_finish_box.generic.x = 0;
	// jkrige - scale2d
	s_finish_box.generic.y = 50*VID_SCALEFACTOR;
	//s_finish_box.generic.y = 70;
	// jkrige - scale2d
	s_finish_box.generic.name = "sync every frame";
	s_finish_box.curvalue = gl_finish->value;
	s_finish_box.itemnames = yesno_names;

	s_defaults_action.generic.type = MTYPE_ACTION;
	s_defaults_action.generic.name = "reset to defaults";
	s_defaults_action.generic.x = 0;
	// jkrige - scale2d
	s_defaults_action.generic.y = 70*VID_SCALEFACTOR;
	//s_defaults_action.generic.y = 90;
	// jkrige - scale2d
	s_defaults_action.generic.callback = ResetDefaults;

	s_cancel_action.generic.type = MTYPE_ACTION;
	s_cancel_action.generic.name = "cancel";
	s_cancel_action.generic.x = 0;
	// jkrige - scale2d
	s_cancel_action.generic.y = 80*VID_SCALEFACTOR;
	//s_cancel_action.generic.y = 100;
	// jkrige - scale2d
	s_cancel_action.generic.callback = CancelChanges;

	
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_mode_list );

	// jkrige - viewsize removal
	//Menu_AddItem( &s_opengl_menu, ( void * ) &s_screensize_slider );
	// jkrige - viewsize removal

	Menu_AddItem( &s_opengl_menu, ( void * ) &s_brightness_slider );


	// jkrige - anisotropic filtering
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_texturemode_box );
	// jkrige - anisotropic filtering


	Menu_AddItem( &s_opengl_menu, ( void * ) &s_fs_box );

	// jkrige - texture quality
	//Menu_AddItem( &s_opengl_menu, ( void * ) &s_tq_slider );
	// jkrige - texture quality

	// jkrige - palette removal
	//Menu_AddItem( &s_opengl_menu, ( void * ) &s_paletted_texture_box );
	// jkrige - palette removal

	Menu_AddItem( &s_opengl_menu, ( void * ) &s_finish_box );

	Menu_AddItem( &s_opengl_menu, ( void * ) &s_defaults_action );
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_cancel_action );

	Menu_Center( &s_opengl_menu );
}

/*
================
VID_MenuDraw
================
*/
void VID_MenuDraw (void)
{
	int w, h;

	s_current_menu = &s_opengl_menu;

	/*
	** draw the banner
	*/
	re.DrawGetPicSize( &w, &h, "m_banner_video" );

	// jkrige - scale2d
	re.DrawPic( viddef.width / 2 - (w*VID_SCALEFACTOR) / 2, viddef.height /2 - (110*VID_SCALEFACTOR), "m_banner_video" );
	//re.DrawPic( viddef.width / 2 - w / 2, viddef.height /2 - 110, "m_banner_video" );
	// jkrige - scale2d

	/*
	** move cursor to a reasonable starting position
	*/
	Menu_AdjustCursor( s_current_menu, 1 );

	/*
	** draw the menu
	*/
	Menu_Draw( s_current_menu );
}

/*
================
VID_MenuKey
================
*/
const char *VID_MenuKey( int key )
{
	menuframework_s *m = s_current_menu;
	static const char *sound = "misc/menu1.wav";

	switch ( key )
	{
	case K_ESCAPE:
		ApplyChanges( 0 );
		return NULL;
	case K_KP_UPARROW:
	case K_UPARROW:
		m->cursor--;
		Menu_AdjustCursor( m, -1 );
		break;
	case K_KP_DOWNARROW:
	case K_DOWNARROW:
		m->cursor++;
		Menu_AdjustCursor( m, 1 );
		break;
	case K_KP_LEFTARROW:
	case K_LEFTARROW:
		Menu_SlideItem( m, -1 );
		break;
	case K_KP_RIGHTARROW:
	case K_RIGHTARROW:
		Menu_SlideItem( m, 1 );
		break;
	case K_KP_ENTER:
	case K_ENTER:
		if ( !Menu_SelectItem( m ) )
			ApplyChanges( NULL );
		break;
	}

	return sound;
}
