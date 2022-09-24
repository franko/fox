/********************************************************************************
*                                                                               *
*                   S y n t a x   H i g h l i g h t   E n g i n e               *
*                                                                               *
*********************************************************************************
* Copyright (C) 2002,2022 by Jeroen van der Zijp.   All Rights Reserved.        *
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
#ifndef SYNTAX_H
#define SYNTAX_H

class Rule;
class Syntax;


// List of syntax rules
typedef FXPtrListOf<Rule> RuleList;


// List of syntaxes
typedef FXPtrListOf<Syntax> SyntaxList;


// Highlight node
class Rule {
  friend class Syntax;
protected:
  FXString      name;           // Name of rule
  FXString      style;          // Colors for highlighting (default)
  RuleList      rules;          // Subrules
  FXint         parent;         // Parent rule
  FXint         index;          // Own style index
protected:
  Rule(){}
private:
  Rule(const Rule&);
  Rule &operator=(const Rule&);
public:

  // Construct node
  Rule(const FXString& nam,const FXString& sty,FXint par,FXint idx);

  // Set rule name
  void setName(const FXString& nm){ name=nm; }

  // Get rule name
  const FXString& getName() const { return name; }

  // Set style name
  void setStyle(const FXString& st){ style=st; }

  // Get style name
  const FXString& getStyle() const { return style; }

  // Get parent index
  FXint getParent() const { return parent; }

  // Get style index
  FXint getIndex() const { return index; }

  // Get number of child rules
  FXint getNumRules() const { return rules.no(); }

  // Get child rule
  Rule* getRule(FXint inx) const { return rules[inx]; }

  // Stylize text
  virtual FXbool stylize(const FXchar* text,FXchar* textstyle,FXint len,FXint pos,FXint& head,FXint& tail) const;

  // Stylize body, i.e. after begin pattern has been seen
  virtual FXbool stylizeBody(const FXchar* text,FXchar* textstyle,FXint len,FXint pos,FXint& head,FXint& tail) const;

  // Destructor
  virtual ~Rule();
  };


// Default highlight node
class DefaultRule : public Rule {
protected:
  DefaultRule(){ }
private:
  DefaultRule(const DefaultRule&);
  DefaultRule &operator=(const DefaultRule&);
public:

  // Construct node
  DefaultRule(const FXString& nam,const FXString& sty,FXint par,FXint idx);

  // Stylize text
  virtual FXbool stylize(const FXchar* text,FXchar *textstyle,FXint len,FXint pos,FXint& head,FXint& tail) const;

  // Stylize body, i.e. after begin pattern has been seen
  virtual FXbool stylizeBody(const FXchar* text,FXchar* textstyle,FXint len,FXint pos,FXint& head,FXint& tail) const;

  // Destructor
  virtual ~DefaultRule();
  };


// Simple highlight node
class SimpleRule : public Rule {
protected:
  FXRex         pattern;        // Pattern to match
protected:
  SimpleRule(){ }
private:
  SimpleRule(const SimpleRule&);
  SimpleRule &operator=(const SimpleRule&);
public:

  // Construct node
  SimpleRule(const FXString& nam,const FXString& sty,const FXString& rex,FXint par,FXint idx);

  // Stylize text
  virtual FXbool stylize(const FXchar* text,FXchar *textstyle,FXint len,FXint pos,FXint& head,FXint& tail) const;

  // Stylize body, i.e. after begin pattern has been seen
  virtual FXbool stylizeBody(const FXchar* text,FXchar* textstyle,FXint len,FXint pos,FXint& head,FXint& tail) const;

  // Destructor
  virtual ~SimpleRule();
  };


// Bracketed highlight node
class BracketRule : public Rule {
protected:
  FXRex         open;           // Beginning pattern
  FXRex         close;          // Ending pattern
protected:
  BracketRule(){ }
private:
  BracketRule(const BracketRule&);
  BracketRule &operator=(const BracketRule&);
public:

  // Construct node
  BracketRule(const FXString& nam,const FXString& sty,const FXString& brex,const FXString& erex,FXint par,FXint idx);

  // Stylize text
  virtual FXbool stylize(const FXchar* text,FXchar *textstyle,FXint len,FXint pos,FXint& head,FXint& tail) const;

  // Stylize body, i.e. after begin pattern has been seen
  virtual FXbool stylizeBody(const FXchar* text,FXchar* textstyle,FXint len,FXint pos,FXint& head,FXint& tail) const;

  // Destructor
  virtual ~BracketRule();
  };


// Bracketed highlight node with termination
class SafeBracketRule : public BracketRule {
protected:
  FXRex         stop;           // Termination pattern
protected:
  SafeBracketRule(){ }
private:
  SafeBracketRule(const SafeBracketRule&);
  SafeBracketRule &operator=(const SafeBracketRule&);
public:

  // Construct node
  SafeBracketRule(const FXString& nam,const FXString& sty,const FXString& brex,const FXString& erex,const FXString& srex,FXint par,FXint idx);

  // Stylize text
  virtual FXbool stylize(const FXchar* text,FXchar *textstyle,FXint len,FXint pos,FXint& head,FXint& tail) const;

  // Stylize body, i.e. after begin pattern has been seen
  virtual FXbool stylizeBody(const FXchar* text,FXchar* textstyle,FXint len,FXint pos,FXint& head,FXint& tail) const;

  // Destructor
  virtual ~SafeBracketRule();
  };


// Span pattern rule
class SpanRule : public Rule {
protected:
  FXRex         pattern;        // Pattern to match
protected:
  SpanRule(){ }
private:
  SpanRule(const SpanRule&);
  SpanRule &operator=(const SpanRule&);
public:

  // Construct node
  SpanRule(const FXString& nam,const FXString& sty,const FXString& rex,FXint par,FXint idx);

  // Stylize text
  virtual FXbool stylize(const FXchar* text,FXchar *textstyle,FXint len,FXint pos,FXint& head,FXint& tail) const;

  // Stylize body, i.e. after begin pattern has been seen
  virtual FXbool stylizeBody(const FXchar* text,FXchar* textstyle,FXint len,FXint pos,FXint& head,FXint& tail) const;

  // Destructor
  virtual ~SpanRule();
  };


// Syntax for a language
class Syntax : public FXObject {
protected:
  RuleList      rules;          // Highlight rules
  FXString      language;       // Language name
  FXString      group;          // Group name for syntax coloring
  FXString      extensions;     // File extensions to recognize language
  FXString      contents;       // Contents to recognize language
  FXString      delimiters;     // Word delimiters in this language
  FXint         contextLines;   // Context lines needed for restyle
  FXint         contextChars;   // Context characters needed for restyle
  FXint         autoindent;     // Auto indent for this language
  FXint         wrapwidth;      // Wrap text at this width
  FXint         tabwidth;       // Tab is this many columns
  FXint         wrapmode;       // Wrap lines on or off
  FXint         tabmode;        // Tab key inserts spaces
protected:
  Syntax(){}
private:
  Syntax(const Syntax&);
  Syntax &operator=(const Syntax&);
public:

  // New language
  Syntax(const FXString& lang,const FXString& grp);

  // Get number of child rules
  FXint getNumRules() const { return rules.no(); }

  // Get rule
  Rule* getRule(FXint rule) const { return rules[rule]; }

  // Return true if p is ancestor of c
  FXbool isAncestor(FXint p,FXint c) const;

  // Return common ancestor of a and b
  FXint commonAncestor(FXint a,FXint b) const;

  // Language name
  void setName(const FXString& lang){ language=lang; }
  const FXString& getName() const { return language; }

  // Style coloring group
  void setGroup(const FXString& grp){ group=grp; }
  const FXString& getGroup() const { return group; }

  // Extensions
  void setExtensions(const FXString& exts){ extensions=exts; }
  const FXString& getExtensions() const { return extensions; }

  // Contents
  void setContents(const FXString& cont){ contents=cont; }
  const FXString& getContents() const { return contents; }

  // Delimiters
  void setDelimiters(const FXString& delims){ delimiters=delims; }
  const FXString& getDelimiters() const { return delimiters; }

  // Context lines
  void setContextLines(FXint num){ contextLines=num; }
  FXint getContextLines() const { return contextLines; }

  // Context characters
  void setContextChars(FXint num){ contextChars=num; }
  FXint getContextChars() const { return contextChars; }

  // Access auto-indent
  void setAutoIndent(FXint a){ autoindent=a; }
  FXint getAutoIndent() const { return autoindent; }

  // Access wrap width
  void setWrapWidth(FXint w){ wrapwidth=w; }
  FXint getWrapWidth() const { return wrapwidth; }

  // Access tab width
  void setTabWidth(FXint w){ tabwidth=w; }
  FXint getTabWidth() const { return tabwidth; }

  // Access line wrap mode
  void setWrapMode(FXint m){ wrapmode=m; }
  FXint getWrapMode() const { return wrapmode; }

  // Access tab expand mode
  void setTabMode(FXint m){ tabmode=m; }
  FXint getTabMode() const { return tabmode; }

  // Find rule index, given name
  FXint getNamedRule(const FXString& name) const;

  // Match filename against wildcards
  FXbool matchFilename(const FXString& name) const;

  // Match contents against regular expression
  FXbool matchContents(const FXString& text) const;

  // Append default rule
  FXint appendDefault(const FXString& name,const FXString& style,FXint parent=0);

  // Append simple rule
  FXint appendSimple(const FXString& name,const FXString& style,const FXString& rex,FXint parent=0);

  // Append span rule
  FXint appendSpan(const FXString& name,const FXString& style,const FXString& rex,FXint parent=0);

  // Append bracket rule
  FXint appendBracket(const FXString& name,const FXString& style,const FXString& brex,const FXString& erex,FXint parent=0);

  // Append safe bracket rule
  FXint appendSafeBracket(const FXString& name,const FXString& style,const FXString& brex,const FXString& erex,const FXString& srex,FXint parent=0);

  // Wipes the rules
  virtual ~Syntax();
  };


#endif
