/********************************************************************************
*                                                                               *
*                           S l i d e r   W i d g e t                           *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997,2024 by Jeroen van der Zijp.   All Rights Reserved.        *
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
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxmath.h"
#include "fxkeys.h"
#include "FXElement.h"
#include "FXArray.h"
#include "FXMetaClass.h"
#include "FXHash.h"
#include "FXMutex.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXStringDictionary.h"
#include "FXSettings.h"
#include "FXRegistry.h"
#include "FXEvent.h"
#include "FXWindow.h"
#include "FXDCWindow.h"
#include "FXApp.h"
#include "FXRangeSlider.h"


/*
  Notes:
  - Maybe add bindings for arrow keys for value changes.
*/

#define OVERHANG        4           // Default amount of overhang
#define MINOVERHANG     3           // Minimal amount of overhang
#define HEADINSIDEBAR   20          // Default for inside bar head size
#define HEADOVERHANGING 9           // Default for overhanging head size

#define RANGESLIDER_MASK (RANGESLIDER_VERTICAL|RANGESLIDER_ARROW_UP|RANGESLIDER_ARROW_DOWN|RANGESLIDER_INSIDE_BAR)

using namespace FX;

/*******************************************************************************/

namespace FX {

// Map
FXDEFMAP(FXRangeSlider) FXRangeSliderMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXRangeSlider::onPaint),
  FXMAPFUNC(SEL_MOTION,0,FXRangeSlider::onMotion),
  FXMAPFUNC(SEL_MOUSEWHEEL,0,FXRangeSlider::onMouseWheel),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXRangeSlider::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXRangeSlider::onLeftBtnRelease),
  FXMAPFUNC(SEL_MIDDLEBUTTONPRESS,0,FXRangeSlider::onMiddleBtnPress),
  FXMAPFUNC(SEL_MIDDLEBUTTONRELEASE,0,FXRangeSlider::onMiddleBtnRelease),
  FXMAPFUNC(SEL_KEYPRESS,0,FXRangeSlider::onKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,0,FXRangeSlider::onKeyRelease),
  FXMAPFUNC(SEL_UNGRABBED,0,FXRangeSlider::onUngrabbed),
  FXMAPFUNC(SEL_QUERY_TIP,0,FXRangeSlider::onQueryTip),
  FXMAPFUNC(SEL_QUERY_HELP,0,FXRangeSlider::onQueryHelp),
  FXMAPFUNC(SEL_TIMEOUT,FXRangeSlider::ID_AUTOSLIDE,FXRangeSlider::onAutoSlide),
  FXMAPFUNC(SEL_COMMAND,FXRangeSlider::ID_SETINTRANGE,FXRangeSlider::onCmdSetIntRange),
  FXMAPFUNC(SEL_COMMAND,FXRangeSlider::ID_GETINTRANGE,FXRangeSlider::onCmdGetIntRange),
  FXMAPFUNC(SEL_COMMAND,FXRangeSlider::ID_SETREALRANGE,FXRangeSlider::onCmdSetRealRange),
  FXMAPFUNC(SEL_COMMAND,FXRangeSlider::ID_GETREALRANGE,FXRangeSlider::onCmdGetRealRange),
  FXMAPFUNC(SEL_COMMAND,FXRangeSlider::ID_SETHELPSTRING,FXRangeSlider::onCmdSetHelp),
  FXMAPFUNC(SEL_COMMAND,FXRangeSlider::ID_GETHELPSTRING,FXRangeSlider::onCmdGetHelp),
  FXMAPFUNC(SEL_COMMAND,FXRangeSlider::ID_SETTIPSTRING,FXRangeSlider::onCmdSetTip),
  FXMAPFUNC(SEL_COMMAND,FXRangeSlider::ID_GETTIPSTRING,FXRangeSlider::onCmdGetTip),
  };


// Object implementation
FXIMPLEMENT(FXRangeSlider,FXFrame,FXRangeSliderMap,ARRAYNUMBER(FXRangeSliderMap))


// Make a slider
FXRangeSlider::FXRangeSlider(){
  flags|=FLAG_ENABLED;
  values[0]=0;
  values[1]=0;
  values[2]=0;
  values[3]=0;
  headPos[0]=0;
  headPos[1]=0;
  headSize=0;
  slotSize=0;
  slotColor=0;
  dragPoint=0;
  active=0;
  incr=1;
  }


