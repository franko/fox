/********************************************************************************
*                                                                               *
*                   S y n t a x   H i g h l i g h t   E n g i n e               *
*                                                                               *
*********************************************************************************
* Copyright (C) 2002,2021 by Jeroen van der Zijp.   All Rights Reserved.        *
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
#include "Syntax.h"


/*
  Notes:

  - Syntax (Language):

    o  Language is recognized any of three ways:

       1  Direct association of filename to language name in registry;
          typically, set explicitly by the user.

       2  File extension matches against Language's list of file extensions.
          Note that this is processed in the order that the languages are
          listed in the syntax file.

       3  Contents of the file (at least, the first fragment of the contents),
          matches against regular expression patten from the Language.

    o  Language sets delimiters in text editor.

    o  Language sets syntax rules to be used for colorizing text.

  - Syntax Rules:

    o  Four types of rules at this time:

       1  DefaultRule (new).  This rule colorizes everything.  By default,
          a top-level DefaultRule is automatically created in the Language,
          corresponding to style index 0 in text editor (default style).
          In some cases, you may want to create another DefaultRule under
          that.  This will color the entire text with another style.

       2  SimpleRule.  This rule matches a single regex pattern. This rule
          has no subrules.  Use this rule to match simple keywords, operators,
          etc.

       3  BracketRule.  This rule matches a opening pattern at the start, and
          a closing pattern at the end.  Everything in between (including the
          subject text matched against the patterns) will be colorized by the
          rule's style.  Subrules under the BracketRule may colorize parts
          differently.

       4  SafeBracketRule. This rule behaves like BracketRule.  Except that
          each time a sub-chunk is matched, a stop-pattern is tested to see
          if we end the subrule prior to the normal closing-pattern.
          The extra clause provides a safety back-stop against matching a
          potentially huge portion of text in the event that the user is
          editing the text and no text currently matches the closing pattern.

    o The pattern in a SimpleRule must be non-empty, i.e. it must match a non-zero
      number of characters.  The other rules may match zero characters (e.g.
      assertions like "^", "\<", "(?=fred)" are OK).

    o Each rule has its own style index.  This is important for the incremental
      restyling algorithm.  Each rule also has a link to its immediate parent
      rule, thus.  Thus, during editing, we can identify the ancestor rules
      so as to minimally recolor the text.  The algorithm backs up in wider and
      wider ranges about the changed text until a starting pattern of an ancestor
      rule is matched again.  In most cases, this search doesn't go very deep,
      and only small chunks of text need recolorizing.

    o Rules MAY have the same NAMES even though they are different rules.  That
      merely means these rules will have the same style.  It says nothing about
      what is matched or how the colorizer works.

  - SyntaxParser is a simple recursive descent parser.  The style language has
    very few lexical elements: comments, spaces, numbers, strings, and identifiers.

    o  Comments.  Comments are lines preceeded by a '#'.  All text after the
       '#' is skipped until the end of the line.

    o  Spaces.  Spaces are not significant, except inside strings.  Spaces are
       simply skipped.

    o  Numbers.  Only decimal integer numbers are supported.

    o  Strings.  Strings are any text between double quotes ('"').  There are no
       escape sequences, except to enter a double quote ('\"').  This is so as
       to keep regular expressions somewhat legible.  Note that the regular
       expression engine *does* support escape sequences, so matching special
       characters is quite simple.

    o  Identifiers.  As with most programming languages, identifiers start with
       a letter, followed by letters or digits.  Only the following identifiers
       are recognized as language elements:

         language, rule, filesmatch, contentsmatch, contextlines, contextchars,
         delimiters, pattern, openpattern, closepattern, stoppattern, end.

       [There may be more in the future].

  - We currently don't use capturing parentheses capabilities; bracketing rules
    seem sufficient for most languages.  Perhaps this will change.
*/

/*******************************************************************************/


FXIMPLEMENT(Rule,FXObject,NULL,0)


// Fill textstyle with style, returns position of last change+1
static inline void fillstyle(FXchar* textstyle,FXchar style,FXint f,FXint t){
  while(f<t) textstyle[f++]=style;
  }


// Stylize text
FXbool Rule::stylize(const FXchar*,FXchar*,FXint,FXint,FXint&,FXint&) const {
  return false;
  }


// Stylize body, i.e. after begin pattern has been seen
FXbool Rule::stylizeBody(const FXchar*,FXchar*,FXint,FXint,FXint&,FXint&) const {
  return false;
  }

/*******************************************************************************/

FXIMPLEMENT(SimpleRule,Rule,NULL,0)


