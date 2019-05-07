// ----------------------------------------------------------------------------
// file name: TECStl.h
//    author: Dominik Seichter (TEC-IT)
// copyright: (c) TEC-IT Datenverarbeitung GmbH., 2004-2006
// ----------------------------------------------------------------------------
//   content: 
// ----------------------------------------------------------------------------
// history: 
// (DS 2004-07-19) : creation
// ----------------------------------------------------------------------------

#ifndef _TEC_STL_H_
#define _TEC_STL_H_

#ifdef __cplusplus
  #ifndef TECIT_NOSTL

    #ifdef _WIN32
      /* #pragma's for WIN32 should be defined as such,
         so that they get only used on WIN32.
         Normally a compiler should ignore them, if he does not 
         know such a pragma - the old SuSE 7.2 compiler
         produced strange results because of them.
      */

      /* Disable some STL warnings, restore them at the end */
      #pragma warning( push ) /* Remember current settings */ 

      /* Disable some warnings */
      #pragma warning( disable : 4127 ) /* conditional expression is constant */
    #endif // _WIN32


    #include <vector>
    #include <list>
    #include <stack>
    #include <map>
    #include <set>
    #include <deque>
    #include <string>
    #include <algorithm>


    ///////////////////////////////////////////////////////////////////////////////
    // TEC-IT standard data types
    ///////////////////////////////////////////////////////////////////////////////
    #ifdef _UNICODE

      // DIAB compiler error: namespace "std" has no member "wstring"
      #ifndef TEC_COMPILER_DIAB
      typedef	std::wstring   c_String;  /* STL wide String class				 */
      #endif
    #else
      typedef	std::string    c_String;  /* STL String class						 */
    #endif

    #ifdef _WIN32
      /* #pragma's for WIN32 should be defined as such,
         so that they get only used on WIN32.
         Normally a compiler should ignore them, if he does not 
         know such a pragma - the old SuSE 7.2 compiler
         produced strange results because of them.
      */

      /* Restore STL warnings */
      #pragma warning( pop )
    #endif // _WIN32

  #endif // TECIT_NOSTL
#endif //__cplusplus

#endif // _TEC_STL_H_