// Make a slider
FXRangeSlider::FXRangeSlider(FXComposite* p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb):FXFrame(p,opts,x,y,w,h,pl,pr,pt,pb){
  flags|=FLAG_ENABLED;
  baseColor=getApp()->getBaseColor();
  hiliteColor=getApp()->getHiliteColor();
  shadowColor=getApp()->getShadowColor();
  borderColor=getApp()->getBorderColor();
  slotColor=getApp()->getBackColor();
  target=tgt;
  message=sel;
  values[0]=0;
  values[1]=10;
  values[2]=90;
  values[3]=100;
  headPos[0]=0;
  headPos[1]=0;
  headSize=(options&RANGESLIDER_INSIDE_BAR)?HEADINSIDEBAR:HEADOVERHANGING;
  slotSize=5;
  dragPoint=0;
  active=0;
  incr=1;
  }


// Enable the window
void FXRangeSlider::enable(){
  if(!(flags&FLAG_ENABLED)){
    FXFrame::enable();
    update();
    }
  }


// Disable the window
void FXRangeSlider::disable(){
  if(flags&FLAG_ENABLED){
    FXFrame::disable();
    update();
    }
  }


// Get default width
FXint FXRangeSlider::getDefaultWidth(){
  FXint w;
  if(options&RANGESLIDER_VERTICAL){
    if(options&RANGESLIDER_INSIDE_BAR) w=4+headSize/2;
    else if(options&(RANGESLIDER_ARROW_LEFT|RANGESLIDER_ARROW_RIGHT)) w=slotSize+MINOVERHANG*2+headSize/2;
    else w=slotSize+MINOVERHANG*2;
    }
  else{
    w=headSize+headSize+4;
    }
  return w+padleft+padright+(border<<1);
  }


// Get default height
FXint FXRangeSlider::getDefaultHeight(){
  FXint h;
  if(options&RANGESLIDER_VERTICAL){
    h=headSize+headSize+4;
    }
  else{
    if(options&RANGESLIDER_INSIDE_BAR) h=4+headSize/2;
    else if(options&(RANGESLIDER_ARROW_UP|RANGESLIDER_ARROW_DOWN)) h=slotSize+2*MINOVERHANG+headSize/2;
    else h=slotSize+MINOVERHANG*2;
    }
  return h+padtop+padbottom+(border<<1);
  }


// Returns true because a slider can receive focus
FXbool FXRangeSlider::canFocus() const { return true; }


// Layout changed; even though the position is still
// the same, the head may have to be moved.
void FXRangeSlider::layout(){
  setValue(0,values[1]);
  setValue(1,values[2]);
  flags&=~FLAG_DIRTY;
  }


// Set help using a message
long FXRangeSlider::onCmdSetHelp(FXObject*,FXSelector,void* ptr){
  setHelpText(*((FXString*)ptr));
  return 1;
  }


// Get help using a message
long FXRangeSlider::onCmdGetHelp(FXObject*,FXSelector,void* ptr){
  *((FXString*)ptr)=getHelpText();
  return 1;
  }


// Set tip using a message
long FXRangeSlider::onCmdSetTip(FXObject*,FXSelector,void* ptr){
  setTipText(*((FXString*)ptr));
  return 1;
  }


// Get tip using a message
long FXRangeSlider::onCmdGetTip(FXObject*,FXSelector,void* ptr){
  *((FXString*)ptr)=getTipText();
  return 1;
  }


// We were asked about tip text
long FXRangeSlider::onQueryTip(FXObject* sender,FXSelector sel,void* ptr){
  if(FXFrame::onQueryTip(sender,sel,ptr)) return 1;
  if((flags&FLAG_TIP) && !tip.empty()){
    sender->handle(this,FXSEL(SEL_COMMAND,ID_SETSTRINGVALUE),(void*)&tip);
    return 1;
    }
  return 0;
  }


// We were asked about status text
long FXRangeSlider::onQueryHelp(FXObject* sender,FXSelector sel,void* ptr){
  if(FXFrame::onQueryHelp(sender,sel,ptr)) return 1;
  if((flags&FLAG_HELP) && !help.empty()){
    sender->handle(this,FXSEL(SEL_COMMAND,ID_SETSTRINGVALUE),(void*)&help);
    return 1;
    }
  return 0;
  }


// Update range from a message
long FXRangeSlider::onCmdSetIntRange(FXObject*,FXSelector,void* ptr){
  setRange(((FXint*)ptr)[0],((FXint*)ptr)[1]);
  return 1;
  }


// Get range with a message
long FXRangeSlider::onCmdGetIntRange(FXObject*,FXSelector,void* ptr){
  ((FXint*)ptr)[0]=values[0];
  ((FXint*)ptr)[1]=values[3];
  return 1;
  }


