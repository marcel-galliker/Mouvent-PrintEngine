/***************************************************************************/
/*                                                                         */
/*  ft2build.h                                                             */
/*                                                                         */
/*    FreeType 2 build and setup macros.                                   */
/*                                                                         */
/*  Copyright 1996-2001, 2006, 2013 by                                     */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* This is the `entry point' for FreeType header file inclusions.  It is */
  /* the only header file which should be included directly; all other     */
  /* FreeType header files should be accessed with macro names (after      */
  /* including `ft2build.h').                                              */
  /*                                                                       */
  /* A typical example is                                                  */
  /*                                                                       */
  /*   #include <ft2build.h>                                               */
  /*   #include FT_FREETYPE_H                                              */
  /*                                                                       */
  /*************************************************************************/


#ifndef __FT2BUILD_H__
#define __FT2BUILD_H__

/*--- adaptions radex -------------------------------------------------*/
#define FT_INTERNAL_PIC_H 				<internal/ftpic.h>
#define FT_INTERNAL_OBJECTS_H			<internal/ftobjs.h>
#define FT_INTERNAL_MEMORY_H			<internal/ftmemory.h>
#define FT_INTERNAL_GLYPH_LOADER_H		<internal/ftgloadr.h>
#define FT_INTERNAL_DRIVER_H			<internal/ftdriver.h>
#define FT_INTERNAL_AUTOHINT_H			<internal/autohint.h>
#define FT_INTERNAL_SERVICE_H			<internal/ftserv.h>
#define FT_INTERNAL_DEBUG_H				<internal/ftdebug.h>
#define FT_INTERNAL_CALC_H				<internal/ftcalc.h>
#define FT_INTERNAL_VALIDATE_H			<internal/ftvalid.h>
#define FT_INTERNAL_RFORK_H				<internal/ftrfork.h>
#define FT_INTERNAL_STREAM_H			<internal/ftstream.h>
#define FT_INTERNAL_SFNT_H				<internal/sfnt.h>
#define FT_INTERNAL_TRUETYPE_TYPES_H	<internal/tttypes.h>

#define FT_CONFIG_CONFIG_H				<config/ftconfig.h>
#define FT_CONFIG_OPTIONS_H				<config/ftoption.h>

#define FT_ERRORS_H  					<fterrors.h>
#define FT_ERROR_DEFINITIONS_H			<fterrdef.h>

#define FT_IMAGE_H						<ftimage.h>
#define FT_TYPES_H						<fttypes.h>
#define FT_MODULE_ERRORS_H				<ftmoderr.h>

/*-----------------------------------------------------*/

#include <config/ftheader.h>

#ifdef _WIN64
	#pragma warning (disable:4244)
#endif

#endif /* __FT2BUILD_H__ */


/* END */
