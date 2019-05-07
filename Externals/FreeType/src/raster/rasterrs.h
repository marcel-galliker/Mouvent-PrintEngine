/***************************************************************************/
/*                                                                         */
/*  rasterrs.h                                                             */
/*                                                                         */
/*    monochrome renderer error codes (specification only).                */
/*                                                                         */
/*  Copyright 2001, 2012 by                                                */
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
  /* This file is used to define the monochrome renderer error enumeration */
  /* constants.                                                            */
  /*                                                                       */
  /*************************************************************************/

#ifndef __RASTERRS_H__
#define __RASTERRS_H__

#include FT_MODULE_ERRORS_H

#undef __FTERRORS_H__

#undef  FT_ERR_PREFIX
#define FT_ERR_PREFIX  Raster_Err_
#define FT_ERR_BASE    FT_Mod_Err_Raster

#include FT_ERRORS_H

#undef FT_THROW
#define FT_THROW( e )  FT_ERR_CAT( Raster_Err_, e )

#undef FT_ERR
#define FT_ERR( e )  FT_ERR_CAT( Raster_Err_, e )

#undef FT_ERR_EQ
#define FT_ERR_EQ( x, e )    ( FT_ERROR_BASE( x ) == FT_ERROR_BASE( FT_ERR( e ) ) )

#undef FT_ERR_NEQ
#define FT_ERR_NEQ( x, e )   ( FT_ERROR_BASE( x ) != FT_ERROR_BASE( FT_ERR( e ) ) )

#undef  FT_INVALID_TOO_SHORT
#define FT_INVALID_TOO_SHORT FT_INVALID( Raster_Err_, Invalid_Table )

/* called when an invalid offset is detected */
#undef  FT_INVALID_OFFSET
#define FT_INVALID_OFFSET   FT_INVALID( Raster_Err_, Invalid_Offset )

  /* called when an invalid format/value is detected */
#undef  FT_INVALID_FORMAT
#define FT_INVALID_FORMAT   FT_INVALID( Raster_Err_, Invalid_Table )

  /* called when an invalid glyph index is detected */
#undef  FT_INVALID_GLYPH_ID
#define FT_INVALID_GLYPH_ID FT_INVALID( Raster_Err_, Invalid_Glyph_Index )

  /* called when an invalid field value is detected */
#undef  FT_INVALID_DATA
#define FT_INVALID_DATA     FT_INVALID( Raster_Err_, Invalid_Table )

#endif /* __RASTERRS_H__ */


/* END */