// Update range from a message
long FXRangeSlider::onCmdSetRealRange(FXObject*,FXSelector,void* ptr){
  setRange((FXint)((FXdouble*)ptr)[0],(FXint)((FXdouble*)ptr)[1]);
  return 1;
  }


// Get range with a message
long FXRangeSlider::onCmdGetRealRange(FXObject*,FXSelector,void* ptr){
  ((FXdouble*)ptr)[0]=(FXdouble)values[0];
  ((FXdouble*)ptr)[1]=(FXdouble)values[3];
  return 1;
  }


// Pressed LEFT button
long FXRangeSlider::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  FXint p;
  flags&=~FLAG_TIP;
  handle(this,FXSEL(SEL_FOCUS_SELF,0),ptr);
  if(isEnabled()){
    grab();
    getApp()->removeTimeout(this,ID_AUTOSLIDE);
    if(target && target->tryHandle(this,FXSEL(SEL_LEFTBUTTONPRESS,message),ptr)) return 1;
    flags&=~FLAG_UPDATE;
    if(options&RANGESLIDER_VERTICAL){
      if(event->win_y<(headPos[0]+headPos[1]+headSize)/2){
        if(event->win_y<headPos[1]){
          getApp()->addTimeout(this,ID_AUTOSLIDE,getApp()->getScrollDelay(),(void*)(FXival)incr);
          p=values[2]+incr;
          }
        else if(event->win_y>headPos[1]+headSize){
          getApp()->addTimeout(this,ID_AUTOSLIDE,getApp()->getScrollDelay(),(void*)(FXival)-incr);
          p=values[2]-incr;
          }
        else{
          dragPoint=event->win_y-headPos[1];
          p=values[2];
          flags|=FLAG_PRESSED;
          }
        active=1;
        }
      else{
        if(event->win_y<headPos[0]){
          getApp()->addTimeout(this,ID_AUTOSLIDE,getApp()->getScrollDelay(),(void*)(FXival)incr);
          p=values[1]+incr;
          }
        else if(event->win_y>headPos[0]+headSize){
          getApp()->addTimeout(this,ID_AUTOSLIDE,getApp()->getScrollDelay(),(void*)(FXival)-incr);
          p=values[1]-incr;
          }
        else{
          dragPoint=event->win_y-headPos[0];
          p=values[1];
          flags|=FLAG_PRESSED;
          }
        active=0;
        }
      }
    else{
      if(event->win_x<(headPos[0]+headSize+headPos[1])/2){
        if(event->win_x<headPos[0]){
          getApp()->addTimeout(this,ID_AUTOSLIDE,getApp()->getScrollDelay(),(void*)(FXival)-incr);
          p=values[1]-incr;
          }
        else if(event->win_x>headPos[0]+headSize){
          getApp()->addTimeout(this,ID_AUTOSLIDE,getApp()->getScrollDelay(),(void*)(FXival)incr);
          p=values[1]+incr;
          }
        else{
          dragPoint=event->win_x-headPos[0];
          p=values[1];
          flags|=FLAG_PRESSED;
          }
        active=0;
        }
      else{
        if(event->win_x<headPos[1]){
          getApp()->addTimeout(this,ID_AUTOSLIDE,getApp()->getScrollDelay(),(void*)(FXival)-incr);
          p=values[2]-incr;
          }
        else if(event->win_x>headPos[1]+headSize){
          getApp()->addTimeout(this,ID_AUTOSLIDE,getApp()->getScrollDelay(),(void*)(FXival)incr);
          p=values[2]+incr;
          }
        else{
          dragPoint=event->win_x-headPos[1];
          p=values[2];
          flags|=FLAG_PRESSED;
          }
        active=1;
        }
      }
    if(p<values[active+0]) p=values[active+0];
    if(p>values[active+2]) p=values[active+2];
    if(p!=values[active+1]){
      setValue(active,p);
      flags|=FLAG_CHANGED;
      if(target) target->tryHandle(this,FXSEL(SEL_CHANGED,message),(void*)&values[1]);
      }
    return 1;
    }
  return 0;
  }


// Released Left button
long FXRangeSlider::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  FXuint flgs=flags;
  if(isEnabled()){
    ungrab();
    getApp()->removeTimeout(this,ID_AUTOSLIDE);
    setValue(active,values[active+1]);                  // Hop to exact position
    flags&=~FLAG_PRESSED;
    flags&=~FLAG_CHANGED;
    flags|=FLAG_UPDATE;
    if(target && target->tryHandle(this,FXSEL(SEL_LEFTBUTTONRELEASE,message),ptr)) return 1;
    if(flgs&FLAG_CHANGED){
      if(target) target->tryHandle(this,FXSEL(SEL_COMMAND,message),(void*)&values[1]);
      }
    return 1;
    }
  return 0;
  }


