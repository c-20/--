#include "code.h"
#include <dirent.h>
#include <errno.h>

#define dashdash main

typedef struct _List {
  CS item; // struct in struct? item.name, item.size
//  CS type;
  IN itemlen;
  CH sort;
  IN index;
  struct _List *prev;
  struct _List *next;
} List;

List *listadditem(List *list, CS item, CH sort) {
  List *new = (List *)calloc(1, sizeof(List));
  new->itemlen = STRLEN(item);
  new->item = STRMEM(new->itemlen);
  STR1(new->item, item);
//  new->type = type;
  new->sort = sort;
//  new->index = index;
 // new->prev = list; // prev is current list item (maybe NULL)
 // IF (new->prev NQNULL) // not first item?
 //   { new->prev->next = new; } // set next of prev
  IF (list EQNULL) { // list is empty - first item
    new->index = 0;
    new->prev = NULL;
    new->next = NULL;
  } EF (list->next NQNULL) { // adding into list here
Ys("intolist..");
Mi(list->index);
Wc(list->sort);
    new->prev = list->prev;
    IF (new->prev) { // this index = prev + 1
      new->index = new->prev->index + 1;
    } EL { new->index = 0; } // no prev, first
    new->next = list;
    list->prev = new;
//    List *ln = new->next;
//    WI (ln NQNULL) {
//Cs("*");
//      INC ln->index;
//      ln = ln->next;
//    } // increment all subsequent indexes
  } EL { // adding to end of list
    new->prev = list;
    IF (new->prev) { // this index = prev + 1
      new->index = new->prev->index + 1;
      new->prev->next = new;
    } // EL empty list .. match earlier
    EL { new->index = 0; Rs("notexpected"); } // not expected!
    new->next = NULL;
  } // ^ list was empty, should match earlier
  WI (new->prev NQNULL AND new->sort LT new->prev->sort) {
Ws("sort");
    List *lp = new->prev;
    lp->next = new->next;
    new->prev = lp->prev;
    lp->prev = new;
    new->next = lp;
    IN swapindex = lp->index;
    lp->index = new->index;
    new->index = swapindex;
    IF (lp->next)
      { lp->next->prev = lp; }
    IF (new->prev)
      { new->prev->next = new; }
  } // sort by sort type 1, 2, 3, 4
  RT new; // point to new item
}

List *listgotolast(List *list) {
  IF (!list) { RT NULL; }
  WI (list->next NQNULL)
    { list = list->next; }
  RT list;
}

List *listgotofirst(List *list) {
  IF (!list) { RT NULL; }
  WI (list->prev NQNULL)
    { list = list->prev; }
  RT list;
}

