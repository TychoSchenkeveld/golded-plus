
//  ------------------------------------------------------------------
//  GoldED+
//  Copyright (C) 1990-1999 Odinn Sorensen
//  Copyright (C) 1999-2000 Alexander S. Aganichev
//  ------------------------------------------------------------------
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License as
//  published by the Free Software Foundation; either version 2 of the
//  License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//  MA 02111-1307 USA
//  ------------------------------------------------------------------
//  $Id$
//  ------------------------------------------------------------------
//  Global utility functions (not overlayed in 16-bit DOS).
//  ------------------------------------------------------------------

#include <cstdarg>
#include <golded.h>


//  ------------------------------------------------------------------

extern bool in_arealist;
extern GPickArealist* PickArealist;


//  ------------------------------------------------------------------

void update_statuslines() {

  char buf[200];
  static char old_status_line[200] = "";
  static int called = NO;

  HandleGEvent(EVTT_REMOVEVOCBUF);

  if(CFG->switches.get(dispstatusline) or not called) {

    called = YES;

    vchar sep = _box_table(W_BSTAT, 3);
    char help[200], clkinfo[200];
    *clkinfo = NUL;
    *help = NUL;

    if(CFG->switches.get(statuslineclock)) {
      time_t t = time(NULL);
      sprintf(clkinfo, "   %s", strftimei(help, 40, LNG->StatusLineTimeFmt, glocaltime(&t)));
    }

    if(CFG->statuslinehelp == -1)
      *help = NUL;
    else if(CFG->statuslinehelp)
      sprintf(help, "%s   ", LNG->StatusLineHelp);
    else
      sprintf(help, "%s%s%s%s %s%i.%i.%i%s   ",
        __gver_prename__,
        __gver_name__,
        __gver_postname__,
        __gver_platform__,
        __gver_preversion__,
        __gver_major__,
        __gver_minor__,
        __gver_release__,
        __gver_postversion__
      );
    
    int help_len = strlen(help);
    int clk_len = strlen(clkinfo);
    int len = MAXCOL-help_len-clk_len-2;
    sprintf(buf, "%c%s%-*.*s%s ", goldmark, help, len, len, information, clkinfo);

    char *begin = buf;
    char *obegin = old_status_line;
    char *end = buf + MAXCOL;
    char *oend = old_status_line + MAXCOL;
    while((*begin != NUL) and (*begin == *obegin)) {
      ++begin;
      ++obegin;
    }
    if(begin == end)
      return;
    // we have at least one mismatch
    if(*obegin) {
      while(*end == *oend) {
        --end;
        --oend;
      }
    }
    memcpy(obegin, begin, end-begin+1);

    #ifdef GOLD_MOUSE
    gmou.GetStatus();
    if(gmou.Row() == MAXROW-1)
      gmou.HideCursor();
    #endif
    int row, col;
    vposget(&row, &col);
    *(++end) = NUL;
    wwprintstr(W_STAT, 0,begin-buf, C_STATW, begin);
    if(*help and ((begin - buf) < (help_len-1)) and ((end - buf) > (help_len-1)))
      wwprintc(W_STAT, 0,help_len-1, C_STATW, sep);
    if(((begin - buf) < (MAXCOL-clk_len)) and ((end - buf) > (MAXCOL-clk_len)))
      wwprintc(W_STAT, 0,MAXCOL-clk_len, C_STATW, sep);
    vposset(row, col);
    #ifdef GOLD_MOUSE
    if(gmou.Row() == MAXROW-1)
      gmou.ShowCursor();
    #endif
  }
}


//  ------------------------------------------------------------------

void update_statusline(const char* info) {

  strxcpy(information, info, sizeof(Subj));
  update_statuslines();
}


//  ------------------------------------------------------------------

void update_statuslinef(const char* format, ...) {

  char winfobuf[350];
  va_list argptr;
  va_start(argptr, format);
  vsprintf(winfobuf, format, argptr);
  va_end(argptr);
  update_statusline(winfobuf);
}


//  ------------------------------------------------------------------

void w_shadow() {

  if(CFG->switches.get(screenshadows))
    wshadow(C_SHADOW);
}