// Moving
long FXRangeSlider::onMotion(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  FXint xx,yy,ww,hh,lo,hi,p,h,travel;
  if(!isEnabled()) return 0;
  if(flags&FLAG_PRESSED){
    yy=border+padtop+2;
    xx=border+padleft+2;
    hh=height-(border<<1)-padtop-padbottom-4;
    ww=width-(border<<1)-padleft-padright-4;
    if(options&RANGESLIDER_VERTICAL){
      h=event->win_y-dragPoint;
      travel=hh-headSize-headSize;
      if(active){
        if(h<yy) h=yy;
        if(h>headPos[0]-headSize) h=headPos[0]-headSize;
        if(travel>0)
          p=values[0]+((values[3]-values[0])*(yy+travel-h)+travel/2)/travel;            // Use rounding!!
        else
          p=values[0];
        }
      else{
        if(h<headPos[1]+headSize) h=headPos[1]+headSize;
        if(h>yy+hh-headSize) h=yy+hh-headSize;
        if(travel>0)
          p=values[0]+((values[3]-values[0])*(yy+travel+headSize-h)+travel/2)/travel;   // Use rounding!!
        else
          p=values[0];
        }
      if(h!=headPos[active]){
        FXMINMAX(lo,hi,headPos[active],h);
        headPos[active]=h;
        update(border,lo-1,width-(border<<1),hi+headSize+2-lo);
        }
      }
    else{
      h=event->win_x-dragPoint;
      travel=ww-headSize-headSize;
      if(active){
        if(h<headPos[0]+headSize) h=headPos[0]+headSize;
        if(h>xx+ww-headSize) h=xx+ww-headSize;
        if(travel>0)
          p=values[0]+((values[3]-values[0])*(h-xx-headSize)+travel/2)/travel;          // Use rounding!!
        else
          p=values[0];
        }
      else{
        if(h<xx) h=xx;
        if(h>headPos[1]-headSize) h=headPos[1]-headSize;
        if(travel>0)
          p=values[0]+((values[3]-values[0])*(h-xx)+travel/2)/travel;                   // Use rounding!!
        else
          p=values[0];
        }
      if(h!=headPos[active]){
        FXMINMAX(lo,hi,headPos[active],h);
        headPos[active]=h;
        update(lo-1,border,hi+headSize+2-lo,height-(border<<1));
        }
      }
    if(p<values[active+0]) p=values[active+0];
    if(p>values[active+2]) p=values[active+2];
    if(p!=values[active+1]){
      values[active+1]=p;
      flags|=FLAG_CHANGED;
      if(target) target->tryHandle(this,FXSEL(SEL_CHANGED,message),(void*)&values[1]);
      }
    return 1;
    }
  return 0;
  }


// Pressed middle or right
long FXRangeSlider::onMiddleBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  FXint xx,yy,ww,hh,lo,hi,p,h,travel;
  flags&=~FLAG_TIP;
  handle(this,FXSEL(SEL_FOCUS_SELF,0),ptr);
  if(isEnabled()){
    grab();
    if(target && target->tryHandle(this,FXSEL(SEL_MIDDLEBUTTONPRESS,message),ptr)) return 1;
    dragPoint=headSize/2;
    yy=border+padtop+2;
    xx=border+padleft+2;
    hh=height-(border<<1)-padtop-padbottom-4;
    ww=width-(border<<1)-padleft-padright-4;
    flags|=FLAG_PRESSED;
    flags&=~FLAG_UPDATE;
    if(options&RANGESLIDER_VERTICAL){
      h=event->win_y-dragPoint;
      travel=hh-headSize-headSize;
      if(event->win_y<(headPos[0]+headSize+headPos[1])/2){
        if(h<yy) h=yy;
        if(h>headPos[0]-headSize) h=headPos[0]-headSize;
        if(travel>0)
          p=values[0]+((values[3]-values[0])*(yy+travel-h)+travel/2)/travel;            // Use rounding!!
        else
          p=values[0];
        active=1;
        }
      else{
        if(h<headPos[1]+headSize) h=headPos[1]+headSize;
        if(h>yy+hh-headSize) h=yy+hh-headSize;
        if(travel>0)
          p=values[0]+((values[3]-values[0])*(yy+travel+headSize-h)+travel/2)/travel;   // Use rounding!!
        else
          p=values[0];
        active=0;
        }
      if(h!=headPos[active]){
        FXMINMAX(lo,hi,headPos[active],h);
        headPos[active]=h;
        update(border,lo-1,width-(border<<1),hi+headSize+2-lo);
        }
      }
    else{
      h=event->win_x-dragPoint;
      travel=ww-headSize-headSize;
      if(event->win_x<(headPos[0]+headSize+headPos[1])/2){
        if(h<xx) h=xx;
        if(h>headPos[1]-headSize) h=headPos[1]-headSize;
        if(travel>0)
          p=values[0]+((values[3]-values[0])*(h-xx-headSize)+travel/2)/travel;          // Use rounding!!
        else
          p=values[0];
        active=0;
        }
      else{
        if(h<headPos[0]+headSize) h=headPos[0]+headSize;
        if(h>xx+ww-headSize) h=xx+ww-headSize;
        if(travel>0)
          p=values[0]+((values[3]-values[0])*(h-xx)+travel/2)/travel;                   // Use rounding!!
        else
          p=values[0];
        active=1;
        }
      if(h!=headPos[active]){
        FXMINMAX(lo,hi,headPos[active],h);
        headPos[active]=h;
        update(lo-1,border,hi+headSize+2-lo,height-(border<<1));
        }
      }
    if(p<values[active+0]) p=values[active+0];
    if(p>values[active+2]) p=values[active+2];
    if(p!=values[active+1]){
      values[active+1]=p;
      flags|=FLAG_CHANGED;
      if(target) target->tryHandle(this,FXSEL(SEL_CHANGED,message),(void*)&values[1]);
      }
    return 1;
    }
  return 0;
  }


