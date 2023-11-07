/********************************************************************************
*                                                                               *
*                    A S C I I   C h a r a c t e r   I n f o                    *
*                                                                               *
*********************************************************************************
* Copyright (C) 2005,2023 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* This library is free software; you can redistribute it and/or modify          *
* it under the terms of the GNU Lesser General Public License as published by   *
* the Free Software Foundation; either version 3 of the License, or             *
* (at your option) any later version.                                           *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
* GNU Lesser General Public License for more details.                           *
*                                                                               *
* You should have received a copy of the GNU Lesser General Public License      *
* along with this program.  If not, see <http://www.gnu.org/licenses/>          *
********************************************************************************/
#ifndef FXASCII_H
#define FXASCII_H


/******** Generated on 2023/08/16 11:48:45 by ascii tool version 4.0.0 *********/


namespace FX {


namespace Ascii {

/// Character properties
enum {
  AlphaNum = 0x0001,    /// Alphabetic or decimal character
  Letter   = 0x0002,    /// Letter character
  Control  = 0x0004,    /// Control character
  Digit    = 0x0008,    /// Decimal digit
  Graph    = 0x0010,    /// Printable character (excluding spaces)
  Lower    = 0x0020,    /// Lower-case character
  Print    = 0x0040,    /// Printable character (including spaces)
  Punct    = 0x0080,    /// Punctuation character
  Space    = 0x0100,    /// Space
  Upper    = 0x0200,    /// Upper-case character
  HexDigit = 0x0400,    /// Hexadecimal digit
  Blank    = 0x0800,    /// Space or tab
  Word     = 0x1000,    /// Word character (including '_')
  Delim    = 0x2000,    /// Delimiter character (excluding '_')
  Case     = 0x4000,    /// Character has case
  };


// Ascii character property table
extern FXAPI const FXushort ascii_data[256];

// Value to ascii digit table
extern FXAPI const FXchar value_to_digit[256];

// Ascii digit to valuetable
extern FXAPI const FXschar digit_to_value[256];


// Numeric value of ascii digit
static inline FXint digitValue(FXchar asc){
  return digit_to_value[(FXuchar)asc];
  }


// Ascii digit of numeric value
static inline FXint valueDigit(FXuchar asc){
  return value_to_digit[(FXuchar)asc];
  }


// Character properties
static inline FXushort charProperties(FXchar asc){
  return ascii_data[(FXuchar)asc];
  }


// Has upper or lower case variant
static inline FXbool hasCase(FXchar asc){
  return (charProperties(asc)&Case)!=0;
  }


// Is upper case
static inline FXbool isUpper(FXchar asc){
  return (charProperties(asc)&Upper)!=0;
  }


// Is lower case
static inline FXbool isLower(FXchar asc){
  return (charProperties(asc)&Lower)!=0;
  }


// Is title case
static inline FXbool isTitle(FXchar asc){
  return (charProperties(asc)&Upper)!=0;
  }


// Is us-ascii
static inline FXbool isAscii(FXchar asc){
  return ((FXuchar)asc)<128;
  }


// Is letter
static inline FXbool isLetter(FXchar asc){
  return (charProperties(asc)&Letter)!=0;
  }


// Is decimal digit
static inline FXbool isDigit(FXchar asc){
  return (charProperties(asc)&Digit)!=0;
  }


// Is letter or digit
static inline FXbool isAlphaNumeric(FXchar asc){
  return (charProperties(asc)&AlphaNum)!=0;
  }


// Is control character
static inline FXbool isControl(FXchar asc){
  return (charProperties(asc)&Control)!=0;
  }


// Is space
static inline FXbool isSpace(FXchar asc){
  return (charProperties(asc)&Space)!=0;
  }


// Is blank
static inline FXbool isBlank(FXchar asc){
  return (charProperties(asc)&Blank)!=0;
  }


// Is punctuation character
static inline FXbool isPunct(FXchar asc){
  return (charProperties(asc)&Punct)!=0;
  }


// Is graphic character
static inline FXbool isGraph(FXchar asc){
  return (charProperties(asc)&Graph)!=0;
  }


// Is printing character
static inline FXbool isPrint(FXchar asc){
  return (charProperties(asc)&Print)!=0;
  }


// Is hexadecimal digit
static inline FXbool isHexDigit(FXchar asc){
  return (charProperties(asc)&HexDigit)!=0;
  }


// Is octal digit
static inline FXbool isOctDigit(FXchar asc){
  return (asc&0xF8)==0x30;
  }


// Is binary digit
static inline FXbool isBinDigit(FXchar asc){
  return (asc&0xFE)==0x30;
  }


// Is word character
static inline FXbool isWord(FXchar asc){
  return (charProperties(asc)&Word)!=0;
  }


// Is delimiter character
static inline FXbool isDelim(FXchar asc){
  return (charProperties(asc)&Delim)!=0;
  }


// Convert to upper case
static inline FXchar toUpper(FXchar asc){
  return asc+((((96-(FXint)asc)&((FXint)asc-123))>>31)&-32);
  }


// Convert to lower case
static inline FXchar toLower(FXchar asc){
  return asc+((((64-(FXint)asc)&((FXint)asc-91))>>31)&32);
  }


// Convert to title case
static inline FXchar toTitle(FXchar asc){
  return asc+((((96-(FXint)asc)&((FXint)asc-123))>>31)&-32);
  }

}

}

#endif