//  ------------------------------------------------------------------

void w_info(const char* info) {

  static int wh=-1;
  static int srow;
  static int scol;
  static int erow;
  static int ecol;
  static int len;
  static char buf[150] = { "" };
  char* buf2 = NULL;

  int prev_wh = whandle();
  if(wh != -1)
    wactiv_(wh);

  if(info) {
    int tmp = strlen(info);
    if(tmp > MAXCOL-5) {
      buf2 = (char *)throw_malloc(MAXCOL-5);
      strxcpy(buf2, info, MAXCOL-5);
      info = buf2;
      tmp = MAXCOL-6;
    }
    if(wh == -1) {
      len = tmp;
      srow = inforow;
      erow = srow+3-1;
      scol = ((MAXCOL-len)/2)-1;
      ecol = scol+len+1;
      wh = wopen(srow, scol, erow, ecol, W_BINFO, C_INFOB, C_INFOW);
      w_shadow();
    }
    else {
      if(len != tmp) {
        len = tmp;
        scol = ((MAXCOL-len)/2)-1;
        ecol = scol+len+1;
        wclose();
        wh = wopen(srow, scol, erow, ecol, W_BINFO, C_INFOB, C_INFOW);
        w_shadow();
      }
    }
    if(not streql(buf, info)) {
      strcpy(buf, info);
      wprints(0, 0, C_INFOW, buf);
    }
  }
  else {
    if(wh != -1) {
      *buf = NUL;
      wclose();
      wh = -1;
    }
  }

  wactiv_(prev_wh);

  if(buf2)
    throw_free(buf2);
}


//  ------------------------------------------------------------------


void w_infof(const char* format, ...) {

  char winfobuf[350];
  va_list argptr;
  va_start(argptr, format);
  vsprintf(winfobuf, format, argptr);
  va_end(argptr);
  w_info(winfobuf);
}


//  ------------------------------------------------------------------

void w_progress(int mode, int attr, long pos, long size, const char* title) {

  static int wh = -1;
  static long prev_pos = 0;

  int prev_wh = whandle();
  if(wh != -1)
    wactiv_(wh);

  switch(mode) {
    case MODE_NEW:
      oops:
      if(wh == -1) {
        wh = wopen_(inforow, ((MAXCOL-63)/2)-1, 3, 63, W_BINFO, C_INFOB, C_INFOW);
        set_title(title, TCENTER, C_INFOT);
        title_shadow();
        goto force_update;
      }
    case MODE_UPDATE:
      if(wh == -1)
        goto oops;  // Oops, someone forgot to open the window..
      if((pos*58/size) != (prev_pos*58/size)) {
      force_update:
        wpropbar(1, 0, 59, attr, pos, size);
      }
      prev_pos = pos;
      break;
    case MODE_QUIT:
      if(wh != -1) {
        wclose();
        wunlink(wh);
        wh = -1;
        prev_pos = 0;
      }
      break;
  }

  wactiv_(prev_wh);
}


//  ------------------------------------------------------------------

void maketitle() {

  wtitle(m_title, m_titlepos, m_titleattr);
}


//  ------------------------------------------------------------------

int maketitle_and_status(char *dir) {

  maketitle();
  update_statuslinef(LNG->ImportStatus, dir);
  return 0;
}

//  ------------------------------------------------------------------

void set_title(const char* t, int p, int a) {

  strcpy(m_title, t);
  m_titlepos  = p;
  m_titleattr = a;
}


//  ------------------------------------------------------------------

void title_shadow() {

  maketitle();
  w_shadow();
}


//  ------------------------------------------------------------------

int IsQuoteChar(const char* s) {

  if(*s) {
    if(*s == '>')
      return true;
    if(*AA->Quotechars())
      if(strchr(AA->Quotechars(), *s))
        return true;
  }
  return false;
}


//  ------------------------------------------------------------------