// Released middle button
long FXRangeSlider::onMiddleBtnRelease(FXObject*,FXSelector,void* ptr){
  FXuint flgs=flags;
  if(isEnabled()){
    ungrab();
    getApp()->removeTimeout(this,ID_AUTOSLIDE);
    flags&=~FLAG_PRESSED;
    flags&=~FLAG_CHANGED;
    flags|=FLAG_UPDATE;
    setValue(active,values[active+1]);                                  // Hop to exact position
    if(target && target->tryHandle(this,FXSEL(SEL_MIDDLEBUTTONRELEASE,message),ptr)) return 1;
    if(flgs&FLAG_CHANGED){
      if(target) target->tryHandle(this,FXSEL(SEL_COMMAND,message),(void*)&values[1]);
      }
    return 1;
    }
  return 0;
  }


// Mouse wheel
long FXRangeSlider::onMouseWheel(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXint a=(options&RANGESLIDER_VERTICAL) ? (event->win_y<=(headPos[0]+headSize+headPos[1])/2) : (event->win_x>=(headPos[0]+headSize+headPos[1])/2);
  FXint p=values[a+1]+(((FXEvent*)ptr)->code*incr)/120;
  if(p<values[a+0]) p=values[a+0];
  if(p>values[a+2]) p=values[a+2];
  if(p!=values[a+1]){
    setValue(a,p);
    if(target) target->tryHandle(this,FXSEL(SEL_COMMAND,message),(void*)&values[1]);
    }
  return 1;
  }


// Keyboard press
long FXRangeSlider::onKeyPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  if(isEnabled()){
    if(target && target->tryHandle(this,FXSEL(SEL_KEYPRESS,message),ptr)) return 1;
    switch(event->code){
      case KEY_Left:
      case KEY_KP_Left:
        if(!(options&RANGESLIDER_VERTICAL)) goto dec;
        break;
      case KEY_Right:
      case KEY_KP_Right:
        if(!(options&RANGESLIDER_VERTICAL)) goto inc;
        break;
      case KEY_Up:
      case KEY_KP_Up:
        if(options&RANGESLIDER_VERTICAL) goto inc;
        break;
      case KEY_Down:
      case KEY_KP_Down:
        if(options&RANGESLIDER_VERTICAL) goto dec;
        break;
      case KEY_plus:
      case KEY_KP_Add:
inc:    setValue(active,values[active+1]+incr,true);
        return 1;
      case KEY_minus:
      case KEY_KP_Subtract:
dec:    setValue(active,values[active+1]-incr,true);
        return 1;
      }
    }
  return 0;
  }


// Keyboard release
long FXRangeSlider::onKeyRelease(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  if(isEnabled()){
    if(target && target->tryHandle(this,FXSEL(SEL_KEYRELEASE,message),ptr)) return 1;
    switch(event->code){
      case KEY_Left:
      case KEY_KP_Left:
      case KEY_Right:
      case KEY_KP_Right:
        if(!(options&RANGESLIDER_VERTICAL)) return 1;
        break;
      case KEY_Up:
      case KEY_KP_Up:
      case KEY_Down:
      case KEY_KP_Down:
        if(options&RANGESLIDER_VERTICAL) return 1;
        break;
      case KEY_plus:
      case KEY_KP_Add:
      case KEY_KP_Subtract:
      case KEY_minus:
        return 1;
      }
    }
  return 0;
  }


