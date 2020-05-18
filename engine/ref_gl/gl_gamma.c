/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
/*
** VID_GAMMA.C
*/

#include "gl_local.h"

static unsigned char gl_gammatable[256];

cvar_t *vid_overbrightbits;
static int overbrightBits;

static unsigned short oldHardwareGamma[3][256];
qboolean deviceSupportsGamma = false;

/*
** VG_CheckHardwareGamma
**
** Determines if the underlying hardware supports the Win32 gamma correction API.
*/
void VG_CheckHardwareGamma( void )
{
	HDC	hDC;

	deviceSupportsGamma = false;

	hDC = GetDC( GetDesktopWindow() );
	deviceSupportsGamma = GetDeviceGammaRamp( hDC, oldHardwareGamma );
	ReleaseDC( GetDesktopWindow(), hDC );

	if ( deviceSupportsGamma )
	{
		// do a sanity check on the gamma values
		if ( ( HIBYTE( oldHardwareGamma[0][255] ) <= HIBYTE( oldHardwareGamma[0][0] ) ) ||
				( HIBYTE( oldHardwareGamma[1][255] ) <= HIBYTE( oldHardwareGamma[1][0] ) ) ||
				( HIBYTE( oldHardwareGamma[2][255] ) <= HIBYTE( oldHardwareGamma[2][0] ) ) )
		{
			deviceSupportsGamma = false;
			ri.Con_Printf(PRINT_ALL, "Device has broken gamma support\n");
		}

		// make sure that we didn't have a prior crash in the game, and if so we need to
		// restore the gamma values to at least a linear value
		if ( ( HIBYTE( oldHardwareGamma[0][181] ) == 255 ) )
		{
			int g;

			ri.Con_Printf(PRINT_ALL, "Suspicious gamma tables, using linear ramp for restoration\n");

			for ( g = 0; g < 255; g++ )
			{
				oldHardwareGamma[0][g] = g << 8;
				oldHardwareGamma[1][g] = g << 8;
				oldHardwareGamma[2][g] = g << 8;
			}
		}
	}
}

/*
** VG_SetGamma
**
** This routine should only be called if deviceSupportsGamma is TRUE
*/
void VG_SetGamma( unsigned char red[256], unsigned char green[256], unsigned char blue[256] )
{
	unsigned short table[3][256];
	int		i, j;
	int		ret;
	//OSVERSIONINFO	vinfo; // jkrige - remove windows version check
	HDC hDC;

	hDC = GetDC(GetDesktopWindow());

	if (!deviceSupportsGamma || !hDC)
		return;

	for ( i = 0; i < 256; i++ ) {
		table[0][i] = ( ( ( unsigned short ) red[i] ) << 8 ) | red[i];
		table[1][i] = ( ( ( unsigned short ) green[i] ) << 8 ) | green[i];
		table[2][i] = ( ( ( unsigned short ) blue[i] ) << 8 ) | blue[i];
	}

	// jkrige - remove windows version check - begin
	// Win2K puts this odd restriction on gamma ramps...
	/*vinfo.dwOSVersionInfoSize = sizeof(vinfo);
	GetVersionEx( &vinfo );
	if ( vinfo.dwMajorVersion == 5 && vinfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
	{
		ri.Con_Printf(PRINT_DEVELOPER, "performing Windows 2000 gamma clamp.\n");
		for ( j = 0 ; j < 3 ; j++ ) {
			for ( i = 0 ; i < 128 ; i++ ) {
				if ( table[j][i] > ( (128+i) << 8 ) ) {
					table[j][i] = (128+i) << 8;
				}
			}
			if ( table[j][127] > 254<<8 ) {
				table[j][127] = 254<<8;
			}
		}
	}
	else
	{
		ri.Con_Printf(PRINT_DEVELOPER, "skipping Windows 2000 gamma clamp.\n");
	}*/
	// jkrige - remove windows version check - end

	// enforce constantly increasing
	for ( j = 0 ; j < 3 ; j++ ) {
		for ( i = 1 ; i < 256 ; i++ ) {
			if ( table[j][i] < table[j][i-1] ) {
				table[j][i] = table[j][i-1];
			}
		}
	}

	ret = SetDeviceGammaRamp( hDC, table );
	if (!ret)
		ri.Con_Printf(PRINT_ALL, "SetDeviceGammaRamp failed.\n");

	ReleaseDC( GetDesktopWindow(), hDC );
}

/*
** VG_RestoreGamma
*/
void VG_RestoreGamma( void )
{
	HDC hDC;

	hDC = GetDC( GetDesktopWindow() );

	if (deviceSupportsGamma)
	{
		SetDeviceGammaRamp(hDC, oldHardwareGamma);
	}

	ReleaseDC(GetDesktopWindow(), hDC);
}

/*
===============
VG_GammaCorrect
===============
*/
void VG_GammaCorrect( byte *buffer, int bufSize )
{
	int i;

	if (!deviceSupportsGamma)
		return;

	for ( i = 0; i < bufSize; i++ )
	{
		buffer[i] = gl_gammatable[buffer[i]];
	}
}

/*
===============
VG_SetColorMappings
===============
*/
void VG_SetColorMappings( void )
{
	int		i, j;
	float	g;
	int		inf;
	int		shift;

	// setup the overbright lighting
	vid_overbrightbits = ri.Cvar_Get("vid_overbrightbits", "0", CVAR_ARCHIVE);
	overbrightBits = vid_overbrightbits->integer;


	if ( !deviceSupportsGamma )
		overbrightBits = 0;		// need hardware gamma for overbright

	// never overbright in windowed mode
	if (!vid_fullscreen->integer)
		overbrightBits = 0;

	// allow 2 overbright bits in 24 bit
	if (overbrightBits > 2)
		overbrightBits = 2;

	if (overbrightBits < 0)
		overbrightBits = 0;

	if (vid_gamma->value < 1.0f)
	{
		ri.Cvar_SetValue("vid_gamma", 1.0f);
		return;
	}
	else if (vid_gamma->value > 2.7f)
	{
		ri.Cvar_SetValue("vid_gamma", 2.7f);
		return;
	}

	g = 3.0f - vid_gamma->value;

	if (vid_overbrightbits->value < 0.0f)
	{
		ri.Cvar_SetValue("vid_overbrightbits", 0.0f);
		return;
	}
	else if (vid_overbrightbits->value >(float)overbrightBits)
	{
		ri.Cvar_SetValue("vid_overbrightbits", (float)overbrightBits);
		return;
	}

	shift = vid_overbrightbits->integer;

	for (i = 0; i < 256; i++)
	{
		if (g == 1)
		{
			inf = i;
		}
		else
		{
			inf = 255 * pow(i / 255.0f, 1.0f / g) + 0.5f;
		}
		inf <<= shift;
		if (inf < 0)
		{
			inf = 0;
		}
		if (inf > 255)
		{
			inf = 255;
		}
		gl_gammatable[i] = inf;
	}

	if (deviceSupportsGamma)
	{
		VG_SetGamma(gl_gammatable, gl_gammatable, gl_gammatable);
	}
}