int is_quote(const char* ptr) {

  const char* endptr = ptr + 11;

  // Skip leading whitespace
  while((*ptr == ' ') or (*ptr == LF) or issoftcr(*ptr))
    ptr++;

  // Check for empty string
  if((*ptr == NUL) or (ptr >= endptr))
    return false;

  // Check for userdefined quotechars after first whitespace
  if(IsQuoteChar(ptr)) 
    return true;

  endptr = ptr + 11; // match 10 chars after whitespaces

  int spaces = 0;
  while((ptr < endptr) and *ptr) {

    if(*ptr == LF or issoftcr(*ptr)) {
      // Ignore LF's and SOFTCR's and extend check zone if found
      endptr++;
    }
    else if(*ptr == '>') {
      // Found true quote char
      return true;
    }
    else if(*ptr == ' ') {
      spaces++;
      if(spaces > 1)
        return false;
    }
    else if(iscntrl(*ptr) or strchr(AA->Quotestops(), *ptr)) {
      // Found a char that cannot occur in a quotestring
      return false;
    }
    ptr++;
  }

  return false;
}


//  ------------------------------------------------------------------

bool is_quote2(const Line* line, const char* ptr)
{
  if (!CFG->quoteusenewai) return true;

  char *head = (char *)ptr;

  // search first '>' before CR, NUL or any other quote character
  for (bool found = false; !found; ptr++)
  {
    if (*ptr == '>')
      found = true;
    else
    {
      if (IsQuoteChar(ptr) || (*ptr == CR) || !*ptr)
        return true;
    }
  }

  // line is double quoted?
  if (is_quote(ptr))
    return true;

  // if "SPACE*[a-zA-Z]{0, 3}>"
  for (ptr--; isspace(*head); head++);

  int nr = 0;
  for (char *tmp = head; tmp < ptr; tmp++)
  {
    char ch = toupper(*tmp);
    if ((ch >= 'A') && (ch <= 'Z'))
      nr++;
  }

  if ((nr < 4) && (nr == (ptr-head)))
    return true;

  // take a look at previous lines
  for (Line *ln = line->prev; ln; ln = ln->prev)
  {
    // previous line is quoted?
    if (ln->isquote())
      return true;
    // or begin of paragraph?
    if ((ln->txt.length() == 0) ||
        (ln->txt[0] == LF) ||
        (ln->txt[0] == CR))
      return true;
    // or kludge?
    if (ln->txt[0] == CTRL_A)
      return true;

    // found begin of citation?
    char *begin = strrchr(ln->txt.c_str(), '<');
    if (begin)
    {
      // found both '<' and '>'?
      if (strchr(begin, '>'))
        return true;

      for (Line *ln2 = ln->next; ln2 != line; ln2 = ln2->next)
      {
        // found both '<' and '>'?
        if (strchr(ln2->txt.c_str(), '>'))
          return true;
      }

      return false;   // don't quote current line
    }
  }

  return true;
}


//  ------------------------------------------------------------------

int quotecolor(const char* line) {

  char buf[MAXQUOTELEN];
  uint len;

  GetQuotestr(line, buf, &len);
  uint qc = 0;

  for(uint i=0; i<len; i++)
    if(IsQuoteChar(&buf[i]))
      qc++;

  return (qc & 1) ? C_READQ : C_READQ2;
}


//  ------------------------------------------------------------------

int GetQuotestr(const char* ptr, char* qbuf, uint* qlen) {

  if(is_quote(ptr)) {

    const char* lp = ptr;
    int n, x;

    for(;;) {

      // Skip leading spaces
      while(isspace(*lp) or issoftcr(*lp))
        lp++;
      if(IsQuoteChar(lp)) {      // Type 1 : ">xxxx>" and ">xxxx:"
        lp++;
        while(isspace(*lp) or issoftcr(*lp))
          lp++;
        if(is_quote(lp))
          continue;
        if(not (IsQuoteChar(lp-1) or (*(lp-1) == ':'))) {
          while(not IsQuoteChar(lp))
            lp--;
          lp++;
        }
      }
      else {                     // Type 2: "xxxx>"
        while(not (IsQuoteChar(lp) and not IsQuoteChar(lp+1)) and (*lp != CR) and (*lp != NUL))
          ++lp;
        if(is_quote(lp))
          continue;
        if(*lp)
          lp++;
      }

      break;
    }

    // lp now points to the character after the quotestring

    x = lp - ptr;
    if((*lp != NUL) and (isspace(*lp) or issoftcr(*lp)))
      x++;

    for(*qlen = n = 0; (n < x) and ((*qlen) < MAXQUOTELEN-1); n++, ptr++) {
      if((*ptr != LF) and not issoftcr(*ptr)) {
        *qbuf++ = *ptr;
        (*qlen)++;
      }
    }
    *qbuf = NUL;
  }
  else {
    *qbuf = NUL;
    *qlen = 0;
  }

  return *qlen;
}