// The widget lost the grab for some reason
long FXRangeSlider::onUngrabbed(FXObject* sender,FXSelector sel,void* ptr){
  FXFrame::onUngrabbed(sender,sel,ptr);
  getApp()->removeTimeout(this,ID_AUTOSLIDE);
  flags&=~FLAG_PRESSED;
  flags&=~FLAG_CHANGED;
  flags|=FLAG_UPDATE;
  return 1;
  }


// Automatically move slider while holding down mouse
long FXRangeSlider::onAutoSlide(FXObject*,FXSelector,void* ptr){
  FXint inc=(FXint)(FXival)ptr;
  FXint p=values[active+1]+inc;
  if(p<=values[active+0]){
    p=values[active+0];
    }
  else if(p>=values[active+2]){
    p=values[active+2];
    }
  else{
    getApp()->addTimeout(this,ID_AUTOSLIDE,getApp()->getScrollSpeed(),(void*)(FXival)inc);
    }
  if(p!=values[active+1]){
    setValue(active,p);
    flags|=FLAG_CHANGED;
    if(target) target->tryHandle(this,FXSEL(SEL_CHANGED,message),(void*)&values[1]);
    return 1;
    }
  return 0;
  }


// Draw slider head
void FXRangeSlider::drawSliderHead(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h){
  FXint m;
  dc.setForeground(baseColor);
  dc.fillRectangle(x,y,w,h);
  if(options&RANGESLIDER_VERTICAL){
    m=(h>>1);
    if(options&RANGESLIDER_ARROW_LEFT){
      dc.setForeground(hiliteColor);
      dc.drawLine(x+m,y,x+w-1,y);
      dc.drawLine(x,y+m,x+m,y);
      dc.setForeground(shadowColor);
      dc.drawLine(x+1,y+h-m-1,x+m+1,y+h-1);
      dc.drawLine(x+m,y+h-2,x+w-1,y+h-2);
      dc.drawLine(x+w-2,y+1,x+w-2,y+h-1);
      dc.setForeground(borderColor);
      dc.drawLine(x,y+h-m-1,x+m,y+h-1);
      dc.drawLine(x+w-1,y+h-1,x+w-1,y);
      dc.fillRectangle(x+m,y+h-1,w-m,1);
      }
    else if(options&RANGESLIDER_ARROW_RIGHT){
      dc.setForeground(hiliteColor);
      dc.drawLine(x,y,x+w-m-1,y);
      dc.drawLine(x,y+1,x,y+h-1);
      dc.drawLine(x+w-1,y+m,x+w-m-1,y);
#ifdef WIN32
      dc.setForeground(shadowColor);
      dc.drawLine(x+w-1,y+h-m-2,x+w-m-2,y+h-1);
      dc.drawLine(x+1,y+h-2,x+w-m-1,y+h-2);
      dc.setForeground(borderColor);
      dc.drawLine(x+w,y+h-m-2,x+w-m-1,y+h-1);
      dc.drawLine(x,y+h-1,x+w-m-1,y+h-1);
#else
      dc.setForeground(shadowColor);
      dc.drawLine(x+w-2,y+h-m-1,x+w-m-2,y+h-1);
      dc.drawLine(x+1,y+h-2,x+w-m-1,y+h-2);
      dc.setForeground(borderColor);
      dc.drawLine(x+w-1,y+h-m-1,x+w-m-1,y+h-1);
      dc.drawLine(x,y+h-1,x+w-m-1,y+h-1);
#endif
      }
    else if(options&RANGESLIDER_INSIDE_BAR){
      drawDoubleRaisedRectangle(dc,x,y,w,h);
      dc.setForeground(shadowColor);
      dc.drawLine(x+1,y+m-1,x+w-2,y+m-1);
      dc.setForeground(hiliteColor);
      dc.drawLine(x+1,y+m,x+w-2,y+m);
      }
    else{
      drawDoubleRaisedRectangle(dc,x,y,w,h);
      }
    }
  else{
    m=(w>>1);
    if(options&RANGESLIDER_ARROW_UP){
      dc.setForeground(hiliteColor);
      dc.drawLine(x,y+m,x+m,y);
      dc.drawLine(x,y+m,x,y+h-1);
      dc.setForeground(shadowColor);
      dc.drawLine(x+w-1,y+m+1,x+w-m-1,y+1);
      dc.drawLine(x+w-2,y+m+1,x+w-2,y+h-1);
      dc.drawLine(x+1,y+h-2,x+w-2,y+h-2);
      dc.setForeground(borderColor);
      dc.drawLine(x+w-1,y+m,x+w-m-1,y);
      dc.drawLine(x+w-1,y+m,x+w-1,y+h-1);
      dc.fillRectangle(x,y+h-1,w,1);
      }
    else if(options&RANGESLIDER_ARROW_DOWN){
      dc.setForeground(hiliteColor);
      dc.drawLine(x,y,x+w-1,y);
      dc.drawLine(x,y+1,x,y+h-m-1);
      dc.drawLine(x,y+h-m-1,x+m,y+h-1);
      dc.setForeground(shadowColor);
      dc.drawLine(x+w-2,y+1,x+w-2,y+h-m-1);
      dc.drawLine(x+w-1,y+h-m-2,x+w-m-1,y+h-2);
      dc.setForeground(borderColor);
      dc.drawLine(x+w-1,y+h-m-1,x+w-m-1,y+h-1);
      dc.fillRectangle(x+w-1,y,1,h-m);
      }
    else if(options&RANGESLIDER_INSIDE_BAR){
      drawDoubleRaisedRectangle(dc,x,y,w,h);
      dc.setForeground(shadowColor);
      dc.drawLine(x+m-1,y+1,x+m-1,y+h-2);
      dc.setForeground(hiliteColor);
      dc.drawLine(x+m,y+1,x+m,y+h-1);
      }
    else{
      drawDoubleRaisedRectangle(dc,x,y,w,h);
      }
    }
  }