IN dashdash($) {
  List *dirs = NULL;
  CS dirlocked = "dirlocked";
  CS diropen   = "diropen";
  List *links = NULL;
  CS linksym  = "linksym";
  CS linkfifo = "linkfifo";
  CS linklnk  = "linklnk";
  List *files = NULL;
  CS fileempty = "fileempty";
  CS filesmall = "filesmall";
  CS filelarge = "filelarge";
  List *data = NULL;
  CS datapng = "datapng";
  CS datamp3 = "datamp3";
  CS dataavi = "dataavi";
  IN maxstrlen = 0;
  CH matchinlinetags = N; // -I (on), -i (off)
  CH displaytags = N;     // -[] (on), -() (off)
  CH supertags = N;       // -[[(cat:name.tag)]]
  CH colourfile = N;      // -RYGCBMrygcbmWLDK0N
  CH colourtags = N;      // -[]RYGCBMrygcbmWLDK0N
  WI ($N GT 0) {
    CH tagparam = N;
    WI ($1[0] EQ '-') {
      IFEQEQ($1[1], '[', $1[2], ']')
        { displaytags = Y; tagparam = Y; NEXTDARG2CH($1); }
      EFEQEQ($1[1], '(', $1[2], ')')
        { displaytags = N; tagparam = Y; NEXTDARG2CH($1); }
      EFEQEQ($1[1], '[', $1[2], '[') {
        IFEQEQ($1[3], ']', $1[4], ']')
          { displaytags = Y; supertags = Y; NEXTDARG4CH($1); }
        EL { Rs("TODO: search for category: or .tag"); RT 100; }
      } EFEQ($1[1], 'I') { matchinlinetags = Y; NEXTDARGCH($1); }
        EFEQ($1[1], 'i') { matchinlinetags = N; NEXTDARGCH($1); }
      EFEQ6($1[1], 'R', 'Y', 'G', 'C', 'B', 'M') { // bright colours
        IF (tagparam EQ Y) { colourtags = $1[1]; }
        EL { colourfile = $1[1]; } ; NEXTDARGCH($1);
      } EFEQ6($1[1], 'r', 'y', 'g', 'c', 'b', 'm') { // dark colours
        IF (tagparam EQ Y) { colourtags = $1[1]; }
        EL { colourfile = $1[1]; } ; NEXTDARGCH($1);
      } EFEQ4($1[1], 'W', 'L', 'D', 'K') {           // grey colours
        IF (tagparam EQ Y) { colourtags = $1[1]; }
        EL { colourfile = $1[1]; } ; NEXTDARGCH($1);
      } EFEQ2($1[1], '0', 'N') {                     // no colour
        IF (tagparam EQ Y) { colourtags = N; }
        EL { colourfile = N; } ; NEXTDARGCH($1);
      } EL { ERR1("Invalid argument %c", $1[1]); RT 1; }
      IF ($1[1] EQNUL) { SHIFT_1; }
    }
    DIROBJECT dir = OPENDIR($1);
    DIRENTITY dent;
    IF (dir) {
      IN stage = 1;
      LOOP {
//      WI ((dent = READDIR(dir)) NQNULL) {
        dent = READDIR(dir);
        IF (dent EQNULL) {
          IF (stage LT 3) {
            REWINDDIR(dir);
            INC stage; // 1->2,2->3,3=BK
            dent = READDIR(dir);
          } EL { BK; } // end of stage 3
        }
        CS name = dent->d_name;
        IF STREQ(name, ".")  { CT; } // skip .
        IF STREQ(name, "..") { CT; } // skip ..
        IN namelen = STRLEN(name);
        UCH type = dent->d_type;
        IF (stage EQ 1) {
          IFEQ4(type, DT_DIR, DT_SOCK, DT_CHR, DT_BLK) {
            IN sort = 'B'; // b for blocked, B for open
            dirs = listadditem(dirs, name, sort);
            IF (namelen GT maxstrlen)
              { maxstrlen = namelen; }
          }
        } EF (stage EQ 2) {
          IFEQ2(type, DT_LNK, DT_FIFO) {
            IN sort = (type EQ DT_LNK ) ? 'C' :
                      (type EQ DT_FIFO) ? 'Y' : '0';
            links = listadditem(links, name, sort);
            IF (namelen GT maxstrlen)
              { maxstrlen = namelen; }
          }
        } EF (stage EQ 3) {
          IFEQ(type, DT_REG) {
            IN sort = 'G';
            files = listadditem(files, name, sort);
            IF (namelen GT maxstrlen)
              { maxstrlen = namelen; }
            // data files should go into the data list
          }
        }
      }
      closedir(dir);
      // can now traverse the lists
      CH strformat[30];
      STRF(strformat, "%%s%%%ds%%s\n", maxstrlen);
      List *d = listgotofirst(dirs);
      WI (d) {
        LOGF(strformat, COLOURSTR(d->sort), d->item, OFFC);
IF (d EQ d->next) { Rs("SELFLOOP"); RT 100; }
        d = d->next;
      }
      List *l = listgotofirst(links);
      WI (l) {
        LOGF(strformat, COLOURSTR(l->sort), l->item, OFFC);
        l = l->next;
      }
      List *f = listgotofirst(files);
      WI (f) {
        LOGF(strformat, COLOURSTR(f->sort), f->item, OFFC);
        f = f->next;
      }
      
//    } EF (errno EQ ENOTDIR) {
// THE ABOVE ONLY HAPPENS IF WE OPEN A DIRECTORY
    } EF (OPENDIRNOTADIR) {
// [] [[]] add tags ... -[]R to colourise
      FS file = OPENFILE($1);
      IN lastch = EOF;
      CH inatag = N;
      IN tagnestlevel = 0;
      IF (file) {
        LOOP {
          IN ch = GETFCH(file);
          BKEQEOF(ch);
          IF (inatag EQ N AND ch EQ '[') {
            IF (lastch EQ EOF) { // start of file
              inatag = Y;     // [first line tag!]
              IF (displaytags EQ Y) {
                IF (colourtags NQ N) {
                  IF (supertags EQ Y)
                    { Ws("[["); } EL { Ws("["); }
                } EL {
                  IF (supertags EQ Y)
                    { _s("[["); } EL { _s("["); }
                }
              }
            } EF (lastch EQ '\n') { // [ of \n[
              inatag = Y;     // \n[next line tag!]
              IF (displaytags EQ Y) {
                IF (colourtags NQ N) {
                  IF (supertags EQ Y)
                    { Ws("[["); } EL { Ws("["); }
                } EL {
                  IF (supertags EQ Y)
                    { _s("[["); } EL { _s("["); }
                }
              }
            } EF (matchinlinetags EQ Y) {
              inatag = Y;     // ..[inline tag]
              IF (displaytags EQ Y) {
                IF (colourtags NQ N) {
                  IF (supertags EQ Y)
                    { Ws("[["); } EL { Ws("["); }
                } EL {
                  IF (supertags EQ Y)
                    { _s("[["); } EL { _s("["); }
                }
              }
            } EL { // this is not part of a tag
              IF (colourfile NQ N) {
                cc(colourfile, '[');
              } EL { PUTSTDCH('['); }
            }
          } EF (inatag EQ Y AND ch EQ '[') {
            INC tagnestlevel;
            IF (colourtags NQ N) {
              cc(colourtags, '[');
            } EL { _c('['); }
          } EF (inatag EQ Y AND ch EQ ']') {
            IF (tagnestlevel GT 0) {
              DEC tagnestlevel;
              IF (colourtags NQ N) {
                cc(colourtags, ']');
              } EL { _c(']'); }
            } EL {
              inatag = N;       // tag ends here
              IF (displaytags EQ Y) {
                IF (colourtags NQ N) {
                  IF (supertags EQ Y)
                    { Ws("]]"); } EL { Ws("]"); }
                } EL {
                  IF (supertags EQ Y)
                    { _s("]]"); } EL { _s("]"); }
                }
              } // EL could send an EOF marker .. CH127?
                // such that CH127CH127 becomes CH127
                // which means an empty file needs to be
                // CH127 NUL CH127 to prevent mixup
            }
          // } EF (inatag EQ N AND ch EQ ']') {
            // just a normal char ... 
          } EL {
            IF (inatag EQ Y) { // from [ to ]
              IF (displaytags EQ Y) {
                IF (colourtags NQ N) {
                  cc(colourtags, ch);
                } EL { _c(ch); }
              }
            } EL {
              IF (colourfile NQ N) {
                cc(colourfile, ch);
              } EL { PUTSTDCH(ch); }
            }
          }
          lastch = ch;
        }
        CLOSEFILE(file);
      } EL { RT1("open fail\n"); }
       // ELSE display error tag \n[!...] ? R!G!B!
       // OR display file as a NUL only
// OTHERWISE CAT THE FILE AS NORMAL
    }
    $$1;
  }
  RT 0;
}

// later: -R file.txt -G file.txt -B file.txt
// later: -- -c12,24,36 for columns
// soon: -- . in columns with file status for colour
//     -- empty: dark
//     -- link:  cyan
//     -- dir:   blue
//     -- tiny:  light
//     -- small: white    - up to 1MB, no compression
//     -- data:  magenta  - png, avi, mp3
//      -- png: red, avi: blue, mp3: green
//      --- stills: red, sound: green, movies: blue
//      --- slideshows: orange, animations: cyan

