/********************************************************************************
*                                                                               *
*                  F O X   D e s k t o p   C a l c u l a t o r                  *
*                                                                               *
*********************************************************************************
* Copyright (C) 2001,2024 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* This program is free software: you can redistribute it and/or modify          *
* it under the terms of the GNU General Public License as published by          *
* the Free Software Foundation, either version 3 of the License, or             *
* (at your option) any later version.                                           *
*                                                                               *
* This program is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
* GNU General Public License for more details.                                  *
*                                                                               *
* You should have received a copy of the GNU General Public License             *
* along with this program.  If not, see <http://www.gnu.org/licenses/>.         *
********************************************************************************/
#include "fx.h"
#include "fxkeys.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <signal.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <ctype.h>
#include <math.h>
#include "icons.h"
#include "Calculator.h"
#include "Preferences.h"
#include "HelpWindow.h"


#define BINARY_LIMIT      64                      // 32 bits
#define OCTAL_LIMIT       22                      // 11 digits
#define DECIMAL_LIMIT     16                      // +1.234567890123456E-308
#define HEXADECIMAL_LIMIT 16                       // 8 hexadecimal digits

#define GOLDEN            1.6180339887498948482045868343        // Golden ratio
#define INVGOLDEN         (1.0/GOLDEN)                          // Inverse golden ratio

#define	DEG2RAD(x)	  (((2.0*PI)/360.0)*(x))  // Degrees to radians
#define	GRA2RAD(x)	  ((PI/200.0)*(x))        // Grad to radians
#define	RAD2DEG(x)	  ((360.0/(2.0*PI))*(x))  // Radians to degrees
#define	RAD2GRA(x)	  ((200.0/PI)*(x))        // Radians to grad


/*
  Notes:

  - On window enter, direct keyboard focus to numeric input buttons
  - When clicking on the display focus should stay on buttons
  - On resize I'd like x-y aspect to remain the same
  - Would be nice if font size more or less follows window size
  - History option in display to retrieve back earlier results
    (stored when pressing '=')
*/

/*******************************************************************************/



// Map
FXDEFMAP(Calculator) CalculatorMap[]={
  FXMAPFUNCS(SEL_UPDATE,Calculator::ID_0,Calculator::ID_9,Calculator::onUpdDigit),
  FXMAPFUNCS(SEL_COMMAND,Calculator::ID_0,Calculator::ID_9,Calculator::onCmdDigit),
  FXMAPFUNCS(SEL_UPDATE,Calculator::ID_A,Calculator::ID_F,Calculator::onUpdHexDigit),
  FXMAPFUNCS(SEL_COMMAND,Calculator::ID_A,Calculator::ID_F,Calculator::onCmdHexDigit),
  FXMAPFUNCS(SEL_COMMAND,Calculator::ID_MODE,Calculator::ID_GRA,Calculator::onCmdAngle),
  FXMAPFUNCS(SEL_UPDATE,Calculator::ID_MODE,Calculator::ID_GRA,Calculator::onUpdAngle),
  FXMAPFUNCS(SEL_COMMAND,Calculator::ID_BASE,Calculator::ID_HEX,Calculator::onCmdBase),
  FXMAPFUNCS(SEL_UPDATE,Calculator::ID_BASE,Calculator::ID_HEX,Calculator::onUpdBase),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_CLEAR,Calculator::onCmdClear),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_CLEARALL,Calculator::onCmdClearAll),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_INV,Calculator::onCmdInverse),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_HYP,Calculator::onCmdHyper),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_MEM_REC,Calculator::onCmdMemRec),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_MEM_ADD,Calculator::onCmdMemAdd),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_MEM_SUB,Calculator::onCmdMemSub),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_MEM_CLR,Calculator::onCmdMemClr),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_PNT,Calculator::onCmdPoint),
  FXMAPFUNC(SEL_UPDATE,Calculator::ID_PNT,Calculator::onUpdPoint),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_EXP,Calculator::onCmdExp),
  FXMAPFUNC(SEL_UPDATE,Calculator::ID_EXP,Calculator::onUpdExp),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_DELETE,Calculator::onCmdDelete),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_SIN,Calculator::onCmdSin),
  FXMAPFUNC(SEL_UPDATE,Calculator::ID_SIN,Calculator::onUpdSin),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_COS,Calculator::onCmdCos),
  FXMAPFUNC(SEL_UPDATE,Calculator::ID_COS,Calculator::onUpdCos),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_TAN,Calculator::onCmdTan),
  FXMAPFUNC(SEL_UPDATE,Calculator::ID_TAN,Calculator::onUpdTan),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_LOG,Calculator::onCmdLog),
  FXMAPFUNC(SEL_UPDATE,Calculator::ID_LOG,Calculator::onUpdLog),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_LN,Calculator::onCmdLn),
  FXMAPFUNC(SEL_UPDATE,Calculator::ID_LN,Calculator::onUpdLn),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_PI,Calculator::onCmdPi),
  FXMAPFUNC(SEL_UPDATE,Calculator::ID_PI,Calculator::onUpdPi),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_FAC,Calculator::onCmdFac),
  FXMAPFUNC(SEL_UPDATE,Calculator::ID_FAC,Calculator::onUpdFac),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_PER,Calculator::onCmdPer),
  FXMAPFUNC(SEL_UPDATE,Calculator::ID_PER,Calculator::onUpdPer),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_COM,Calculator::onCmdCom),
  FXMAPFUNC(SEL_UPDATE,Calculator::ID_COM,Calculator::onUpdCom),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_RECIP,Calculator::onCmdRecip),
  FXMAPFUNC(SEL_UPDATE,Calculator::ID_RECIP,Calculator::onUpdRecip),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_PLUSMIN,Calculator::onCmdPlusMin),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_XTOY,Calculator::onCmdXToY),
  FXMAPFUNC(SEL_UPDATE,Calculator::ID_XTOY,Calculator::onUpdXToY),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_SQRT,Calculator::onCmdSqrt),
  FXMAPFUNC(SEL_UPDATE,Calculator::ID_SQRT,Calculator::onUpdSqrt),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_SHL,Calculator::onCmdShl),
  FXMAPFUNC(SEL_UPDATE,Calculator::ID_SHL,Calculator::onUpdShl),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_SHR,Calculator::onCmdShr),
  FXMAPFUNC(SEL_UPDATE,Calculator::ID_SHR,Calculator::onUpdShr),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_2LOG,Calculator::onCmd2Log),
  FXMAPFUNC(SEL_UPDATE,Calculator::ID_2LOG,Calculator::onUpd2Log),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_LPAR,Calculator::onCmdLPar),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_RPAR,Calculator::onCmdRPar),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_AND,Calculator::onCmdAnd),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_OR,Calculator::onCmdOr),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_XOR,Calculator::onCmdXor),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_NOT,Calculator::onCmdNot),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_MUL,Calculator::onCmdMul),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_DIV,Calculator::onCmdDiv),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_MOD,Calculator::onCmdMod),
  FXMAPFUNC(SEL_UPDATE,Calculator::ID_MOD,Calculator::onUpdMod),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_ADD,Calculator::onCmdAdd),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_SUB,Calculator::onCmdSub),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_ENTER,Calculator::onCmdEnter),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_CLIPBOARD_COPY,Calculator::onCmdClipboardCopy),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_CLIPBOARD_PASTE,Calculator::onCmdClipboardPaste),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_PREFERENCES,Calculator::onCmdPreferences),
  FXMAPFUNCS(SEL_COMMAND,Calculator::ID_COLOR_DISPLAY,Calculator::ID_COLOR_CLEAR,Calculator::onCmdColor),
  FXMAPFUNCS(SEL_CHANGED,Calculator::ID_COLOR_DISPLAY,Calculator::ID_COLOR_CLEAR,Calculator::onCmdColor),
  FXMAPFUNCS(SEL_UPDATE,Calculator::ID_COLOR_DISPLAY,Calculator::ID_COLOR_CLEAR,Calculator::onUpdColor),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_EXPONENT_ALWAYS,Calculator::onCmdExponent),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_EXPONENT_NEVER,Calculator::onCmdExponent),
  FXMAPFUNC(SEL_UPDATE,Calculator::ID_EXPONENT_ALWAYS,Calculator::onUpdExponent),
  FXMAPFUNC(SEL_UPDATE,Calculator::ID_EXPONENT_NEVER,Calculator::onUpdExponent),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_ENGINEERING_MODE,Calculator::onCmdEngineeringMode),
  FXMAPFUNC(SEL_UPDATE,Calculator::ID_ENGINEERING_MODE,Calculator::onUpdEngineeringMode),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_DECIMAL_POINT,Calculator::onCmdDecimalPoint),
  FXMAPFUNC(SEL_UPDATE,Calculator::ID_DECIMAL_POINT,Calculator::onUpdDecimalPoint),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_PRECISION,Calculator::onCmdPrecision),
  FXMAPFUNC(SEL_UPDATE,Calculator::ID_PRECISION,Calculator::onUpdPrecision),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_QUESTION,Calculator::onCmdQuestion),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_BEEP,Calculator::onCmdBeep),
  FXMAPFUNC(SEL_UPDATE,Calculator::ID_BEEP,Calculator::onUpdBeep),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_DISPLAYFONT,Calculator::onCmdDisplayFont),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_MODEFONT,Calculator::onCmdModeFont),
  FXMAPFUNC(SEL_COMMAND,Calculator::ID_OPERATORFONT,Calculator::onCmdOperatorFont),
  FXMAPFUNC(SEL_CLIPBOARD_LOST,0,Calculator::onClipboardLost),
  FXMAPFUNC(SEL_CLIPBOARD_GAINED,0,Calculator::onClipboardGained),
  FXMAPFUNC(SEL_CLIPBOARD_REQUEST,0,Calculator::onClipboardRequest),
  };


// Implementation
FXIMPLEMENT(Calculator,FXMainWindow,CalculatorMap,ARRAYNUMBER(CalculatorMap))


// Double precision inf and nan
static const union{ FXulong u; FXdouble f; } dblinf={FXULONG(0x7ff0000000000000)};
static const union{ FXulong u; FXdouble f; } dblnan={FXULONG(0x7fffffffffffffff)};

// Physics constants
const FXdouble LIGHTSPEED=299792458.0;          // Light speed in vacuum (m/s)
const FXdouble PLANCK=6.62607015E-34;           // Planck constant h, (J*s)

const FXdouble MU0=1.25663706127E-6;            // Vacuum magnetic permeability (H/m)
const FXdouble EPS0=8.8541878188E-12;           // Vacuum electric permittivity (F/m)
const FXdouble ZETA0=376.730313412;             // Free space wave impedance (Ohm)
const FXdouble GRAVITY=6.67430E-11;             // Gravitational constant (m^3/(kg*s^2))

const FXdouble ELECTRONVOLT=1.602176634E-19;    // Electron volt (J)
const FXdouble ELECTRONCHARGE=1.602176634E-19;  // Change of the electron (Coulomb)
const FXdouble ELECTRONMASS=9.1093837139E-31;   // Electron mass (kg)
const FXdouble PROTONMASS=1.66053906892E-27;    // Proton mass (kg)