// Stylize complex recursive expression
FXbool SimpleRule::stylizeBody(const FXchar* text,FXchar *textstyle,FXint fm,FXint to,FXint& start,FXint& stop) const {
  if(pat.amatch(text,to,fm,FXRex::Normal,&start,&stop,1)){
    fillstyle(textstyle,index,start,stop);
    return true;
    }
  return false;
  }


// Stylize simple expression
FXbool SimpleRule::stylize(const FXchar* text,FXchar *textstyle,FXint fm,FXint to,FXint& start,FXint& stop) const {
  if(pat.amatch(text,to,fm,FXRex::Normal,&start,&stop,1)){
    fillstyle(textstyle,index,start,stop);
    return true;
    }
  return false;
  }

/*******************************************************************************/

FXIMPLEMENT(BracketRule,Rule,NULL,0)


// Stylize complex recursive expression
FXbool BracketRule::stylizeBody(const FXchar* text,FXchar *textstyle,FXint fm,FXint to,FXint& start,FXint& stop) const {
  FXint head,tail,node;
  start=fm;
  while(fm<to){
    for(node=0; node<rules.no(); node++){
      if(rules[node]->stylize(text,textstyle,fm,to,head,tail)){
        fm=tail;
        goto nxt;
        }
      }
    if(end.amatch(text,to,fm,FXRex::Normal,&head,&stop,1)){
      fillstyle(textstyle,index,head,stop);
      return true;
      }
    textstyle[fm++]=index;
nxt:continue;
    }
  stop=fm;
  return true;
  }


// Stylize complex recursive expression
FXbool BracketRule::stylize(const FXchar* text,FXchar *textstyle,FXint fm,FXint to,FXint& start,FXint& stop) const {
  FXint head,tail;
  if(beg.amatch(text,to,fm,FXRex::Normal,&start,&tail,1)){
    fillstyle(textstyle,index,start,tail);
    BracketRule::stylizeBody(text,textstyle,tail,to,head,stop);
    return true;
    }
  return false;
  }

/*******************************************************************************/

FXIMPLEMENT(SafeBracketRule,BracketRule,NULL,0)


// Stylize complex recursive expression with termination pattern
FXbool SafeBracketRule::stylizeBody(const FXchar* text,FXchar *textstyle,FXint fm,FXint to,FXint& start,FXint& stop) const {
  FXint head,tail,node;
  start=fm;
  while(fm<to){
    if(esc.amatch(text,to,fm,FXRex::Normal,&head,&stop,1)){     // Each time around, check stop pattern (changed from old method!)
      fillstyle(textstyle,index,head,stop);
      return true;
      }
    for(node=0; node<rules.no(); node++){
      if(rules[node]->stylize(text,textstyle,fm,to,head,tail)){
        fm=tail;
        goto nxt;
        }
      }
    if(end.amatch(text,to,fm,FXRex::Normal,&head,&stop,1)){
      fillstyle(textstyle,index,head,stop);
      return true;
      }
    textstyle[fm++]=index;
nxt:continue;
    }
  stop=fm;
  return true;
  }


// Stylize complex recursive expression with termination pattern
FXbool SafeBracketRule::stylize(const FXchar* text,FXchar *textstyle,FXint fm,FXint to,FXint& start,FXint& stop) const {
  FXint head,tail;
  if(beg.amatch(text,to,fm,FXRex::Normal,&start,&tail,1)){
    fillstyle(textstyle,index,start,tail);
    SafeBracketRule::stylizeBody(text,textstyle,tail,to,head,stop);
    return true;
    }
  return false;
  }

/*******************************************************************************/

FXIMPLEMENT(DefaultRule,Rule,NULL,0)


// Stylize body
FXbool DefaultRule::stylizeBody(const FXchar* text,FXchar *textstyle,FXint fm,FXint to,FXint& start,FXint& stop) const {
  FXint head,tail,node;
  start=fm;
  while(fm<to){
    for(node=0; node<rules.no(); node++){
      if(rules[node]->stylize(text,textstyle,fm,to,head,tail)){
        fm=tail;
        goto nxt;
        }
      }
    textstyle[fm++]=index;
nxt:continue;
    }
  stop=to;
  return true;
  }


// Stylize text
FXbool DefaultRule::stylize(const FXchar* text,FXchar *textstyle,FXint fm,FXint to,FXint& start,FXint& stop) const {
  return DefaultRule::stylizeBody(text,textstyle,fm,to,start,stop);
  }

/*******************************************************************************/