//  ------------------------------------------------------------------

static int KeyCmp(const gkey* a, const gkey* b) {

  return CmpV(*a, *b);
}


//  ------------------------------------------------------------------

gkey SearchKey(gkey key, std::list<CmdKey>::iterator keys, int totkeys) {

  std::list<CmdKey>::iterator kmin;
  int again = 0;

  do {
    kmin = keys;
    int tkeys=totkeys;
    while(tkeys > 0) {
      int j = KeyCmp(&key, &(kmin->key));
      if(j == 0)
        return(kmin->cmd);
      else if(j < 0)
        break;
      else {
        kmin++;
        tkeys--;
      }
    }

    // Key not found. Try again, this time without the scancode.
    key &= 0x00FF;
  } while(not again++ and key);

  return 0;
}


//  ------------------------------------------------------------------

static void call_func_geutil(VfvCP func) {

  struct _menu_t* menu;
  int row, col;

  bool hidden = vcurhidden();
  vposget(&row, &col);
  menu = gwin.cmenu;
  (*func)();
  gwin.cmenu = menu;
  vposset(row, col);
  if(hidden)
    vcurhide();
  else
    vcurshow();
}

  
//  ------------------------------------------------------------------

void call_help() {

  // search through onkey linked list for a
  // matching defined onkey.  if one is found,
  // then save the current environment, call the
  // onkey's function, and restore the environment.

  KBnd* onkey = gkbd.onkey;
  while(onkey != NULL) {
    if(onkey->keycode == Key_F1) {
      call_func_geutil(onkey->func);
      break;
    }
    onkey = onkey->prev;
  }
}


//  ------------------------------------------------------------------

void CheckTick(gkey quitkey) {

  Clock idle_secs = (gkbd.tickvalue - gkbd.tickpress) / 10L;

  if(CFG->timeout) {
    if(((signed long)idle_secs > 0) and (idle_secs >= CFG->timeout)) {
      kbput(quitkey);
      return;
    }
  }

  IdleCheckSemaphores();

  if(CFG->screenblanker) {
    if(idle_secs >= CFG->screenblanker) {

      blanked = true;
      ScreenBlankIdle();
      kbdsettickfunc(ScreenBlankIdle);

      getxch();

      blanked = false;
      ScreenBlankIdle();
      kbdsettickfunc(update_statuslines);

      //maybe we've scanned areas while in screenblanker, so
      //update screen
      if(in_arealist) {
        PickArealist->update();
        PickArealist->do_delayed();
      }
    }
  }
}


//  ------------------------------------------------------------------

void IdleCheckSemaphores() {

  // I don't like this solution either... :(
  static Clock last_secs = 0;
  Clock idle_secs = (gkbd.tickvalue - gkbd.tickpress) / 10L;

  // Make sure the stuff below is only run once in a second
  if(not idle_secs or (idle_secs - last_secs == 0))
    return ;

  if(in_arealist) {
    if(CFG->semaphore.idletime) {
      if((idle_secs % CFG->semaphore.idletime) == 0)
        CheckSemaphores();
    }
  }

  last_secs = idle_secs;
}


//  ------------------------------------------------------------------

char* strtmp(const char* str) {

  static INam tmp;
  return strxcpy(tmp, str, sizeof(tmp));
}


//  ------------------------------------------------------------------

bool find(const std::vector<const char *> &vec, const char *str) {

  std::vector<const char *>::const_iterator i;

  for(i = vec.begin(); i != vec.end(); i++)
    if(streql(*i, str))
      return true;

  return false;
}