// Handle repaint
long FXRangeSlider::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  FXint tx,ty,hhs=headSize/2;
  FXint xx,yy,ww,hh;
  FXDCWindow dc(this,event);

  // Repaint background
  dc.setForeground(backColor);
  dc.fillRectangle(0,0,width,height);

  // Repaint border
  drawFrame(dc,0,0,width,height);

  // Slot placement
  xx=border+padleft;
  yy=border+padtop;
  ww=width-(border<<1)-padleft-padright;
  hh=height-(border<<1)-padtop-padbottom;

  // Draw the slot
  if(options&RANGESLIDER_VERTICAL){

    // Draw slider
    if(options&RANGESLIDER_INSIDE_BAR){
      drawDoubleSunkenRectangle(dc,xx,yy,ww,hh);
      dc.setStipple(STIPPLE_GRAY);
      dc.setForeground(slotColor);
      dc.setBackground(baseColor);
      dc.setFillStyle(FILL_OPAQUESTIPPLED);
      dc.fillRectangle(xx+2,yy+2,ww-4,hh-4);
      dc.setFillStyle(FILL_SOLID);
      if(isEnabled()){
        drawSliderHead(dc,xx+2,headPos[0],ww-4,headSize);
        drawSliderHead(dc,xx+2,headPos[1],ww-4,headSize);
        }
      }
    else{
      if(options&RANGESLIDER_ARROW_LEFT) tx=xx+hhs+(ww-slotSize-hhs)/2;
      else if(options&RANGESLIDER_ARROW_RIGHT) tx=xx+(ww-slotSize-hhs)/2;
      else tx=xx+(ww-slotSize)/2;
      drawDoubleSunkenRectangle(dc,tx,yy,slotSize,hh);
      dc.setForeground(slotColor);
      dc.fillRectangle(tx+2,yy+2,slotSize-4,hh-4);
      if(isEnabled()){
        drawSliderHead(dc,xx,headPos[0],ww,headSize);
        drawSliderHead(dc,xx,headPos[1],ww,headSize);
        }
      }
    }
  else{

    // Draw slider
    if(options&RANGESLIDER_INSIDE_BAR){
      drawDoubleSunkenRectangle(dc,xx,yy,ww,hh);
      dc.setForeground(slotColor);
      dc.setStipple(STIPPLE_GRAY);
      dc.setForeground(slotColor);
      dc.setBackground(baseColor);
      dc.setFillStyle(FILL_OPAQUESTIPPLED);
      dc.fillRectangle(xx+2,yy+2,ww-4,hh-4);
      dc.setFillStyle(FILL_SOLID);
      if(isEnabled()){
        drawSliderHead(dc,headPos[0],yy+2,headSize,hh-4);
        drawSliderHead(dc,headPos[1],yy+2,headSize,hh-4);
        }
      }
    else{
      if(options&RANGESLIDER_ARROW_UP) ty=yy+hhs+(hh-slotSize-hhs)/2;
      else if(options&RANGESLIDER_ARROW_DOWN) ty=yy+(hh-slotSize-hhs)/2;
      else ty=yy+(hh-slotSize)/2;
      drawDoubleSunkenRectangle(dc,xx,ty,ww,slotSize);
      dc.setForeground(slotColor);
      dc.fillRectangle(xx+2,ty+2,ww-4,slotSize-4);
      if(isEnabled()){
        drawSliderHead(dc,headPos[0],yy,headSize,hh);
        drawSliderHead(dc,headPos[1],yy,headSize,hh);
        }
      }
    }
  return 1;
  }