FXIMPLEMENT(Syntax,FXObject,NULL,0)


// Construct syntax object; needs at least one master rule
Syntax::Syntax(const FXString& lang,const FXString& grp):language(lang),group(grp),delimiters(FXText::textDelimiters),contextLines(1),contextChars(1),autoindent(-1),wrapwidth(-1),tabwidth(-1),wrapmode(-1),tabmode(-1){
  FXTRACE((10,"Syntax::Syntax(\"%s\",\"%s\")\n",lang.text(),grp.text()));
  rules.append(new DefaultRule("Default",FXString::null,-1,0));
  }


// Find rule given name
FXint Syntax::getNamedRule(const FXString& name) const {
  for(FXint i=0; i<rules.no(); ++i){
    if(rules[i]->getName()==name) return i;
    }
  return -1;
  }


// Match filename against wildcards
FXbool Syntax::matchFilename(const FXString& name) const {
  return FXPath::match(name,extensions);
  }


// Match contents against regular expression
FXbool Syntax::matchContents(const FXString& text) const {
  FXRex rex(contents);
  return rex.search(text,0,text.length(),FXRex::Normal)>=0;
  }


// Append default rule
FXint Syntax::appendDefault(const FXString& name,const FXString& style,FXint parent){
  FXTRACE((10,"Syntax::appendDefault(\"%s\",\"%s\",%d)\n",name.text(),style.text(),parent));
  FXint index=rules.no();
  FXASSERT(0<=parent && parent<rules.no());
  DefaultRule *rule=new DefaultRule(name,style,parent,index);
  rules.append(rule);
  rules[parent]->rules.append(rule);
  return index;
  }


// Append simple rule
FXint Syntax::appendSimple(const FXString& name,const FXString& style,const FXString& rex,FXint parent){
  FXTRACE((10,"Syntax::appendSimple(\"%s\",\"%s\",\"%s\",%d)\n",name.text(),style.text(),rex.text(),parent));
  FXint index=rules.no();
  FXASSERT(0<=parent && parent<rules.no());
  SimpleRule *rule=new SimpleRule(name,style,rex,parent,index);
  rules.append(rule);
  rules[parent]->rules.append(rule);
  return index;
  }


// Append bracket rule
FXint Syntax::appendBracket(const FXString& name,const FXString& style,const FXString& brex,const FXString& erex,FXint parent){
  FXTRACE((10,"Syntax::appendBracket(\"%s\",\"%s\",\"%s\",\"%s\",%d)\n",name.text(),style.text(),brex.text(),erex.text(),parent));
  FXint index=rules.no();
  FXASSERT(0<=parent && parent<rules.no());
  BracketRule *rule=new BracketRule(name,style,brex,erex,parent,index);
  rules.append(rule);
  rules[parent]->rules.append(rule);
  return index;
  }


// Append safe bracket rule
FXint Syntax::appendSafeBracket(const FXString& name,const FXString& style,const FXString& brex,const FXString& erex,const FXString& srex,FXint parent){
  FXTRACE((10,"Syntax::appendSafeBracket(\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d)\n",name.text(),style.text(),brex.text(),erex.text(),srex.text(),parent));
  FXint index=rules.no();
  FXASSERT(0<=parent && parent<rules.no());
  SafeBracketRule *rule=new SafeBracketRule(name,style,brex,erex,srex,parent,index);
  rules.append(rule);
  rules[parent]->rules.append(rule);
  return index;
  }


// Return true if toplevel rule
FXbool Syntax::isRoot(FXint rule) const {
  FXASSERT(0<=rule && rule<rules.no());
  return rule==0 || rules[rule]->parent==0;
  }


// Return true if p is ancestor of c
FXbool Syntax::isAncestor(FXint p,FXint c) const {
  FXASSERT(0<=p && p<rules.no());
  FXASSERT(0<=c && c<rules.no());
  while(0<c){
    c=rules[c]->getParent();
    if(c==p) return true;
    }
  return false;
  }


// Return common ancestor of a and b
FXint Syntax::commonAncestor(FXint a,FXint b) const {
  FXASSERT(0<=a && a<rules.no());
  FXASSERT(0<=b && b<rules.no());
  if(0<a && 0<b){
    FXint p=a;
    while(0<p){
      FXint q=b;
      while(0<q){
        if(q==p) return p;
        q=rules[q]->getParent();
        }
      p=rules[p]->getParent();
      }
    }
  return 0;
  }


// Clean up
Syntax::~Syntax(){
  for(FXint i=0; i<rules.no(); i++){ delete rules[i]; }
  }