const FXdouble FARADAY=96485.33212;             // Faraday constant (Coulomb/mol)
const FXdouble AVOGADRO=6.02214076E23;          // Avogadro constant (1/mol)
const FXdouble BOLTZMAN=1.380649E-23;           // Boltzman constant (J/K)
const FXdouble RGAS=8.31446261815324;           // Gas constant (J/(mol*K))

// Operator priorities
static const FXuchar priority[]={
  1,  // DY_OR
  1,  // DY_XOR
  1,  // DY_AND
  2,  // DY_SUB
  2,  // DY_ADD
  3,  // DY_MOD
  3,  // DY_IDIV
  3,  // DY_DIV
  3,  // DY_MUL
  4,  // DY_XTOY
  4,  // DY_XTOINVY
  5,  // DY_PER
  5,  // DY_COM
  8,  // DY_LPAR
  8,  // DY_RPAR
  };



/*******************************************************************************/

// Construct calculator dialog
Calculator::Calculator(FXApp* a):FXMainWindow(a,"FOX Calculator",nullptr,nullptr,DECOR_ALL, 0,0,0,0, 0,0){

  // Default font used by default, duh!
  displayFont=nullptr;
  modeFont=nullptr;
  operatorFont=nullptr;

  // Make some icons
  bigicon=new FXGIFIcon(getApp(),bigcalc);
  smallicon=new FXGIFIcon(getApp(),tinycalc);
  cmem=new FXBMPIcon(getApp(),constmem);
  quest=new FXGIFIcon(getApp(),question);

  // Application icons
  setIcon(bigicon);
  setMiniIcon(smallicon);

  // Interior
  FXVerticalFrame *vert=new FXVerticalFrame(this,LAYOUT_FILL_X,0,0,0,0, 8,8,8,4, 1,1);
  FXHorizontalFrame *displayframe=new FXHorizontalFrame(vert,LAYOUT_FILL_X,0,0,0,0, 0,0,0,0);
  new FXButton(displayframe,"FOX Calculator",bigicon,this,ID_PREFERENCES,ICON_BEFORE_TEXT|JUSTIFY_LEFT|LAYOUT_FILL_Y,0,0,0,0, 4,4,2,2);
  new FXButton(displayframe,FXString::null,quest,this,ID_QUESTION,ICON_BEFORE_TEXT|JUSTIFY_LEFT|LAYOUT_FILL_Y,0,0,0,0, 4,4,2,2);
  display=new FXTextField(displayframe,16,this,ID_TEXT,TEXTFIELD_READONLY|FRAME_SUNKEN|FRAME_THICK|JUSTIFY_RIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 4,4,1,1);
  new FXLabel(vert,FXString::null,cmem,LAYOUT_RIGHT,0,0,0,0, 0,0,0,0);

  FXHorizontalFrame *modeframe=new FXHorizontalFrame(this,LAYOUT_FILL_X,0,0,0,0, 8,8,0,4, 8,8);

  FXHorizontalFrame *baseframe=new FXHorizontalFrame(modeframe,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH, 0,0,0,0, 0,0,0,0 ,0,0);

  numbase[0]=new FXButton(baseframe,"&Hex",nullptr,this,ID_HEX,FRAME_RAISED|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  numbase[1]=new FXButton(baseframe,"D&ec",nullptr,this,ID_DEC,FRAME_RAISED|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  numbase[2]=new FXButton(baseframe,"&Oct",nullptr,this,ID_OCT,FRAME_RAISED|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  numbase[3]=new FXButton(baseframe,"&Bin",nullptr,this,ID_BIN,FRAME_RAISED|LAYOUT_FILL_X|LAYOUT_FILL_Y);

  FXHorizontalFrame *degframe=new FXHorizontalFrame(modeframe,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH, 0,0,0,0, 0,0,0,0 ,0,0);

  angmode[0]=new FXButton(degframe,"&Deg",nullptr,this,ID_DEG,FRAME_RAISED|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 2,2,2,2);
  angmode[1]=new FXButton(degframe,"&Rad",nullptr,this,ID_RAD,FRAME_RAISED|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 2,2,2,2);
  angmode[2]=new FXButton(degframe,"&Gra",nullptr,this,ID_GRA,FRAME_RAISED|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 2,2,2,2);

  // Frame for button blocks
  FXHorizontalFrame *buttonframe=new FXHorizontalFrame(this,LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH, 0,0,0,0, 4,4,4,4, 0,0);

  // Functions block
  FXMatrix *funcblock=new FXMatrix(buttonframe,6,MATRIX_BY_ROWS|LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH|PACK_UNIFORM_HEIGHT);
  inverse=new FXButton(funcblock,"inv",nullptr,this,ID_INV,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);
  functions[0]=new FXButton(funcblock,"\xC2\xB1",nullptr,this,ID_PLUSMIN,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);
//functions[0]=new FXButton(funcblock,"+/-",nullptr,this,ID_PLUSMIN,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);
  functions[1]=new FXButton(funcblock,"1/x",nullptr,this,ID_RECIP,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);
  functions[2]=new FXButton(funcblock,"x^y",nullptr,this,ID_XTOY,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);
  functions[3]=new FXButton(funcblock,"\xE2\x88\x9A",nullptr,this,ID_SQRT,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);
//functions[3]=new FXButton(funcblock,"sqrt",nullptr,this,ID_SQRT,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);
//functions[4]=new FXButton(funcblock,"2log",nullptr,this,ID_2LOG,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);
  functions[4]=new FXButton(funcblock,"log\xE2\x82\x82",nullptr,this,ID_2LOG,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);

  functions[5]=new FXButton(funcblock,"\xCF\x80",nullptr,this,ID_PI,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);
//  functions[5]=new FXButton(funcblock,"pi",nullptr,this,ID_PI,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);
  functions[6]=new FXButton(funcblock,"shl",nullptr,this,ID_SHL,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);
  functions[7]=new FXButton(funcblock,"shr",nullptr,this,ID_SHR,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);
  functions[8]=new FXButton(funcblock,"x!",nullptr,this,ID_FAC,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);
  functions[9]=new FXButton(funcblock,"nPr",nullptr,this,ID_PER,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);
  functions[10]=new FXButton(funcblock,"nCr",nullptr,this,ID_COM,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);
  functions[6]->setTipText(tr("Shift left"));
  functions[7]->setTipText(tr("Shift right"));
  functions[8]->setTipText(tr("Factorial"));
  functions[9]->setTipText(tr("Permutations"));
  functions[10]->setTipText(tr("Combinations"));

  hyper2=new FXButton(funcblock,"hyp",nullptr,this,ID_HYP,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);
  functions[11]=new FXButton(funcblock,"sin",nullptr,this,ID_SIN,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);
  functions[12]=new FXButton(funcblock,"cos",nullptr,this,ID_COS,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);
  functions[13]=new FXButton(funcblock,"tan",nullptr,this,ID_TAN,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);
  functions[14]=new FXButton(funcblock,"log",nullptr,this,ID_LOG,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);
  functions[15]=new FXButton(funcblock,"ln",nullptr,this,ID_LN,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);

  digit[10]=new FXButton(funcblock,"A",nullptr,this,ID_A,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);
  digit[11]=new FXButton(funcblock,"B",nullptr,this,ID_B,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);
  digit[12]=new FXButton(funcblock,"C",nullptr,this,ID_C,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);
  digit[13]=new FXButton(funcblock,"D",nullptr,this,ID_D,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);
  digit[14]=new FXButton(funcblock,"E",nullptr,this,ID_E,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);
  digit[15]=new FXButton(funcblock,"F",nullptr,this,ID_F,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 8,8,1,1);

  // Main block
  FXMatrix *mainblock=new FXMatrix(buttonframe,5,MATRIX_BY_ROWS|LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH|PACK_UNIFORM_HEIGHT);
  memory[0]=new FXButton(mainblock,"MR",nullptr,this,ID_MEM_REC,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  digit[7]=new FXButton(mainblock,"7",nullptr,this,ID_7,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  digit[4]=new FXButton(mainblock,"4",nullptr,this,ID_4,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  digit[1]=new FXButton(mainblock,"1",nullptr,this,ID_1,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  digit[0]=new FXButton(mainblock,"0",nullptr,this,ID_0,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);

  memory[1]=new FXButton(mainblock,"M+",nullptr,this,ID_MEM_ADD,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  digit[8]=new FXButton(mainblock,"8",nullptr,this,ID_8,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  digit[5]=new FXButton(mainblock,"5",nullptr,this,ID_5,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  digit[2]=new FXButton(mainblock,"2",nullptr,this,ID_2,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  operators[0]=new FXButton(mainblock,".",nullptr,this,ID_PNT,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);

  memory[2]=new FXButton(mainblock,"M-",nullptr,this,ID_MEM_SUB,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  digit[9]=new FXButton(mainblock,"9",nullptr,this,ID_9,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  digit[6]=new FXButton(mainblock,"6",nullptr,this,ID_6,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  digit[3]=new FXButton(mainblock,"3",nullptr,this,ID_3,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  operators[1]=new FXButton(mainblock,"EXP",nullptr,this,ID_EXP,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);

  memory[3]=new FXButton(mainblock,"MC",nullptr,this,ID_MEM_CLR,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  operators[2]=new FXButton(mainblock,"(",nullptr,this,ID_LPAR,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  operators[3]=new FXButton(mainblock,"\xC3\x97",nullptr,this,ID_MUL,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
//operators[3]=new FXButton(mainblock,"*",nullptr,this,ID_MUL,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  operators[4]=new FXButton(mainblock,"+",nullptr,this,ID_ADD,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  operators[5]=new FXButton(mainblock,"=",nullptr,this,ID_ENTER,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);

  clearbtn=new FXButton(mainblock,"C",nullptr,this,ID_CLEAR,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  operators[6]=new FXButton(mainblock,")",nullptr,this,ID_RPAR,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  operators[7]=new FXButton(mainblock,"\xC3\xB7",nullptr,this,ID_DIV,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
//operators[7]=new FXButton(mainblock,"/",nullptr,this,ID_DIV,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  operators[8]=new FXButton(mainblock,"-",nullptr,this,ID_SUB,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
//operators[9]=new FXButton(mainblock,"mod",nullptr,this,ID_MOD,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  operators[9]=new FXButton(mainblock,"%",nullptr,this,ID_MOD,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);

  clearallbtn=new FXButton(mainblock,"AC",nullptr,this,ID_CLEARALL,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  operators[10]=new FXButton(mainblock,"&&",nullptr,this,ID_AND,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  //operators[10]=new FXButton(mainblock,"\xE2\x88\xA7",nullptr,this,ID_AND,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
//operators[10]=new FXButton(mainblock,"and",nullptr,this,ID_AND,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  operators[11]=new FXButton(mainblock,"|",nullptr,this,ID_OR,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  //operators[11]=new FXButton(mainblock,"\xE2\x88\xA8",nullptr,this,ID_OR,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
//operators[11]=new FXButton(mainblock,"or",nullptr,this,ID_OR,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  operators[12]=new FXButton(mainblock,"^",nullptr,this,ID_XOR,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  //operators[12]=new FXButton(mainblock,"\xE2\x8A\x95",nullptr,this,ID_XOR,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
//operators[12]=new FXButton(mainblock,"xor",nullptr,this,ID_XOR,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
//  operators[13]=new FXButton(mainblock,"~",nullptr,this,ID_NOT,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  operators[13]=new FXButton(mainblock,"\xC2\xAC",nullptr,this,ID_NOT,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
//operators[13]=new FXButton(mainblock,"not",nullptr,this,ID_NOT,BUTTON_NORMAL|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 1,1,2,2);
  operators[10]->setTipText(tr("Bitwise and"));
  operators[11]->setTipText(tr("Bitwise or"));
  operators[12]->setTipText(tr("Bitwise xor"));
  operators[13]->setTipText(tr("Bitwise not"));

  // Hot keys for digits
  digit[0]->addHotKey(MKUINT(KEY_0,0));
  digit[0]->addHotKey(MKUINT(KEY_KP_0,0));
  digit[1]->addHotKey(MKUINT(KEY_1,0));
  digit[1]->addHotKey(MKUINT(KEY_KP_1,0));
  digit[2]->addHotKey(MKUINT(KEY_2,0));
  digit[2]->addHotKey(MKUINT(KEY_KP_2,0));
  digit[3]->addHotKey(MKUINT(KEY_3,0));
  digit[3]->addHotKey(MKUINT(KEY_KP_3,0));
  digit[4]->addHotKey(MKUINT(KEY_4,0));
  digit[4]->addHotKey(MKUINT(KEY_KP_4,0));
  digit[5]->addHotKey(MKUINT(KEY_5,0));
  digit[5]->addHotKey(MKUINT(KEY_KP_5,0));
  digit[6]->addHotKey(MKUINT(KEY_6,0));
  digit[6]->addHotKey(MKUINT(KEY_KP_6,0));
  digit[7]->addHotKey(MKUINT(KEY_7,0));
  digit[7]->addHotKey(MKUINT(KEY_KP_7,0));
  digit[8]->addHotKey(MKUINT(KEY_8,0));
  digit[8]->addHotKey(MKUINT(KEY_KP_8,0));
  digit[9]->addHotKey(MKUINT(KEY_9,0));
  digit[9]->addHotKey(MKUINT(KEY_KP_9,0));

  // Hot keys for hex
  digit[10]->addHotKey(MKUINT(KEY_a,0));
  digit[11]->addHotKey(MKUINT(KEY_b,0));
  digit[12]->addHotKey(MKUINT(KEY_c,0));
  digit[13]->addHotKey(MKUINT(KEY_d,0));
  digit[14]->addHotKey(MKUINT(KEY_e,0));
  digit[15]->addHotKey(MKUINT(KEY_f,0));

  // Hot keys for operators
  operators[0]->addHotKey(MKUINT(KEY_period,0));
  operators[0]->addHotKey(MKUINT(KEY_KP_Decimal,0));
  operators[1]->addHotKey(MKUINT(KEY_E,SHIFTMASK));
  operators[2]->addHotKey(MKUINT(KEY_parenleft,SHIFTMASK));
  operators[3]->addHotKey(MKUINT(KEY_asterisk,SHIFTMASK));
  operators[3]->addHotKey(MKUINT(KEY_KP_Multiply,0));
  operators[4]->addHotKey(MKUINT(KEY_plus,SHIFTMASK));
  operators[4]->addHotKey(MKUINT(KEY_KP_Add,0));
  operators[5]->addHotKey(MKUINT(KEY_equal,0));
  operators[6]->addHotKey(MKUINT(KEY_parenright,SHIFTMASK));
  operators[7]->addHotKey(MKUINT(KEY_slash,0));
  operators[7]->addHotKey(MKUINT(KEY_KP_Divide,0));
  operators[8]->addHotKey(MKUINT(KEY_minus,0));
  operators[8]->addHotKey(MKUINT(KEY_KP_Subtract,0));
  operators[9]->addHotKey(MKUINT(KEY_percent,SHIFTMASK));
  operators[10]->addHotKey(MKUINT(KEY_ampersand,SHIFTMASK));
  operators[11]->addHotKey(MKUINT(KEY_bar,SHIFTMASK));
  operators[12]->addHotKey(MKUINT(KEY_asciicircum,SHIFTMASK));
  operators[13]->addHotKey(MKUINT(KEY_asciitilde,SHIFTMASK));

  // Shifting
  functions[6]->addHotKey(MKUINT(KEY_less,SHIFTMASK));
  functions[7]->addHotKey(MKUINT(KEY_greater,SHIFTMASK));
  functions[8]->addHotKey(MKUINT(KEY_exclam,SHIFTMASK));

  inverse->addHotKey(MKUINT(KEY_i,0));
  hyper2->addHotKey(MKUINT(KEY_h,0));

  // Add accelerators
  getAccelTable()->addAccel(MKUINT(KEY_Q,0),this,FXSEL(SEL_COMMAND,ID_CLOSE));
  getAccelTable()->addAccel(MKUINT(KEY_q,0),this,FXSEL(SEL_COMMAND,ID_CLOSE));
  getAccelTable()->addAccel(MKUINT(KEY_q,CONTROLMASK),this,FXSEL(SEL_COMMAND,ID_CLOSE));
  getAccelTable()->addAccel(MKUINT(KEY_Escape,0),this,FXSEL(SEL_COMMAND,ID_CLEAR));
  getAccelTable()->addAccel(MKUINT(KEY_BackSpace,0),this,FXSEL(SEL_COMMAND,ID_DELETE));
  getAccelTable()->addAccel(MKUINT(KEY_Delete,0),this,FXSEL(SEL_COMMAND,ID_DELETE));
  getAccelTable()->addAccel(MKUINT(KEY_KP_Delete,0),this,FXSEL(SEL_COMMAND,ID_DELETE));
  getAccelTable()->addAccel(MKUINT(KEY_Return,0),this,FXSEL(SEL_COMMAND,ID_ENTER));
  getAccelTable()->addAccel(MKUINT(KEY_KP_Enter,0),this,FXSEL(SEL_COMMAND,ID_ENTER));
  getAccelTable()->addAccel(MKUINT(KEY_c,CONTROLMASK),this,FXSEL(SEL_COMMAND,ID_CLIPBOARD_COPY));
  getAccelTable()->addAccel(MKUINT(KEY_v,CONTROLMASK),this,FXSEL(SEL_COMMAND,ID_CLIPBOARD_PASTE));

  // Initialize stuff
  display->setText("0");
  recall=0.0;
  numstack[0]=0.0;
  numsp=0;
  opsp=-1;
  limit=DECIMAL_LIMIT;
  digits=1;
  base=NUM_DEC;
  angles=ANG_RAD;
  precision=16;
  exponent=EXPONENT_IFNEEDED;
  beep=true;
  parens=0;
  modifiers=0;
  }


// Create and show window
void Calculator::create(){
  readRegistry();
  FXMainWindow::create();
  setDisplayValue(0.0);
  show();
  }


// Destroy calculator dialog
Calculator::~Calculator(){
  delete displayFont;
  delete modeFont;
  delete operatorFont;
  delete bigicon;
  delete smallicon;
  delete cmem;
  delete quest;
  }

/*******************************************************************************/

// Set digit color
void Calculator::setDigitColor(FXColor clr){
  FXColor hilite=makeHiliteColor(clr);
  FXColor shadow=makeShadowColor(clr);
  for(FXuint i=0; i<10; i++){
    digit[i]->setBackColor(clr);
    digit[i]->setHiliteColor(hilite);
    digit[i]->setShadowColor(shadow);
    }
  }


// Get digit color
FXColor Calculator::getDigitColor() const {
  return digit[0]->getBackColor();
  }


// Set digit color
void Calculator::setHexDigitColor(FXColor clr){
  FXColor hilite=makeHiliteColor(clr);
  FXColor shadow=makeShadowColor(clr);
  for(FXuint i=10; i<16; i++){
    digit[i]->setBackColor(clr);
    digit[i]->setHiliteColor(hilite);
    digit[i]->setShadowColor(shadow);
    }
  }


// Get digit color
FXColor Calculator::getHexDigitColor() const {
  return digit[10]->getBackColor();
  }


// Set operator color
void Calculator::setOperatorColor(FXColor clr){
  FXColor hilite=makeHiliteColor(clr);
  FXColor shadow=makeShadowColor(clr);
  for(FXuint i=0; i<ARRAYNUMBER(operators); i++){
    operators[i]->setBackColor(clr);
    operators[i]->setHiliteColor(hilite);
    operators[i]->setShadowColor(shadow);
    }
  }


// Get operator color
FXColor Calculator::getOperatorColor() const {
  return operators[0]->getBackColor();
  }


// Set function color
void Calculator::setFunctionColor(FXColor clr){
  FXColor hilite=makeHiliteColor(clr);
  FXColor shadow=makeShadowColor(clr);
  for(FXuint i=0; i<ARRAYNUMBER(functions); i++){
    functions[i]->setBackColor(clr);
    functions[i]->setHiliteColor(hilite);
    functions[i]->setShadowColor(shadow);
    }
  }


// Get function color
FXColor Calculator::getFunctionColor() const {
  return functions[0]->getBackColor();
  }


// Set memory color
void Calculator::setMemoryColor(FXColor clr){
  FXColor hilite=makeHiliteColor(clr);
  FXColor shadow=makeShadowColor(clr);
  for(FXuint i=0; i<ARRAYNUMBER(memory); i++){
    memory[i]->setBackColor(clr);
    memory[i]->setHiliteColor(hilite);
    memory[i]->setShadowColor(shadow);
    }
  }


// Get memory color
FXColor Calculator::getMemoryColor() const {
  return memory[0]->getBackColor();
  }


// Set inverse color
void Calculator::setInverseColor(FXColor clr){
  inverse->setBackColor(clr);
  inverse->setHiliteColor(makeHiliteColor(clr));
  inverse->setShadowColor(makeShadowColor(clr));
  }


// Get inverse color
FXColor Calculator::getInverseColor() const {
  return inverse->getBackColor();
  }


// Set hyp color
void Calculator::setHyperColor(FXColor clr){
  hyper2->setBackColor(clr);
  hyper2->setHiliteColor(makeHiliteColor(clr));
  hyper2->setShadowColor(makeShadowColor(clr));
  }


// Get hyp color
FXColor Calculator::getHyperColor() const {
  return hyper2->getBackColor();
  }


// Set clear color
void Calculator::setClearColor(FXColor clr){
  clearbtn->setBackColor(clr);
  clearbtn->setHiliteColor(makeHiliteColor(clr));
  clearbtn->setShadowColor(makeShadowColor(clr));
  }


// Get clear color
FXColor Calculator::getClearColor() const {
  return clearbtn->getBackColor();
  }


// Set clear all color
void Calculator::setClearAllColor(FXColor clr){
  clearallbtn->setBackColor(clr);
  clearallbtn->setHiliteColor(makeHiliteColor(clr));
  clearallbtn->setShadowColor(makeShadowColor(clr));
  }


// Get clear all color
FXColor Calculator::getClearAllColor() const {
  return clearallbtn->getBackColor();
  }


// Set display color
void Calculator::setDisplayColor(FXColor clr){
  display->setBackColor(clr);
  display->setSelTextColor(clr);
  display->setHiliteColor(makeHiliteColor(clr));
  display->setShadowColor(makeShadowColor(clr));
  }


// Get display color
FXColor Calculator::getDisplayColor() const {
  return display->getBackColor();
  }


// Set display color
void Calculator::setDisplayNumberColor(FXColor clr){
  display->setTextColor(clr);
  display->setSelBackColor(clr);
  }


// Get display color
FXColor Calculator::getDisplayNumberColor() const {
  return display->getTextColor();
  }


// Set numeric base color
void Calculator::setBaseColor(FXColor clr){
  FXColor hilite=FXRGB(FXREDVAL(clr)+((255-FXREDVAL(clr))*3)/8,
                       FXGREENVAL(clr)+((255-FXGREENVAL(clr))*3)/8,
                       FXBLUEVAL(clr)+((255-FXBLUEVAL(clr))*3)/8);
  FXColor shadow=makeShadowColor(clr);
  for(FXuint i=0; i<ARRAYNUMBER(numbase); i++){
    numbase[i]->setBackColor(clr);
    numbase[i]->setHiliteColor(hilite);
    numbase[i]->setShadowColor(shadow);
    }
  }


// Get numeric base  color
FXColor Calculator::getBaseColor() const {
  return numbase[0]->getBackColor();
  }


// Set angle mode color
void Calculator::setAngleColor(FXColor clr){
  FXColor hilite=FXRGB(FXREDVAL(clr)+((255-FXREDVAL(clr))*3)/8,
                       FXGREENVAL(clr)+((255-FXGREENVAL(clr))*3)/8,
                       FXBLUEVAL(clr)+((255-FXBLUEVAL(clr))*3)/8);
  FXColor shadow=makeShadowColor(clr);
  for(FXuint i=0; i<ARRAYNUMBER(angmode); i++){
    angmode[i]->setBackColor(clr);
    angmode[i]->setHiliteColor(hilite);
    angmode[i]->setShadowColor(shadow);
    }
  }


// Get angle mode color
FXColor Calculator::getAngleColor() const {
  return angmode[0]->getBackColor();
  }


// Set display font
void Calculator::setDisplayFont(FXFont* fnt){
  display->setFont(fnt);
  }


// Return display font
FXFont* Calculator::getDisplayFont() const {
  return display->getFont();
  }

// Set mode button font
void Calculator::setModeFont(FXFont* fnt){
  for(FXuint i=0; i<ARRAYNUMBER(numbase); i++){
    numbase[i]->setFont(fnt);
    }
  for(FXuint i=0; i<ARRAYNUMBER(angmode); i++){
    angmode[i]->setFont(fnt);
    }
  }

// Return mode button font
FXFont* Calculator::getModeFont() const {
  return numbase[0]->getFont();
  }


// Set operator font
void Calculator::setOperatorFont(FXFont* fnt){
  for(FXuint i=0; i<ARRAYNUMBER(operators); i++){
    operators[i]->setFont(fnt);
    }
  for(FXuint i=0; i<ARRAYNUMBER(functions); i++){
    functions[i]->setFont(fnt);
    }
  for(FXuint i=0; i<ARRAYNUMBER(digit); i++){
    digit[i]->setFont(fnt);
    }
  for(FXuint i=0; i<ARRAYNUMBER(memory); i++){
    memory[i]->setFont(fnt);
    }
  clearbtn->setFont(fnt);
  clearallbtn->setFont(fnt);
  inverse->setFont(fnt);
  hyper2->setFont(fnt);
  }

// Return operator font
FXFont* Calculator::getOperatorFont() const {
  return operators[0]->getFont();
  }

/*******************************************************************************/

// Read registry
void Calculator::readRegistry(){
  FXString fontspec;

  // Position
  FXint xx=getApp()->reg().readIntEntry("SETTINGS","x",50);
  FXint yy=getApp()->reg().readIntEntry("SETTINGS","y",50);
  FXint ww=getApp()->reg().readIntEntry("SETTINGS","w",0);
  FXint hh=getApp()->reg().readIntEntry("SETTINGS","h",0);

  // Read colors
  FXColor digitclr=getApp()->reg().readColorEntry("SETTINGS","digitcolor",FXRGB(94,209,204));
  FXColor hexdigitclr=getApp()->reg().readColorEntry("SETTINGS","hexdigitcolor",FXRGB(151,189,206));
  FXColor operatorclr=getApp()->reg().readColorEntry("SETTINGS","operatorcolor",FXRGB(255,222,163));
  FXColor functionclr=getApp()->reg().readColorEntry("SETTINGS","functioncolor",FXRGB(158,213,188));
  FXColor memoryclr=getApp()->reg().readColorEntry("SETTINGS","memorycolor",FXRGB(181,207,227));
  FXColor inverseclr=getApp()->reg().readColorEntry("SETTINGS","inversecolor",FXRGB(224,222,69));
  FXColor hyperclr=getApp()->reg().readColorEntry("SETTINGS","hypercolor",FXRGB(224,222,69));
  FXColor clearclr=getApp()->reg().readColorEntry("SETTINGS","clearcolor",FXRGB(238,148,0));
  FXColor clearallclr=getApp()->reg().readColorEntry("SETTINGS","clearallcolor",FXRGB(238,118,0));
  FXColor displayclr=getApp()->reg().readColorEntry("SETTINGS","displaycolor",FXRGB(255,255,255));
  FXColor numberclr=getApp()->reg().readColorEntry("SETTINGS","displaynumbercolor",FXRGB(0,0,0));
  FXColor numbaseclr=getApp()->reg().readColorEntry("SETTINGS","numbasecolor",FXRGB(203,203,203));
  FXColor angmodeclr=getApp()->reg().readColorEntry("SETTINGS","anglemodecolor",FXRGB(203,203,203));

  // Number base
  FXint nbase=getApp()->reg().readIntEntry("SETTINGS","base",NUM_DEC);

  // Angle type
  FXint amode=getApp()->reg().readIntEntry("SETTINGS","angles",ANG_RAD);

  // Exponent mode
  FXint expmode=getApp()->reg().readIntEntry("SETTINGS","exponent",EXPONENT_IFNEEDED);

  // Engineering mode
  FXint engmode=getApp()->reg().readBoolEntry("SETTINGS","engineering",false);

  // Force decimal point
  FXint forcedec=getApp()->reg().readBoolEntry("SETTINGS","forcedecimal",false);

  // Precision
  FXint prec=getApp()->reg().readIntEntry("SETTINGS","precision",10);

  // Beep
  FXbool noise=getApp()->reg().readIntEntry("SETTINGS","beep",true);

  // Memory cell
  recall=getApp()->reg().readRealEntry("SETTINGS","memory",0.0);

  // Display Font
  fontspec=getApp()->reg().readStringEntry("SETTINGS","displayfont","");
  if(!fontspec.empty()){
    displayFont=new FXFont(getApp(),fontspec);
    setDisplayFont(displayFont);
    }

  // Digit Font
  fontspec=getApp()->reg().readStringEntry("SETTINGS","modefont","");
  if(!fontspec.empty()){
    modeFont=new FXFont(getApp(),fontspec);
    setModeFont(modeFont);
    }

  // Display Font
  fontspec=getApp()->reg().readStringEntry("SETTINGS","operatorfont","");
  if(!fontspec.empty()){
    operatorFont=new FXFont(getApp(),fontspec);
    setOperatorFont(operatorFont);
    }

  // Set colors
  setDigitColor(digitclr);
  setHexDigitColor(hexdigitclr);
  setOperatorColor(operatorclr);
  setFunctionColor(functionclr);
  setMemoryColor(memoryclr);
  setInverseColor(inverseclr);
  setHyperColor(hyperclr);
  setClearColor(clearclr);
  setClearAllColor(clearallclr);
  setDisplayColor(displayclr);
  setDisplayNumberColor(numberclr);
  setBaseColor(numbaseclr);
  setAngleColor(angmodeclr);

  // Number base
  setBase(nbase);
  setAngles(amode);
  setPrecision(prec);
  setExponentMode(expmode);
  setEngineeringMode(engmode);
  setDecimalPoint(forcedec);
  setBeep(noise);

  // Placement
  setX(xx);
  setY(yy);
  setWidth(ww);
  setHeight(hh);
  }

/*******************************************************************************/

// Write registry
void Calculator::writeRegistry(){
  FXString fontspec;

  // Position
  getApp()->reg().writeIntEntry("SETTINGS","x",getX());
  getApp()->reg().writeIntEntry("SETTINGS","y",getY());
  getApp()->reg().writeIntEntry("SETTINGS","w",getWidth());
  getApp()->reg().writeIntEntry("SETTINGS","h",getHeight());

  // Write colors
  getApp()->reg().writeColorEntry("SETTINGS","digitcolor",getDigitColor());
  getApp()->reg().writeColorEntry("SETTINGS","hexdigitcolor",getHexDigitColor());
  getApp()->reg().writeColorEntry("SETTINGS","operatorcolor",getOperatorColor());
  getApp()->reg().writeColorEntry("SETTINGS","functioncolor",getFunctionColor());
  getApp()->reg().writeColorEntry("SETTINGS","memorycolor",getMemoryColor());
  getApp()->reg().writeColorEntry("SETTINGS","inversecolor",getInverseColor());
  getApp()->reg().writeColorEntry("SETTINGS","hypercolor",getHyperColor());
  getApp()->reg().writeColorEntry("SETTINGS","clearcolor",getClearColor());
  getApp()->reg().writeColorEntry("SETTINGS","clearallcolor",getClearAllColor());
  getApp()->reg().writeColorEntry("SETTINGS","displaycolor",getDisplayColor());
  getApp()->reg().writeColorEntry("SETTINGS","displaynumbercolor",getDisplayNumberColor());
  getApp()->reg().writeColorEntry("SETTINGS","numbasecolor",getBaseColor());
  getApp()->reg().writeColorEntry("SETTINGS","anglemodecolor",getAngleColor());

  // Number base
  getApp()->reg().writeIntEntry("SETTINGS","base",getBase());

  // Angle type
  getApp()->reg().writeIntEntry("SETTINGS","angles",getAngles());

  // Exponent mode
  getApp()->reg().writeIntEntry("SETTINGS","exponent",getExponentMode());

  // Engineering mode
  getApp()->reg().writeIntEntry("SETTINGS","engineering",getEngineeringMode());

  // Force decimal point
  getApp()->reg().writeIntEntry("SETTINGS","forcedecimal",getDecimalPoint());

  // Precision
  getApp()->reg().writeIntEntry("SETTINGS","precision",getPrecision());

  // Beep
  getApp()->reg().writeIntEntry("SETTINGS","beep",getBeep());

  // Memory contents
  getApp()->reg().writeRealEntry("SETTINGS","memory",recall);

  // Display Font
  fontspec=getDisplayFont()->getFont();
  getApp()->reg().writeStringEntry("SETTINGS","displayfont",fontspec.text());

  // Digit Font
  fontspec=getModeFont()->getFont();
  getApp()->reg().writeStringEntry("SETTINGS","modefont",fontspec.text());

  // Operator Font
  fontspec=getOperatorFont()->getFont();
  getApp()->reg().writeStringEntry("SETTINGS","operatorfont",fontspec.text());
  }

/*******************************************************************************/

// Get display text
FXString Calculator::getDisplayText() const {
  return display->getText();
  }


// Display text
void Calculator::setDisplayText(const FXString& txt){
  display->setText(txt);
  }


// Get displayed value
FXdouble Calculator::getDisplayValue() const {
  FXdouble val;
  if(base==10)
    val=getDisplayText().toDouble();
  else
    val=(FXdouble)getDisplayText().toLong(base);
  return val;
  }


// Redisplay new value
void Calculator::setDisplayValue(FXdouble val){
  FXString string;
  if(Math::fpNan(val)){
    setDisplayText("ERROR");
    if(beep) getApp()->beep();
    }
  else if(Math::fpInfinite(val)){
    if(Math::fpSign(val)){
      setDisplayText("-\xE2\x88\x9E");  // "-INF"
      }
    else{
      setDisplayText("+\xE2\x88\x9E");  // "+INF"
      }
    if(beep) getApp()->beep();
    }
  else if(base==10){
    if(val==0.0) val=0.0;       // Don't ever print out -0 instead of 0
    setDisplayText(string.fromDouble(val,precision,exponent));
    }
  else{
    //setDisplayText(string.fromLong((FXlong)((0.0<=val)?val+0.5:val-0.5),base));
    setDisplayText(string.fromLong(Math::lrint(val),base));
    }
  }


/*******************************************************************************/

// Push to number stack
FXdouble Calculator::pushnum(FXdouble num){
  FXASSERT(numsp<STACKLEN);
  numstack[++numsp]=num;
  return num;
  }


// Replace number on top of stack
FXdouble Calculator::setnum(FXdouble num){
  FXASSERT(0<=numsp && numsp<STACKLEN);
  numstack[numsp]=num;
  return num;
  }


// Pop number of stack
FXdouble Calculator::popnum(){
  FXASSERT(0<=numsp);
  return numstack[numsp--];
  }


// Return number on top of stack
FXdouble Calculator::getnum(){
  FXASSERT(0<=numsp && numsp<STACKLEN);
  return numstack[numsp];
  }


// Set number base
void Calculator::setBase(FXint b){
  switch(b){
    case  2: base=2;  limit=BINARY_LIMIT; break;
    case  8: base=8;  limit=OCTAL_LIMIT; break;
    case 16: base=16; limit=HEXADECIMAL_LIMIT; break;
    default: base=10; limit=DECIMAL_LIMIT; break;
    }
  setDisplayValue(getnum());
  modifiers=0;
  }


// Set exponent mode
void Calculator::setExponentMode(FXuchar expmode){
  exponent=(expmode&3)|(exponent&12);
  setDisplayValue(getnum());
  modifiers=0;
  }


// Set exponent mode
void Calculator::setEngineeringMode(FXbool engmode){
  exponent^=((0-engmode)^exponent)&4;
  setDisplayValue(getnum());
  modifiers=0;
  }


// Force decimal point
void Calculator::setDecimalPoint(FXbool forcedec){
  exponent^=((0-forcedec)^exponent)&8;
  setDisplayValue(getnum());
  modifiers=0;
  }


// Set precision
void Calculator::setPrecision(FXint prec){
  precision=prec;
  setDisplayValue(getnum());
  modifiers=0;
  }


// Argument to sine, cosine, etc
FXdouble Calculator::trigarg(FXdouble a) const {
  switch(angles){
    case ANG_DEG: return DEG2RAD(a);
    case ANG_GRA: return GRA2RAD(a);
    case ANG_RAD: return a;
    }
  return a;
  }


// Result from arcsine, arccosine, etc
FXdouble Calculator::trigres(FXdouble r) const {
  switch(angles){
    case ANG_DEG: return RAD2DEG(r);
    case ANG_GRA: return RAD2GRA(r);
    case ANG_RAD: return r;
    }
  return r;
  }


// Factorial
//
// result = n!
//
static FXdouble factorial(FXdouble n){
  FXdouble num=Math::lrint(n);
  FXdouble result=1.0;
  if(0.0<=num && num==n){
    while(num>0.0){
      if(!Math::fpFinite(result)) break;
      result=result*num;
      num=num-1.0;
      }
    return result;
    }
  return dblnan.f;
  }


// Permutations
//
//             n!
// result =  ------
//            (n-r)!
//
static FXdouble permutations(FXdouble n,FXdouble r){
  FXdouble num=Math::lrint(n);
  FXdouble den=Math::lrint(r);
  FXdouble result=1.0;
  if(0.0<=num && 0.0<=den && den<=num && num==n && den==r){
    while(den>0.0){
      if(!Math::fpFinite(result)) break;
      result=result*num;
      num=num-1.0;
      den=den-1.0;
      }
    return result;
    }
  return dblnan.f;
  }


// Combinations
//
//               n!
// result =  ----------
//            r! (n-r)!
//
static FXdouble combinations(FXdouble n,FXdouble r){
  FXdouble num=Math::lrint(n);
  FXdouble den=Math::lrint(r);
  FXdouble res1=1.0;
  FXdouble res2=1.0;
  if(0.0<=num && 0.0<=den && den<=num && num==n && den==r){
    while(den>0.0){
      if(!Math::fpFinite(res1)) break;
      res1=res1*num;
      res2=res2*den;
      num=num-1.0;
      den=den-1.0;
      }
    return res1/res2;
    }
  return dblnan.f;
  }


// Reset calculator
void Calculator::clearAll(){
  setDisplayValue(0.0);
  numstack[0]=0.0;
  numsp=0;
  opsp=-1;
  parens=0;
  modifiers=0;
  }


// Clear calculator
void Calculator::clear(){
  setDisplayValue(0.0);
  setnum(0.0);
  modifiers=0;
  }


// Perform unary operator
void Calculator::unary(FXuchar op){
  FXdouble acc,val;
  FXASSERT(0<=numsp);
  val=getnum();
  acc=0.0;
  switch(op){
    case UN_NOT:
      acc=(FXdouble) (~((FXlong)Math::lrint(val)));
      break;
    case UN_NEG:
      acc=-val;
      break;
    case UN_SHL:
      acc=(FXdouble) (((FXulong)Math::lrint(val))<<1);
      break;
    case UN_SHR:
      acc=(FXdouble) (((FXulong)Math::lrint(val))>>1);
      break;
    case UN_SAR:
      acc=(FXdouble) (((FXlong)Math::lrint(val))>>1);
      break;
    case UN_RECIP:
      acc=1.0/val;
      break;
    case UN_FAC:
      acc=factorial(val);
      break;
    case UN_SQRT:
      acc=Math::sqrt(val);
      break;
    case UN_SQR:
      acc=Math::sqr(val);
      break;
    case UN_CBRT:
      acc=Math::cbrt(val);
      break;
    case UN_CUB:
      acc=Math::cub(val);
      break;
    case UN_2LOG:
      acc=Math::log2(val);
      break;
    case UN_2TOX:
      acc=Math::exp2(val);
      break;
    case UN_LOG:
      acc=Math::log10(val);
      break;
    case UN_10TOX:
      acc=Math::exp10(val);
      break;
    case UN_LN:
      acc=Math::log(val);
      break;
    case UN_EXP:
      acc=Math::exp(val);
      break;
    case UN_SIN:
      acc=Math::sin(trigarg(val));
      break;
    case UN_COS:
      acc=Math::cos(trigarg(val));
      break;
    case UN_TAN:
      acc=Math::tan(trigarg(val));
      break;
    case UN_ASIN:
      acc=trigres(Math::asin(val));
      break;
    case UN_ACOS:
      acc=trigres(Math::acos(val));
      break;
    case UN_ATAN:
      acc=trigres(Math::atan(val));
      break;
    case UN_SINH:
      acc=Math::sinh(val);
      break;
    case UN_COSH:
      acc=Math::cosh(val);
      break;
    case UN_TANH:
      acc=Math::tanh(val);
      break;
    case UN_ASINH:
      acc=Math::asinh(val);
      break;
    case UN_ACOSH:
      acc=Math::acosh(val);
      break;
    case UN_ATANH:
      acc=Math::atanh(val);
      break;
    case UN_CEIL:
      acc=Math::ceil(val);
      break;
    case UN_FLOOR:
      acc=Math::floor(val);
      break;
    default:
      break;
    }
  setnum(acc);
  setDisplayValue(acc);
  modifiers=0;
  }


// Perform operator
void Calculator::dyop(FXuchar op){
  FXdouble acc,val;
  FXASSERT(0<=numsp);
  val=popnum();
  FXASSERT(0<=numsp);
  acc=getnum();
  switch(op){
    case DY_OR:
      acc=(FXdouble) (((FXulong)Math::lrint(acc)) | ((FXulong)Math::lrint(val)));
      break;
    case DY_XOR:
      acc=(FXdouble) (((FXulong)Math::lrint(acc)) ^ ((FXulong)Math::lrint(val)));
      break;
    case DY_AND:
      acc=(FXdouble) (((FXulong)Math::lrint(acc)) & ((FXulong)Math::lrint(val)));
      break;
    case DY_SUB:
      acc=acc-val;
      break;
    case DY_ADD:
      acc=acc+val;
      break;
    case DY_MOD:                // Theo Veenker <Theo.Veenker@let.uu.nl> suggested this new definition of "mod":
      //val=Math::fabs(val);      // x = a div |b|        ; with a round toward 0
      //acc=Math::fmod(acc,val);  // y = a mod |b|
      //break;                    // a = x * |b| + y
      acc=(FXdouble) (((FXlong)Math::lrint(acc)) % ((FXlong)Math::lrint(val)));
      break;
    case DY_IDIV:
      //modf(acc/val,&acc);
      acc=(FXdouble) (((FXlong)Math::lrint(acc)) / ((FXlong)Math::lrint(val)));
      break;
    case DY_DIV:
      acc=acc/val;
      break;
    case DY_MUL:
      acc=acc*val;
      break;
    case DY_XTOY:
      acc=Math::pow(acc,val);
      break;
    case DY_XTOINVY:
      acc=Math::pow(acc,1.0/val);
      break;
    case DY_PER:
      acc=permutations(acc,val);
      break;
    case DY_COM:
      acc=combinations(acc,val);
    default:
      break;
    }
  setnum(acc);
  setDisplayValue(acc);
  modifiers=0;
  }


// Enter operator
void Calculator::dyadic(FXuchar op){
  if(opsp<0 || opstack[opsp]==DY_LPAR || priority[op]>priority[opstack[opsp]]){
    pushnum(getnum());
    opstack[++opsp]=op;
    }
  else{
    dyop(opstack[opsp]);
    pushnum(getnum());
    opstack[opsp]=op;
    }
  modifiers=0;
  }


// Enter evaluate
void Calculator::evaluate(){
  FXuchar op;
  while(0<=opsp){
    op=opstack[opsp--];
    if(op!=DY_LPAR)
      dyop(op);
    else
      parens--;
    }
  setDisplayValue(getnum());
  modifiers=0;
  }


// Left parentheses
void Calculator::lparen(){
  opstack[++opsp]=DY_LPAR;
  setnum(0.0);
  setDisplayValue(0.0);
  parens++;
  modifiers=0;
  }


// Right parentheses
void Calculator::rparen(){
  FXuchar op;
  while(0<=opsp){
    op=opstack[opsp--];
    if(op==DY_LPAR){ parens--; break; }
    dyop(op);
    }
  setDisplayValue(getnum());
  modifiers=0;
  }


/*******************************************************************************/

// Close the window and save registry
FXbool Calculator::close(FXbool notify){
  writeRegistry();
  return FXMainWindow::close(notify);
  }


// Change preferences
long Calculator::onCmdPreferences(FXObject*,FXSelector,void*){
  Preferences preferences(this);
  preferences.setX(getX()+80);
  preferences.setY(getY()+80);
  preferences.execute(PLACEMENT_DEFAULT);
  return 1;
  }


// Change colors
long Calculator::onCmdColor(FXObject*,FXSelector sel,void* ptr){
  FXColor clr=(FXColor)(FXuval)ptr;
  switch(FXSELID(sel)){
    case ID_COLOR_DISPLAY: setDisplayColor(clr); break;
    case ID_COLOR_DISPLAYNUMBER: setDisplayNumberColor(clr); break;
    case ID_COLOR_DIGITS: setDigitColor(clr); break;
    case ID_COLOR_HEXDIGITS: setHexDigitColor(clr); break;
    case ID_COLOR_OPERATORS: setOperatorColor(clr); break;
    case ID_COLOR_FUNCTIONS: setFunctionColor(clr); break;
    case ID_COLOR_MEMORY: setMemoryColor(clr); break;
    case ID_COLOR_BASE: setBaseColor(clr); break;
    case ID_COLOR_ANGLES: setAngleColor(clr); break;
    case ID_COLOR_INVERT: setInverseColor(clr); break;
    case ID_COLOR_HYPER: setHyperColor(clr); break;
    case ID_COLOR_CLEARALL: setClearAllColor(clr); break;
    case ID_COLOR_CLEAR: setClearColor(clr); break;
    }
  return 1;
  }


// Update colors
long Calculator::onUpdColor(FXObject* sender,FXSelector sel,void*){
  FXColor clr;
  switch(FXSELID(sel)){
    case ID_COLOR_DISPLAY: clr=getDisplayColor(); break;
    case ID_COLOR_DISPLAYNUMBER: clr=getDisplayNumberColor(); break;
    case ID_COLOR_DIGITS: clr=getDigitColor(); break;
    case ID_COLOR_HEXDIGITS: clr=getHexDigitColor(); break;
    case ID_COLOR_OPERATORS: clr=getOperatorColor(); break;
    case ID_COLOR_FUNCTIONS: clr=getFunctionColor(); break;
    case ID_COLOR_MEMORY: clr=getMemoryColor(); break;
    case ID_COLOR_BASE: clr=getBaseColor(); break;
    case ID_COLOR_ANGLES: clr=getAngleColor(); break;
    case ID_COLOR_INVERT: clr=getInverseColor(); break;
    case ID_COLOR_HYPER: clr=getHyperColor(); break;
    case ID_COLOR_CLEARALL: clr=getClearAllColor(); break;
    case ID_COLOR_CLEAR: clr=getClearColor(); break;
    }
  sender->handle(this,FXSEL(SEL_COMMAND,ID_SETINTVALUE),(void*)&clr);
  return 1;
  }


// Change font
long Calculator::onCmdDisplayFont(FXObject*,FXSelector,void*){
  FXFontDialog fontdlg(this,"Change Display Font",DECOR_BORDER|DECOR_TITLE);
  FXFontDesc fontdesc=getDisplayFont()->getFontDesc();
  fontdlg.setFontDesc(fontdesc);
  if(fontdlg.execute()){
    FXFont *oldfont=displayFont;
    fontdesc=fontdlg.getFontDesc();
    displayFont=new FXFont(getApp(),fontdesc);
    displayFont->create();
    setDisplayFont(displayFont);
    delete oldfont;
    }
  return 1;
  }


// Change mode button font
long Calculator::onCmdModeFont(FXObject*,FXSelector,void*){
  FXFontDialog fontdlg(this,"Change Mode Button Font",DECOR_BORDER|DECOR_TITLE);
  FXFontDesc fontdesc=getModeFont()->getFontDesc();
  fontdlg.setFontDesc(fontdesc);
  if(fontdlg.execute()){
    FXFont *oldfont=modeFont;
    fontdesc=fontdlg.getFontDesc();
    modeFont=new FXFont(getApp(),fontdesc);
    modeFont->create();
    setModeFont(modeFont);
    delete oldfont;
    }
  return 1;
  }


// Change operator button font
long Calculator::onCmdOperatorFont(FXObject*,FXSelector,void*){
  FXFontDialog fontdlg(this,"Change Operator Button Font",DECOR_BORDER|DECOR_TITLE);
  FXFontDesc fontdesc=getOperatorFont()->getFontDesc();
  fontdlg.setFontDesc(fontdesc);
  if(fontdlg.execute()){
    FXFont *oldfont=operatorFont;
    fontdesc=fontdlg.getFontDesc();
    operatorFont=new FXFont(getApp(),fontdesc);
    operatorFont->create();
    setOperatorFont(operatorFont);
    delete oldfont;
    }
  return 1;
  }

/*******************************************************************************/

// Copy value
long Calculator::onCmdClipboardCopy(FXObject*,FXSelector,void*){
  FXDragType types[4]={stringType,textType,utf8Type};
  if(acquireClipboard(types,ARRAYNUMBER(types))){
    clipped.fromDouble(numstack[0],precision,exponent&3);
    }
  return 1;
  }


// Copy value
long Calculator::onCmdClipboardPaste(FXObject*,FXSelector,void*){
  FXString string;
  FXdouble num;
  if(getDNDData(FROM_SELECTION,utf8Type,string) || getDNDData(FROM_SELECTION,textType,string) || getDNDData(FROM_SELECTION,stringType,string)){
    if(string.scan("%'lf",&num)==1){
      setnum(num);
      setDisplayValue(num);
      return 1;
      }
    }
  getApp()->beep();
  return 1;
  }


// We now really do have the clipboard, keep clipped text
long Calculator::onClipboardGained(FXObject*,FXSelector,void*){
  return 1;
  }


// We lost the clipboard, free clipped text
long Calculator::onClipboardLost(FXObject*,FXSelector,void*){
  clipped.clear();
  return 1;
  }


// Somebody wants our clipped text
long Calculator::onClipboardRequest(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  if(event->target==stringType || event->target==textType || event->target==utf8Type){
    setDNDData(FROM_CLIPBOARD,event->target,clipped);
    return 1;
    }
  return 0;
  }

/*******************************************************************************/

// Change exponential notation
long Calculator::onCmdExponent(FXObject*,FXSelector sel,void* ptr){
  if((FXSELID(sel)==ID_EXPONENT_ALWAYS) && ptr) setExponentMode(EXPONENT_ALWAYS);
  else if((FXSELID(sel)==ID_EXPONENT_NEVER) && ptr) setExponentMode(EXPONENT_NEVER);
  else setExponentMode(EXPONENT_IFNEEDED);
  return 1;
  }


// Update exponential notation
long Calculator::onUpdExponent(FXObject* sender,FXSelector sel,void*){
  if(FXSELID(sel)==ID_EXPONENT_ALWAYS && getExponentMode()==EXPONENT_ALWAYS)
    sender->handle(this,FXSEL(SEL_COMMAND,ID_CHECK),nullptr);
  else if(FXSELID(sel)==ID_EXPONENT_NEVER && getExponentMode()==EXPONENT_NEVER)
    sender->handle(this,FXSEL(SEL_COMMAND,ID_CHECK),nullptr);
  else
    sender->handle(this,FXSEL(SEL_COMMAND,ID_UNCHECK),nullptr);
  return 1;
  }


// Change engineering notation
long Calculator::onCmdEngineeringMode(FXObject*,FXSelector,void*){
  setEngineeringMode(!getEngineeringMode());
  return 1;
  }


// Update exponential notation
long Calculator::onUpdEngineeringMode(FXObject* sender,FXSelector,void*){
  if(getEngineeringMode())
    sender->handle(this,FXSEL(SEL_COMMAND,ID_CHECK),nullptr);
  else
    sender->handle(this,FXSEL(SEL_COMMAND,ID_UNCHECK),nullptr);
  return 1;
  }


// Force decimal point
long Calculator::onCmdDecimalPoint(FXObject*,FXSelector,void*){
  setDecimalPoint(!getDecimalPoint());
  return 1;
  }


// Update force decimal point
long Calculator::onUpdDecimalPoint(FXObject* sender,FXSelector,void*){
  if(getDecimalPoint())
    sender->handle(this,FXSEL(SEL_COMMAND,ID_CHECK),nullptr);
  else
    sender->handle(this,FXSEL(SEL_COMMAND,ID_UNCHECK),nullptr);
  return 1;
  }


// Change precision
long Calculator::onCmdPrecision(FXObject* sender,FXSelector,void*){
  FXint prec=16;
  sender->handle(this,FXSEL(SEL_COMMAND,ID_GETINTVALUE),(void*)&prec);
  setPrecision(prec);
  return 1;
  }


// Update precision
long Calculator::onUpdPrecision(FXObject* sender,FXSelector,void*){
  sender->handle(this,FXSEL(SEL_COMMAND,ID_SETINTVALUE),(void*)&precision);
  return 1;
  }


// Change beep mode
long Calculator::onCmdBeep(FXObject*,FXSelector,void*){
  beep=!beep;
  return 1;
  }


// Update beep mode
long Calculator::onUpdBeep(FXObject* sender,FXSelector,void*){
  sender->handle(this,beep ? FXSEL(SEL_COMMAND,ID_CHECK) : FXSEL(SEL_COMMAND,ID_UNCHECK), nullptr);
  return 1;
  }


// Popup help
long Calculator::onCmdQuestion(FXObject*,FXSelector,void*){
  HelpWindow helpwindow(this,"Calculator Help");
  helpwindow.setHelp(help);
  helpwindow.setX(getX()+80);
  helpwindow.setY(getY()+80);
  helpwindow.execute(PLACEMENT_DEFAULT);
  return 1;
  }


// Change angle mode
long Calculator::onCmdAngle(FXObject*,FXSelector sel,void*){
  angles=(FXSELID(sel)-ID_MODE);
  return 1;
  }


// Update radio button for angle mode
long Calculator::onUpdAngle(FXObject* sender,FXSelector sel,void*){
  sender->handle(this,angles==(FXSELID(sel)-ID_MODE) ? FXSEL(SEL_COMMAND,ID_CHECK) : FXSEL(SEL_COMMAND,ID_UNCHECK), nullptr);
  return 1;
  }


// Change angle mode
long Calculator::onCmdBase(FXObject*,FXSelector sel,void*){
  setBase(FXSELID(sel)-ID_BASE);
  return 1;
  }


// Update radio button for angle mode
long Calculator::onUpdBase(FXObject* sender,FXSelector sel,void*){
  sender->handle(this,base==(FXSELID(sel)-ID_BASE) ? FXSEL(SEL_COMMAND,ID_CHECK) : FXSEL(SEL_COMMAND,ID_UNCHECK), nullptr);
  return 1;
  }


// Physical constant names
static const FXchar *const physicslabel[6][4]={
  {"c","h",FXString::null,FXString::null},
  {"\xCE\xB5\xE2\x82\x80","\xCE\xBC\xE2\x82\x80","\xCE\x96\xE2\x82\x80","G"},
  {"eV","e",FXString::null,FXString::null},
  {"F","NA",FXString::null,FXString::null},
  {"k","Rgas",FXString::null,FXString::null},
  {"Me","Mu",FXString::null,FXString::null},
  };


// Physical constant tooltips
static const FXchar *const physicstip[6][4]={
  {"Lightspeed (m/s)","Plank (J*s)",FXString::null,FXString::null},
  {"Vacuum permittivity (F/m)","Vacuum permeability (H/m)","Vacuum impedance (Ohm)","Gravity (m^3/(kg*s^2))"},
  {"Electron volt (J)","Charge electron (C)",FXString::null,FXString::null},
  {"Faraday (C/mol)","Avogadro (1/mol)",FXString::null,FXString::null},
  {"Boltzman (J/K)","Rgas (J/(mol*K))",FXString::null,FXString::null},
  {"Mass electron (kg)","Molar mass (kg)",FXString::null,FXString::null},
  };


// Physical constant names values
static const FXdouble physicsconst[6][4]={
  {LIGHTSPEED,PLANCK,0.0,0.0},
  {EPS0,MU0,ZETA0,GRAVITY},
  {ELECTRONVOLT,ELECTRONCHARGE,0.0,0.0},
  {FARADAY,AVOGADRO,0.0,0.0},
  {BOLTZMAN,RGAS,0.0,0.0},
  {ELECTRONMASS,PROTONMASS,0.0,0.0},
  };


// Update digits based on base
long Calculator::onUpdHexDigit(FXObject* sender,FXSelector sel,void* ptr){
  if(base==10){
    FXint d=FXSELID(sel)-ID_0;
    FXint m=modifiers&(MOD_INV|MOD_HYP);
    digit[d]->setText(physicslabel[d-10][m]);
    digit[d]->setTipText(physicstip[d-10][m]);
    if(physicslabel[d-10][m]!=FXString::null)
      digit[d]->enable();
    else
      digit[d]->disable();
    return 1;
    }
  return onUpdDigit(sender,sel,ptr);
  }


// Enter digits into display
long Calculator::onCmdHexDigit(FXObject* sender,FXSelector sel,void* ptr){
  if(base==10){
    FXint d=FXSELID(sel)-ID_A;
    FXint m=modifiers&(MOD_INV|MOD_HYP);
    setnum(physicsconst[d][m]);
    setDisplayValue(getnum());
    modifiers=0;
    return 1;
    }
  return onCmdDigit(sender,sel,ptr);
  }


// Regular digit labels
static const FXchar *const digitlabel[16]={
  "0","1","2","3","4","5","6","7","8","9","A","B","C","D","E","F"
  };


// Update digits based on base
long Calculator::onUpdDigit(FXObject*,FXSelector sel,void*){
  FXint d=FXSELID(sel)-ID_0;
  digit[d]->setText(digitlabel[d]);
  if(d<base)
    digit[d]->enable();
  else
    digit[d]->disable();
  return 1;
  }


// Enter digit into display
long Calculator::onCmdDigit(FXObject*,FXSelector sel,void*){
  FXString text=getDisplayText();
  FXint pos;
  if(!(modifiers&MOD_ENT)){ text=""; digits=0; }
  if((base==10) && (pos=text.find('E'))>=0){
    pos++;                                          // Skip 'E'
    if(text[pos]=='-' || text[pos]=='+') pos++;     // Skip sign
    if(text[pos]=='0' || (text[pos] && text[pos+1] && text[pos+2])){
      while(text[pos+1]){ text[pos]=text[pos+1]; pos++; }
      text[pos]=Ascii::valueDigit(FXSELID(sel)-ID_0);
      }
    else{
      text.append(Ascii::valueDigit(FXSELID(sel)-ID_0));
      }
    }
  else if(digits<limit){
    text+=Ascii::valueDigit(FXSELID(sel)-ID_0);
    digits++;
    }
  setDisplayText(text);
  setnum(getDisplayValue());
  modifiers|=MOD_ENT;
  return 1;
  }


// Decimal point
long Calculator::onCmdPoint(FXObject*,FXSelector,void*){
  FXString text=getDisplayText();
  if(!(modifiers&MOD_ENT)){ text="0"; digits=1; }
  if(base==10 && text.find('.')<0 && text.find('E')<0) text+='.';
  setDisplayText(text);
  setnum(getDisplayValue());
  modifiers|=MOD_ENT;
  return 1;
  }


// Update decimal point
long Calculator::onUpdPoint(FXObject* sender,FXSelector,void*){
  sender->handle(this,(base==10) ? FXSEL(SEL_COMMAND,ID_ENABLE) : FXSEL(SEL_COMMAND,ID_DISABLE),nullptr);
  return 1;
  }


// Exponent
long Calculator::onCmdExp(FXObject*,FXSelector,void*){
  FXString text=getDisplayText();
  if(!(modifiers&MOD_ENT)){ text="0"; digits=1; }
  if(base==10 && text.find('E')<0) text+="E+0";
  setDisplayText(text);
  setnum(getDisplayValue());
  modifiers|=MOD_ENT;
  return 1;
  }


// Update exponent
long Calculator::onUpdExp(FXObject* sender,FXSelector,void*){
  sender->handle(this,(base==10) ? FXSEL(SEL_COMMAND,ID_ENABLE) : FXSEL(SEL_COMMAND,ID_DISABLE),nullptr);
  return 1;
  }


// Plus minus +/-
long Calculator::onCmdPlusMin(FXObject*,FXSelector,void*){
  FXString text=getDisplayText();
  FXint pos;
  if(modifiers&MOD_ENT){
    if((base==10) && (pos=text.find('E'))>=0){
      if(text[pos+1]=='+') text[pos+1]='-';
      else if(text[pos+1]=='-') text[pos+1]='+';
      else text.insert(pos+1,'-');
      }
    else{
      if(text[0]=='-') text.erase(0);
      else if(text[0]=='+') text[0]='-';
      else if(text!="0") text.prepend('-');
      }
    setDisplayText(text);
    setnum(getDisplayValue());
    }
  else{
    unary(UN_NEG);
    }
  return 1;
  }


// Delete last character
long Calculator::onCmdDelete(FXObject*,FXSelector,void*){
  FXString text=getDisplayText();
  FXint len;
  if(modifiers&MOD_ENT){
    len=text.length();
    if(0<len){
      if(base==10 && text.find('E')>=0){
        len--;
        if(0<len && (text[len-1]=='+' || text[len-1]=='-')) len--;
        if(0<len && text[len-1]=='E') len--;
        }
      else{
        len--;
        if(Ascii::isDigit(text[len])) digits--;
        if(0<len && (text[len-1]=='+' || text[len-1]=='-')) len--;
        if(0<len && text[len-1]=='.') len--;
        }
      text.trunc(len);
      }
    if(len<=0){
      text="0";
      modifiers&=~MOD_ENT;
      digits=1;
      }
    setDisplayText(text);
    setnum(getDisplayValue());
    }
  else{
    clear();
    }
  return 1;
  }


// Clear entry
long Calculator::onCmdClear(FXObject*,FXSelector,void*){
  clear();
  return 1;
  }


// Clear entry
long Calculator::onCmdClearAll(FXObject*,FXSelector,void*){
  clearAll();
  return 1;
  }


// Inverse
long Calculator::onCmdInverse(FXObject*,FXSelector,void*){
  modifiers^=MOD_INV;
  return 1;
  }


// Hyper
long Calculator::onCmdHyper(FXObject*,FXSelector,void*){
  modifiers^=MOD_HYP;
  return 1;
  }


// Sine button
long Calculator::onCmdSin(FXObject*,FXSelector,void*){
  unary(UN_SIN+(modifiers&(MOD_INV|MOD_HYP)));
  return 1;
  }


// Update sine button
long Calculator::onUpdSin(FXObject* sender,FXSelector,void*){
  FXString label="sin";
  if(modifiers&MOD_INV) label.prepend('a');
  if(modifiers&MOD_HYP) label.append('h');
  sender->handle(this,FXSEL(SEL_COMMAND,ID_SETSTRINGVALUE),(void*)&label);
  sender->handle(this,(base==10) ? FXSEL(SEL_COMMAND,ID_ENABLE) : FXSEL(SEL_COMMAND,ID_DISABLE),nullptr);
  return 1;
  }


// Cosine button
long Calculator::onCmdCos(FXObject*,FXSelector,void*){
  unary(UN_COS+(modifiers&(MOD_INV|MOD_HYP)));
  return 1;
  }


// Update cosine button
long Calculator::onUpdCos(FXObject* sender,FXSelector,void*){
  FXString label="cos";
  if(modifiers&MOD_INV) label.prepend('a');
  if(modifiers&MOD_HYP) label.append('h');
  sender->handle(this,FXSEL(SEL_COMMAND,ID_SETSTRINGVALUE),(void*)&label);
  sender->handle(this,(base==10) ? FXSEL(SEL_COMMAND,ID_ENABLE) : FXSEL(SEL_COMMAND,ID_DISABLE),nullptr);
  return 1;
  }


// Tangent button
long Calculator::onCmdTan(FXObject*,FXSelector,void*){
  unary(UN_TAN+(modifiers&(MOD_INV|MOD_HYP)));
  return 1;
  }


// Update tangent button
long Calculator::onUpdTan(FXObject* sender,FXSelector,void*){
  FXString label="tan";
  if(modifiers&MOD_INV) label.prepend('a');
  if(modifiers&MOD_HYP) label.append('h');
  sender->handle(this,FXSEL(SEL_COMMAND,ID_SETSTRINGVALUE),(void*)&label);
  sender->handle(this,(base==10) ? FXSEL(SEL_COMMAND,ID_ENABLE) : FXSEL(SEL_COMMAND,ID_DISABLE),nullptr);
  return 1;
  }


// Log button
long Calculator::onCmdLog(FXObject*,FXSelector,void*){
  unary(UN_LOG+(modifiers&MOD_INV));
  return 1;
  }


// Update Log button
long Calculator::onUpdLog(FXObject* sender,FXSelector,void*){
  FXString label=(modifiers&MOD_INV)?"10^x":"log";
  sender->handle(this,FXSEL(SEL_COMMAND,ID_SETSTRINGVALUE),(void*)&label);
  sender->handle(this,(base==10) ? FXSEL(SEL_COMMAND,ID_ENABLE) : FXSEL(SEL_COMMAND,ID_DISABLE),nullptr);
  return 1;
  }


// Ln button
long Calculator::onCmdLn(FXObject*,FXSelector,void*){
  unary(UN_LN+(modifiers&MOD_INV));
  return 1;
  }


// Update Ln button
long Calculator::onUpdLn(FXObject* sender,FXSelector,void*){
  FXString label=(modifiers&MOD_INV)?"e^x":"ln";
  sender->handle(this,FXSEL(SEL_COMMAND,ID_SETSTRINGVALUE),(void*)&label);
  sender->handle(this,(base==10) ? FXSEL(SEL_COMMAND,ID_ENABLE) : FXSEL(SEL_COMMAND,ID_DISABLE),nullptr);
  return 1;
  }


// Update PI button
long Calculator::onCmdPi(FXObject*,FXSelector,void*){
  setnum((modifiers&MOD_HYP)?((modifiers&MOD_INV)?INVGOLDEN:GOLDEN):((modifiers&MOD_INV)?EULER:PI));
  setDisplayValue(getnum());
  modifiers=0;
  return 1;
  }


// Update PI button
long Calculator::onUpdPi(FXObject* sender,FXSelector,void*){
  FXString label=(modifiers&MOD_HYP) ? (modifiers&MOD_INV) ? "1/\xCF\x86" : "\xCF\x86" : (modifiers&MOD_INV) ? "e" : "\xCF\x80";
//FXString label=(modifiers&MOD_HYP) ? (modifiers&MOD_INV) ? "1/phi" : "phi" : (modifiers&MOD_INV) ? "e" : "pi";
  sender->handle(this,FXSEL(SEL_COMMAND,ID_SETSTRINGVALUE),(void*)&label);
  sender->handle(this,(base==10) ? FXSEL(SEL_COMMAND,ID_ENABLE) : FXSEL(SEL_COMMAND,ID_DISABLE),nullptr);
  return 1;
  }


// Factorial
long Calculator::onCmdFac(FXObject*,FXSelector,void*){
  unary(UN_FAC);
  return 1;
  }


// Update factorial
long Calculator::onUpdFac(FXObject* sender,FXSelector,void*){
  sender->handle(this,(base==10) ? FXSEL(SEL_COMMAND,ID_ENABLE) : FXSEL(SEL_COMMAND,ID_DISABLE),nullptr);
  return 1;
  }


// Permutations
long Calculator::onCmdPer(FXObject*,FXSelector,void*){
  if(modifiers&MOD_INV)
    unary(UN_CEIL);
  else
    dyadic(DY_PER);
  return 1;
  }


// Update permutations
long Calculator::onUpdPer(FXObject*,FXSelector,void*){
  if(modifiers&MOD_INV)
    functions[9]->setText("\xE2\x8C\x88x\xE2\x8C\x89"); // "\xE2\x8E\xA1x\xE2\x8E\xA4"
  else
    functions[9]->setText("nPr");
  if(base==10)
    functions[9]->enable();
  else
    functions[9]->disable();
  return 1;
  }


// Combinations
long Calculator::onCmdCom(FXObject*,FXSelector,void*){
  if(modifiers&MOD_INV)
    unary(UN_FLOOR);
  else
    dyadic(DY_COM);
  return 1;
  }


// Update combinations
long Calculator::onUpdCom(FXObject*,FXSelector,void*){
  if(modifiers&MOD_INV)
    functions[10]->setText("\xE2\x8C\x8Ax\xE2\x8C\x8B");// "\xE2\x8E\xA3x\xE2\x8E\xA6"
  else
    functions[10]->setText("nCr");
  if(base==10)
    functions[10]->enable();
  else
    functions[10]->disable();
  return 1;
  }


// Reciprocal
long Calculator::onCmdRecip(FXObject*,FXSelector,void*){
  unary(UN_RECIP);
  return 1;
  }


// Update reciprocal
long Calculator::onUpdRecip(FXObject* sender,FXSelector,void*){
  sender->handle(this,(base==10) ? FXSEL(SEL_COMMAND,ID_ENABLE) : FXSEL(SEL_COMMAND,ID_DISABLE),nullptr);
  return 1;
  }


// X ^ Y
long Calculator::onCmdXToY(FXObject*,FXSelector,void*){
  dyadic(DY_XTOY+(modifiers&MOD_INV));
  return 1;
  }


// Update X ^ Y
long Calculator::onUpdXToY(FXObject* sender,FXSelector,void*){
  FXString label=(modifiers&MOD_INV)?"x^1/y":"x^y";
  sender->handle(this,FXSEL(SEL_COMMAND,ID_SETSTRINGVALUE),(void*)&label);
  sender->handle(this,(base==10) ? FXSEL(SEL_COMMAND,ID_ENABLE) : FXSEL(SEL_COMMAND,ID_DISABLE),nullptr);
  return 1;
  }


// Sqrt/cbrt or x^2/x^3
long Calculator::onCmdSqrt(FXObject*,FXSelector,void*){
  unary(UN_SQRT+(modifiers&(MOD_INV|MOD_HYP)));
  return 1;
  }


// Update Sqrt/cbrt
long Calculator::onUpdSqrt(FXObject*,FXSelector,void*){
  if(modifiers&MOD_HYP){
    if(modifiers&MOD_INV)
      functions[3]->setText("x\xC2\xB3");
    else
      functions[3]->setText("\xE2\x88\x9B");
    }
  else{
    if(modifiers&MOD_INV)
      functions[3]->setText("x\xC2\xB2");
    else
      functions[3]->setText("\xE2\x88\x9A");
    }
  if(base==10){
    functions[3]->enable();
    }
  else{
    functions[3]->disable();
    }
  return 1;
  }


// Shift left
long Calculator::onCmdShl(FXObject*,FXSelector,void*){
  unary(UN_SHL);
  return 1;
  }


// Update Shift left
long Calculator::onUpdShl(FXObject* sender,FXSelector,void*){
  FXString label="shl";
  sender->handle(this,FXSEL(SEL_COMMAND,ID_SETSTRINGVALUE),(void*)&label);
  return 1;
  }


// Shift right
long Calculator::onCmdShr(FXObject*,FXSelector,void*){
  unary(UN_SHR+(modifiers&MOD_INV));
  return 1;
  }


// Update Shift right
long Calculator::onUpdShr(FXObject* sender,FXSelector,void*){
  FXString label=(modifiers&MOD_INV)?"sar":"shr";
  sender->handle(this,FXSEL(SEL_COMMAND,ID_SETSTRINGVALUE),(void*)&label);
  return 1;
  }


// Base 2 log
long Calculator::onCmd2Log(FXObject*,FXSelector,void*){
  unary(UN_2LOG+(modifiers&MOD_INV));
  return 1;
  }


// Update Base 2 log
long Calculator::onUpd2Log(FXObject*,FXSelector,void*){
  if(modifiers&MOD_INV){
    functions[4]->setText("2^x");
    functions[4]->setTipText("Base-2 exponential");
    }
  else{
    functions[4]->setText("log\xE2\x82\x82");
    functions[4]->setTipText("Base-2 log");
    }
  if(base==10){
    functions[4]->enable();
    }
  else{
    functions[4]->disable();
    }
  return 1;
  }


// Left parenth
long Calculator::onCmdLPar(FXObject*,FXSelector,void*){
  lparen();
  return 1;
  }


// Right parenth
long Calculator::onCmdRPar(FXObject*,FXSelector,void*){
  rparen();
  return 1;
  }


// Bitwise AND
long Calculator::onCmdAnd(FXObject*,FXSelector,void*){
  dyadic(DY_AND);
  return 1;
  }


// Bitwise OR
long Calculator::onCmdOr(FXObject*,FXSelector,void*){
  dyadic(DY_OR);
  return 1;
  }


// Bitwise XOR
long Calculator::onCmdXor(FXObject*,FXSelector,void*){
  dyadic(DY_XOR);
  return 1;
  }


// Bitwise NOT
long Calculator::onCmdNot(FXObject*,FXSelector,void*){
  unary(UN_NOT);
  return 1;
  }


// Multiply
long Calculator::onCmdMul(FXObject*,FXSelector,void*){
  dyadic(DY_MUL);
  return 1;
  }


// Divide
long Calculator::onCmdDiv(FXObject*,FXSelector,void*){
  dyadic(DY_DIV);
  return 1;
  }


// Modulo
long Calculator::onCmdMod(FXObject*,FXSelector,void*){
  dyadic(DY_MOD+(modifiers&MOD_INV));
  return 1;
  }


// Update mod
long Calculator::onUpdMod(FXObject*,FXSelector,void*){
  if(modifiers&MOD_INV){
    operators[9]->setText("/");
    operators[9]->setTipText("Integer divide");
    }
  else{
    operators[9]->setText("%");
    operators[9]->setTipText("Integer remainder");
    }
  return 1;
  }


// Add
long Calculator::onCmdAdd(FXObject*,FXSelector,void*){
  dyadic(DY_ADD);
  return 1;
  }


// Sub
long Calculator::onCmdSub(FXObject*,FXSelector,void*){
  dyadic(DY_SUB);
  return 1;
  }


// Enter
long Calculator::onCmdEnter(FXObject*,FXSelector,void*){
  evaluate();
  return 1;
  }


// Recall from memory
long Calculator::onCmdMemRec(FXObject*,FXSelector,void*){
  setnum(recall);
  setDisplayValue(recall);
  modifiers=0;
  return 1;
  }


// Add to memory
long Calculator::onCmdMemAdd(FXObject*,FXSelector,void*){
  recall+=getnum();
  modifiers=0;
  return 1;
  }


// Subtract from memory
long Calculator::onCmdMemSub(FXObject*,FXSelector,void*){
  recall-=getnum();
  modifiers=0;
  return 1;
  }


// Clear memory
long Calculator::onCmdMemClr(FXObject*,FXSelector,void*){
  recall=0.0;
  modifiers=0;
  return 1;
  }