// Set slider range; this also revalidates the position,
// and possibly moves the head [even if the position was still OK,
// the head might still have to be moved to the exact position].
void FXRangeSlider::setRange(FXint lo,FXint hi,FXbool notify){
  if(lo>hi){ fxerror("%s::setRange: trying to set negative range.\n",getClassName()); }
  if(lo!=values[0] || hi!=values[3]){
    values[0]=lo;
    values[3]=hi;
    setValue(0,values[1],notify);
    setValue(1,values[2],notify);
    }
  }


// Set position; this should always cause the head to reflect
// the exact [discrete] value representing pos, even if several
// head positions may represent the same position!
// Also, the minimal amount is repainted, as one sometimes as very
// large/wide sliders.
void FXRangeSlider::setValue(FXint head,FXint value,FXbool notify){
  FXint interval=values[3]-values[0];
  FXint travel,lo,hi,h;
  if(value<values[head+0]) value=values[head+0];
  if(value>values[head+2]) value=values[head+2];
  if(options&RANGESLIDER_VERTICAL){
    travel=height-(border<<1)-padtop-padbottom-headSize-headSize-4;
    h=height-border-padbottom-2-headSize-head*headSize;                 // Top of head
    if(0<interval) h-=(travel*(value-values[0]))/interval;
    if(h!=headPos[head]){
      FXMINMAX(lo,hi,headPos[head],h);
      headPos[head]=h;
      update(border,lo-1,width-(border<<1),hi+headSize+2-lo);
      }
    }
  else{
    travel=width-(border<<1)-padleft-padright-headSize-headSize-4;
    h=border+padleft+2+head*headSize;
    if(0<interval) h+=(travel*(value-values[0]))/interval;
    if(h!=headPos[head]){
      FXMINMAX(lo,hi,headPos[head],h);
      headPos[head]=h;
      update(lo-1,border,hi+headSize+2-lo,height-(border<<1));
      }
    }
  if(value!=values[1+head]){
    values[1+head]=value;
    if(notify && target){target->tryHandle(this,FXSEL(SEL_COMMAND,message),(void*)&values[1]);}
    }
  }


// Get slider options
FXuint FXRangeSlider::getSliderStyle() const {
  return (options&RANGESLIDER_MASK);
  }


// Set slider options
void FXRangeSlider::setSliderStyle(FXuint style){
  FXuint opts=(options&~RANGESLIDER_MASK) | (style&RANGESLIDER_MASK);
  if(options!=opts){
    headSize=(opts&RANGESLIDER_INSIDE_BAR)?HEADINSIDEBAR:HEADOVERHANGING;
    options=opts;
    recalc();
    update();
    }
  }


// Set head size
void FXRangeSlider::setHeadSize(FXint hs){
  if(headSize!=hs){
    headSize=hs;
    recalc();
    update();
    }
  }


// Set slot size
void FXRangeSlider::setSlotSize(FXint bs){
  if(slotSize!=bs){
    slotSize=bs;
    recalc();
    update();
    }
  }


// Set increment
void FXRangeSlider::setIncrement(FXint inc){
  if(inc<=0){ fxerror("%s::setIncrement: negative or zero increment specified.\n",getClassName()); }
  incr=inc;
  }


// Set slot color
void FXRangeSlider::setSlotColor(FXColor clr){
  if(slotColor!=clr){
    slotColor=clr;
    update();
    }
  }


// Save object to stream
void FXRangeSlider::save(FXStream& store) const {
  FXFrame::save(store);
  store << headSize;
  store << slotSize;
  store << slotColor;
  store << values[0];
  store << values[1];
  store << values[2];
  store << values[3];
  store << incr;
  store << help;
  store << tip;
  }


// Load object from stream
void FXRangeSlider::load(FXStream& store){
  FXFrame::load(store);
  store >> headSize;
  store >> slotSize;
  store >> slotColor;
  store >> values[0];
  store >> values[1];
  store >> values[2];
  store >> values[3];
  store >> incr;
  store >> help;
  store >> tip;
  }


// Delete
FXRangeSlider::~FXRangeSlider(){
  getApp()->removeTimeout(this,ID_AUTOSLIDE);
  }

}
