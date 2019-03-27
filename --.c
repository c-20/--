#include "code.h"
#include <dirent.h>
#include <errno.h>

#define dashdash main

#define MAXBOUNDARYSTRLEN	1024
#define MAXHEADERSTRLEN		4096

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

VD displayfilechar(CH ch, CH displaynormalchars, CH displayctrlchars, CH displayextended,
                          CH colourfile, CH colourctrlchars, CH colourextended) {
  IF (displayextended EQ Y AND ch GT ('~' + 1)) {
    IF (colourextended NQ N) {
      cc(colourextended, ch);
    } EL { PUTSTDCH(ch); }
  } EF (displaynormalchars EQ Y AND EQ2(ch, '\n', '\t')) {
    PUTSTDCH(ch); // \n maps to * as ctrl char ... override and display as normal
  } EF (displayctrlchars EQ Y AND ch LT ' ' OR ch GT '~') {
    IF (colourctrlchars NQ N) {
      cc(colourctrlchars, ((EQ2(ch, '~' + 1, NUL) ? 1 : ch) + ' ')); // \0 -> !
    } EL { PUTSTDCH(ch); } // display ctrl chars as ctrl chars
  } EF (displaynormalchars EQ Y) {
    IF (colourfile NQ N) { // normal chars in default/normal file colour
      cc(colourfile, ch); // write char in file colour
    } EL { PUTSTDCH(ch); } // write char with no colour
  } // EL displaynormalchars et al could redirect to outputs ........
}

// #define SUPERTAG, NORMALTAG START END: "[[" "]]", "[" "]"
//  defines SUPERTAGSTART, SUPERTAGEND, NORMALTAGSTART, NORMALTAGEND
#define NORMALTAGSTART       "["
#define NESTEDTAGSTART       "["
#define DATATAGSTART         "<"
#define SUPERTAGSTART        "[["
#define SUPERDATATAGSTART    "[<"
#define SUPERNESTEDTAGSTART  "[["
#define NORMALENDTAGSTART    "[/"
#define SUPERENDTAGSTART     "[[/"
#define DATAENDTAGSTART      "</"
#define SUPERDATAENDTAGSTART "[</"
#define NORMALTAGEND         "]"
#define NESTEDTAGEND         "]"
#define DATATAGEND           ">"
#define SUPERTAGEND          "]]"
#define SUPERDATATAGEND      ">]"
#define SUPERNESTEDTAGEND    "]]"
//#define NESTEDENDTAGSTART      "[/"   there are no nested end tags .... !
//#define SUPERNESTEDENDTAGSTART "[[/"  there are no nested end tags .... !
// -- TAGS ------------------------------------------------------------------------------------
VD displaytagstart(CH displaytags, CH colourtags, CH supertags, CH colourtagmarkers) {
  IF (displaytags EQ Y) {
    CS tag = (supertags EQ Y) ? SUPERTAGSTART : NORMALTAGSTART;
    IF (colourtags NQ N) {
      cs(colourtagmarkers, tag);
    } EL { PUTSTDSTR(tag); }
  } // EL could send a SOF marker .. CH0?
}
VD displaytagchar(CH ch, CH displaytags, CH colourtags) {
  IF (displaytags EQ Y) {
    IF (colourtags NQ N) {
      cc(colourtags, ch);
    } EL { PUTSTDCH(ch); }
  }
}
VD displaytagnul(CH displaytags, CH displaytagnuls, CH colourtags, CH colourtagmarkers) {
   IF (displaytagnuls EQ Y) { // by default, show the NUL as a |
     IF (colourtags NQ N) {
       cc(colourtagmarkers, '|'); // pipe | replaces NUL \0 for coloured tags
     } EL { PUTSTDCH(NUL); } // otherwise display the NUL
  } // displaytagnuls is enabled by default, but so is colour. disable tag colour for NUL
} // can be used for datatags with displaydatatagnuls -> displaytagnuls
VD displayendtagstart(CH displaytags, CH colourtags, CH supertags, CH colourtagmarkers) {
  IF (displaytags EQ Y) {
    CS tag = (supertags EQ Y) ? SUPERENDTAGSTART : NORMALENDTAGSTART;
    IF (colourtags NQ N) {
      cs(colourtagmarkers, tag);
    } EL { PUTSTDSTR(tag); }
  } // EL could send an EOF marker .. CH127? .. should send after matching end tag
}
VD displaytagend(CH displaytags, CH colourtags, CH supertags, CH colourtagmarkers) {
  IF (displaytags EQ Y) {
    CS tag = (supertags EQ Y) ? SUPERTAGEND : NORMALTAGEND;
    IF (colourtags NQ N) {
      cs(colourtagmarkers, tag);
    } EL { PUTSTDSTR(tag); }
  }
}
// -- NESTED TAGS -----------------------------------------------------------------------------
VD displaynestedtagstart(CH displaytags, CH colourtags, CH supertags, CH colournestedtagmarkers) {
  IF (displaytags EQ Y) {
    CS tag = (supertags EQ Y) ? SUPERNESTEDTAGSTART : NESTEDTAGSTART;
    IF (colourtags NQ N) {
      cs(colournestedtagmarkers, tag);
    } EL { PUTSTDSTR(tag); }
  }
}
VD displaynestedtagend(CH displaytags, CH colourtags, CH supertags, CH colournestedtagmarkers) {
  IF (displaytags EQ Y) {
    CS tag = (supertags EQ Y) ? SUPERNESTEDTAGEND : NESTEDTAGEND;
    IF (colourtags NQ N) {
      cs(colournestedtagmarkers, tag);
    } EL { PUTSTDSTR(tag); }
  }
}
// -- DATA TAGS -------------------------------------------------------------------------------
VD displaydatatagstart(CH displaydatatags, CH colourtags, CH supertags, CH colourtagmarkers) {
  IF (displaydatatags EQ Y) {
    CS tag = (supertags EQ Y) ? SUPERDATATAGSTART : DATATAGSTART;
    IF (colourtags NQ N) {
      cs(colourtagmarkers, tag);
    } EL { PUTSTDSTR(tag); }
  } // EL could send a SOF marker .. CH0?
}
VD displaydatachar(CH ch, CH displaydata, CH colourdata, CH colourextended, CH colourctrlchars) {
  IF (displaydata EQ Y) {
    IF (colourdata NQ N) {
      IF (ch GT ('~' + 1)) { // extended characters use extended char colours
        IF (colourextended NQ N) {
          cc(colourextended, ch);
        } EL { PUTSTDCH(ch); }
      } EF (ch LT ' ' OR ch GT '~') { // includes \n -> * when printing data
        IF (colourctrlchars NQ N) {
          cc(colourctrlchars, ((EQ2(ch, '~' + 1, NUL) ? 1 : ch) + ' '));
        } EL { PUTSTDCH(ch); } // display ctrl chars as ctrl chars
      } EL { cc(colourdata, ch); }
    } EL { PUTSTDCH(ch); } // do not colour extended or ctrl if non-colour data
  } // EL data is muted but could be sent to other outputs '1' '2' '3' etc
}
VD displaydataendtagstart(CH displaydatatags, CH colourtags, CH supertags, CH colourtagmarkers) {
  IF (displaydatatags EQ Y) {
    CS tag = (supertags EQ Y) ? SUPERDATAENDTAGSTART : DATAENDTAGSTART;
    IF (colourtags NQ N) {
      cs(colourtagmarkers, tag);
    } EL { PUTSTDSTR(tag); }
  } // EL could send EOF ... displaydatatagend will also be called... (but also called for the start tag)
  // will only be called if the end boundary was matching
}
VD displaydatatagend(CH displaydatatags, CH colourtags, CH supertags, CH colourtagmarkers) {
  IF (displaydatatags EQ Y) {
    CS tag = (supertags EQ Y) ? SUPERDATATAGEND : DATATAGEND;
    IF (colourtags NQ N) {
      cs(colourtagmarkers, tag);
    } EL { PUTSTDSTR(tag); }
  } // EL could send an EOF marker .. CH127?
}
// -- MAIN PROGRAM ----------------------------------------------------------------------------
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
  CH matchinlinetags = Y; // -I (on), -i (off) -- includes inline data tags!
//  CH displaytags = N;      // -[] (on), -() (off)
  CH displaynormalchars = Y; // N will mute normal output and only display ctrl/extended/tags/data
  CH displayctrlchars = Y;   // display control chars (by default in colour, NUL -> ' ', '~' + 1 -> ' ')
  CH displayextended = Y;    // display extended chars (not remapped)
  CH displaytags = Y;     // DISPLAY BY DEFAULT (else cannot write .ini)
  CH displaytagnuls = Y;  // enabled by default, but colourtags NQ N means display a 'W' pipe
  CH displayendtags = Y;  // normal tags can have end tags [tag] bla [/tag] -- todo: match end tags!
  CH displaydatatagnuls = N; // [data]data[/data] vs [\0data\0]data[/data\0] .. colourtags=N if enabled
  CH displaydataheaders = Y; // display data header content, footer will be []
  CH displaydatastartboundaries = Y; // otherwise the [x]...[] data tag will be empty      []
  CH displaydataendboundaries = Y;   // otherwise the []...[/x] end data tag will be empty [/]
  // could allow [\0x\0]...[\0/x\0] but no need .. can use start/end match to generated <tags></tags>
  CH displaydataendtags = Y;  // [\0bound\0]data[/bound\0] data end tags have a NUL before ]
  CH displaydata = Y;     //                              ^^ should only match top level if closes required
// to change data tags to <data></data>, need to check the char after [ .. [\0 = data tag, [/ = end tag
  CH supertags = N;       // -[[(cat:name.tag)]]          ^^ data end tags include boundary ...
  CH colourfile = N;      // -RYGCBMrygcbmWLDK0N          ^^ normal end tags could be checked the same way
  CH colourtags = N;      // -[]RYGCBMrygcbmWLDK0N        ^^ check from tree list of boundaries to match
  CH colourtagmarkers = 'W';  // default to use if colourtags is enabled
  CH colournestedtagmarkers = 'D'; // dark instead of white
  CH colourctrlchars = 'Y';   // yellow control chars
  CH colourextended = 'R';    // red extended chars
  CH colourdataheaders = 'B'; // blue data header chars
  CH colourdatatags = 'M';    // magenta data tag chars - surrounding [] [/] = 'W' if colourtags NQ N
  CH colourdata = 'C';        // cyan data - uses ctrlchars,extended colours if NQ N
  // cannot read stdin before parameters because
  //  INPUTPIPE does not become false ... use fileno() ?
  WI ($N GT 0 OR INPUTPIPE) { // extra iteration for stdin
    CH tagparam = N;
    WI ($N GT 0 AND $1[0] EQ '-') {
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
      } EFEQ3($1[1], '0', 'N', 'n') {                // no colour
        IF (tagparam EQ Y) { colourtags = N; }
        EL { colourfile = N; } ; NEXTDARGCH($1);
      } EL { ERR1("Invalid argument %c", $1[1]); RT 1; }
      IF ($1[1] EQNUL) { SHIFT_1; }
    }
    DIROBJECT dir = NULL;
    FS file = NULL; // could become set to stdin
    IF ($N GT 0) {
      dir = OPENDIR($1);
    } // else not a dirobject, open inputpipe if available later
    //DIROBJECT dir = OPENDIR($1);
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
      //SHIFT_1; // dir parameter handled /.. $$1 later
//    } EF (errno EQ ENOTDIR) {
// THE ABOVE ONLY HAPPENS IF WE OPEN A DIRECTORY
    } EF (OPENDIRNOTADIR OR INPUTPIPE) { // includes stdin
// [] [[]] add tags ... -[]R to colourise
//      IF (INPUTPIPE) { // close when finished
//        file = stdin;
      // cannot read stdin before params because fclose(stdin)
      //  does not change the response of INPUTPIPE
      IF ($N GT 0) { // open if available
        file = OPENFILE($1);
      } EF (INPUTPIPE) { // else use stdin if available
        file = stdin;
      } // EL no params, no input.. outer loop should have exited
      IN lastch = EOF;
      CH inatag = N;
      CH inadatatag = N;
      CH inadataheader = N;
      CH inadatablock = N;
      IN inadatablockend = -3; // not in a data block end
      CH boundarystr[MAXBOUNDARYSTRLEN + 1];
      CS boundarych = boundarystr;
      CH headerstr[MAXHEADERSTRLEN + 1];
      CS headerch = headerstr;
      CS boundarymatchch = NULL; // set when matching begins
      IN tagnestlevel = 0;
      IF (file) {
        LOOP {
          IN ch = GETFCH(file);
          BKEQEOF(ch);
// ------ (non-nested) tag starts here ....... [ in [tag] .. not [[, which is inatag already ---------
          IF (inatag EQ N AND ch EQ '[') { // match first [ .. no display in case of data [\0 or end [/
            IF (lastch EQ EOF) { // start of file
              inatag = Y;     // [first line tag!]
            } EF (lastch EQ '\n') { // [ of \n[
              inatag = Y;     // \n[next line tag!]
            } EF (matchinlinetags EQ Y) { // includes inlinedatatags which requires inatag EQ Y
              inatag = Y;     // ..[inline tag]
            } EL { // this is not part of a tag (nor data, which is inatag + inadatatag)
              displayfilechar('[', displaynormalchars, displayctrlchars, displayextended,
                                   colourfile, colourctrlchars, colourextended);
            } // displaytagstart will be called at first tag char (unless NUL or / follows)
// ------ handle nested tags (tags in tags) [tags [tags] [tags] [tags tags]] -------------------------
          } EF (inatag EQ Y AND inadatatag EQ N AND ch EQ '[') {
            INC tagnestlevel;
            displaynestedtagstart(displaytags, colourtags, supertags, colournestedtagmarkers);
// ------ handle tag ends ] - including nested tags ends ]]] -- .. tag ends when nesting reaches 0 ---
          } EF (inatag EQ Y AND inadatatag EQ N AND ch EQ ']') {
            IF (tagnestlevel GT 0) {
              DEC tagnestlevel;
              displaynestedtagend(displaytags, colourtags, supertags, colournestedtagmarkers);
            } EL {
              inatag = N;       // tag ends here
              IF (lastch EQ '[' AND tagnestlevel EQ 0) { // this is an empty tag
                displaytagstart(displaytags, colourtags, supertags, colourtagmarkers);
              } // display the delayed tag start char since first tag char can't trigger it
              displaytagend(displaytags, colourtags, supertags, colourtagmarkers);
            }
          // } EF (inatag EQ N AND ch EQ ']') { -- normal char
// ------ a / after the [ in a tag triggers an end tag  ----------------------------------------------
          } EF (inadatatag EQ N AND inatag EQ Y AND ch EQ '/') { // match [/ (+ handle bad /)
            IF (lastch EQ '[' AND tagnestlevel EQ 0) { // nested tags cannot have ends ......
//              inanendtag = Y; // tags after tags can have ends matching from the top-down stack (todo)
DRs("()"); // need to detect which tag is being ended ....
              displayendtagstart(displaytags, colourtags, supertags, colourtagmarkers);
              // ^ instead of displaytagstart() (displaytagstart would change [/bla] ends to [bla])
              // once ] is reached, inatag will become false .... should check at that point
              //  if the end tag content matched any start tags ........
            } EF (lastch EQ '[') { // nested tags can begin with / .. they are not end tags
              //displaynestedtagstart(displaytags, colourtags, colourtagmarkers);
              // ^ nestedtagstart does not get delayed because there are no nested end tags
              displaytagchar('/', displaytags, colourtags);
            } EL { // allow / within tags as long as it is not the first char (else it is an end tag)
              displaytagchar('/', displaytags, colourtags);
            }
// ------ a NUL after the [ in a tag triggers a data tag  ----------------------------------------------
          } EF (inadatatag EQ N AND inatag EQ Y AND ch EQ NUL) { // match [\0 (+ handle bad \0)
            IF (lastch EQ '[' AND tagnestlevel EQ 0) { // prevent [[[\0data and [tag\0data
              inadatatag = Y; // prefix [\0 detected
              displaydatatagstart(displaytags, colourtags, supertags, colourtagmarkers);
 // consider using displaydatatags EQ Y ^ instead of displaytags
              boundarych = boundarystr;
              headerch = headerstr;
              *boundarych = *headerch = NUL; // empty strings
              displaytagnul(displaytags, displaydatatagnuls, colourtags, colourtagmarkers); // | in colour
              // stays inadatatag until closing boundary tag
            } EL { // cannot have data tags nested in other tags or with a [PREFIX\0bound\0header]
              displaytagnul(displaytags, displaytagnuls, colourtags, colourtagmarkers);
            } // still inatag, tagnestlevel remains where it was, NUL ignored as data trigger
// ------ any other char in a tag is a tag char ... tag start displayed before first ch ----------------
          } EF (inadatatag EQ N AND inatag EQ Y) { // match any tag char except NUL and / handled above
            IF (lastch EQ '[' AND tagnestlevel EQ 0) { // this is the char after the [
              displaytagstart(displaytags, colourtags, supertags, colourtagmarkers);
              displaytagchar(ch, displaytags, colourtags);              
            } EL { // ^ first char of non-nested tags get delayed in case of <data> or [/tag]
              displaytagchar(ch, displaytags, colourtags);
            } // tag chars displayed here !
// ------ data tag boundary reading until \0 ----- 'datatag\0' in [\0datatag\0] ------------------------
          } EF (inadatatag EQ Y AND inadataheader EQ N AND inadatablock EQ N) {
            // data boundary continues until NUL is reached, then inadataheader until inadatablock
            // note that the datablockend is in the datablock until the ] is reached
            IF (ch EQ NUL) { // suffix \0 after prefix [\0 detected
              *boundarych = NUL; // end of boundary string
              inadataheader = Y; // optional header string
              displaytagnul(displaytags, displaydatatagnuls, colourtags, colourtagmarkers); // | in colour
              // ^ send the post-boundary data start tag NUL character
            } EF (boundarych LT (boundarystr + MAXBOUNDARYSTRLEN)) {
              *boundarych = ch; // this is a char of boundary data
              INC boundarych;
              *boundarych = NUL; // safe string if limit reached
              // boundary is being stored until NUL is received
              // it will be displayed when the first header char is reached, or the ]
            } EL { Rs("boundary too long"); RT 101; } // disallow
          } EF (inadataheader EQ Y) { // header ends when ] received (consider \])
// -------- data tag header reading and matching ] in [\0boundary\0(OPTIONALHEADERDATA)] ----------------
            IF (ch EQ ']') {
              IF (displaydatastartboundaries EQ Y) {
                IF (colourdatatags NQ N) {
                  cs(colourdatatags, boundarystr);
                } EL { PUTSTDSTR(boundarystr); }
              }
              displaydatatagend(displaytags, colourtags, supertags, colourtagmarkers);
              // ^ end of data start tag and end of data end tag use the same function
              inadataheader = N; // header stops here
              inadatablock = Y;  // data starts here
            } EF (headerch LT (headerstr + MAXHEADERSTRLEN)) {
              *headerch = ch; // this is a char of header data
              INC headerch;
              *headerch = NUL; // safe string if limit reached
              IF (displaydataheaders EQ Y) {
                IF (colourdataheaders NQ N) {
                  cc(colourdataheaders, ch); // .. in colour
                } EL { PUTSTDCH(ch); } // show the header character
              } // EL displaydataheaders could point to output ports '1', '2', '3' etc ...
            } EL { Rs("header too long"); RT 102; } // disallow
// ------------ data end boundary trigger matching ------ '[/' in [/boundary\0] ---------------------
          } EF (inadatablock EQ Y AND ch EQ '[') {
            inadatablockend = -1; // half boundary prefix [ matched
          } EF (inadatablock EQ Y AND inadatablockend EQ -1 AND ch EQ '/') { // EQ2(,,NUL)
            inadatablockend = 0;  // boundary prefix [\0 matched
            boundarymatchch = boundarystr; // ready to match
          } EF (inadatablock EQ Y AND inadatablockend EQ -1 AND ch EQ NUL) { // EQ2(,,NUL)
            DMs("()"); // ignoring nested data boundary .....
            displaydatachar('[', displaydata, colourdata, colourextended, colourctrlchars);
            inadatablockend = -3; // inanesteddatablock = 1;
            // still in a data block ....
            displaydatachar(NUL, displaydata, colourdata, colourextended, colourctrlchars);
          } EF (inadatablock EQ Y AND inadatablockend EQ -1) { // cancel the end match.. AND ch NQ '/') {
            displaydatachar('[', displaydata, colourdata, colourextended, colourctrlchars);
            inadatablockend = -3; // not in a data block end
            // still in a data block ... still in a data tag
            displaydatachar(ch, displaydata, colourdata, colourextended, colourctrlchars);
// ------------- data end boundary string matching --- 'boundary\0' in [/boundary\0] ----------------
          } EF (inadatablock EQ Y AND inadatablockend GQ 0) {
            IF (ch EQNUL) { // end of boundary [/boundary\0 ... if unexpected NUL, cancel match
              IF (*boundarymatchch EQNUL) { // the boundary matches
                inadatablockend = -2; // waiting for ']'
                // boundarymatchch waits in case ] does not follow NUL
              } EL { // was expecting more boundary chars .. non-matching boundary
                inadatablockend = -3; // not in a data block end
                displaydatachar('[', displaydata, colourdata, colourextended, colourctrlchars);
                displaydatachar('/', displaydata, colourdata, colourextended, colourctrlchars);
                CS boundarynomatchch = boundarystr;
                WI (boundarynomatchch LT boundarymatchch) { // write all matched chars
                  displaydatachar(*boundarynomatchch, displaydata, colourdata, colourextended, colourctrlchars);
                  INC boundarynomatchch;
                } // until non-matching char
                boundarymatchch = boundarystr; // this will reset at next [\0 anyway
                // still in a data block ... still in a data tag
                displaydatachar(NUL, displaydata, colourdata, colourextended, colourctrlchars);
              } // end of code to handle the second NUL in a data footer tag
            } EL { // reading boundary ...
              IF (*boundarymatchch EQNUL) { // original boundary string ends earlier
                inadatablockend = -3; // not in a data block end
                // still in a data block ..... still in a data tag
              } EF (*boundarymatchch EQ ch) { // this character matches
                INC boundarymatchch; // ready for the next match (NUL checked later)
                INC inadatablockend; // increment block end counter
              } EL { // this is a non-matching boundary char..
                inadatablockend = -3; // not in a data block end
                displaydatachar('[', displaydata, colourdata, colourextended, colourctrlchars);
                displaydatachar('/', displaydata, colourdata, colourextended, colourctrlchars);
                CS boundarynomatchch = boundarystr;
                WI (boundarynomatchch LT boundarymatchch) { // write all matched chars
                  displaydatachar(*boundarynomatchch, displaydata, colourdata, colourextended, colourctrlchars);
                  INC boundarynomatchch;
                } // until non-matching char
                boundarymatchch = boundarystr; // this will reset at next [\0 anyway
                // still in a data block ... still in a data tag
                displaydatachar(ch, displaydata, colourdata, colourextended, colourctrlchars);
              } // end of code to match boundary chars until NUL is reached
            } // end of boundary checker, matching chars until NUL triggers a state change
// ------------ data end[/boundary\0] --- matching the ], changing state --------------------------
          } EF (inadatablock EQ Y AND inadatablockend EQ -2 AND ch EQ ']') {
            inadatablockend = -3; // no longer in a data block end
            inadatablock = N; // the data block has ended .... trigger an event ?
            inadatatag = N; // the data tag has ended .... trigger an event ?
            inatag = N;       // tag ends here --- back to normal file
            IF (displaydataendtags EQ Y) { // different from end tags ... that would be inatagend (todo)
              displaydataendtagstart(displaytags, colourtags, supertags, colourtagmarkers);
              // displaydatatags ? ^ start of end tag is [/ (or [[/ for super)
              IF (displaydataendboundaries EQ Y) {
                IF (colourdatatags NQ N) {
                  cs(colourdatatags, boundarystr);
                } EL { PUTSTDSTR(boundarystr); }
                displaytagnul(displaytags, displaydatatagnuls, colourtags, colourtagmarkers); // | in colour
                // ^ send the post-boundary data end tag NUL character '\0' in [/boundary\0
              }
              displaydatatagend(displaytags, colourtags, supertags, colourtagmarkers);
              // ^ displaydatatags ? ^ end of start tag and end of end tag uses the same function
              // not escaping means [\0\0 and [\0] and [] need to be evaluated carefully
            } // EL could send an EOF marker .. CH127?
// ------------ data end[/boundary\0] --- ] expected... boundary in data! display as data ---------------
          } EF (inadatablock EQ Y AND inadatablockend EQ -2 AND ch NQ ']') {
            inadatablockend = -3; // not in a data block end because boundary\0] expected
            displaydatachar('[', displaydata, colourdata, colourextended, colourctrlchars);
            displaydatachar('/', displaydata, colourdata, colourextended, colourctrlchars);
            CS boundarynomatchch = boundarystr;
            WI (boundarynomatchch LT boundarymatchch) { // write all matched chars
              displaydatachar(*boundarynomatchch, displaydata, colourdata, colourextended, colourctrlchars);
              INC boundarynomatchch;
            } // until non-matching char
            boundarymatchch = boundarystr; // this will reset at next [\0 anyway
            // still in a data block ... still in a data tag
            displaydatachar(NUL, displaydata, colourdata, colourextended, colourctrlchars);
            displaydatachar(ch, displaydata, colourdata, colourextended, colourctrlchars);
            // ^ display the non-] data char that triggered this mess
            // consider returning an error instead - unexpected boundary in data (with prefix [/)
// ------------ printing of normal file text + data text ---------------------------------------
          } EL { // just a normal char ... (could be in a tag (not a data tag (but could be in data block)))
            IF (inatag EQ Y) { // char within a tag from [ to ]
              IF (inadatablock EQ Y) { // should trigger after boundary is read \0].......
                displaydatachar(ch, displaydata, colourdata, colourextended, colourctrlchars);
                // if extended/control chars have no colour, they will be displayed as normal
                // they will also be displayed as normal if colourdata EQ N
//            } EF (inadataheader EQ Y) { // headers matched earlier for copying...
              } EL {
Rs("NOTEXPECTED!!!!!!");
                displaytagchar(ch, displaytags, colourtags);
              }
            } EL { // EL display a normal char .... that is not in a tag (therefore also not data)
              displayfilechar(ch, displaynormalchars, displayctrlchars, displayextended,
                                  colourfile, colourctrlchars, colourextended);
            }
          }
          lastch = ch;
        }
    //    IF (file NQ stdin) // stdin stays open ... or should close once processed?
        CLOSEFILE(file); // stdin will be closed once processed ...
        IF (file EQ stdin) { BK; } // break after stdin because INPUTPIPE still true
      } EL { RT1("open fail\n"); }
       // ELSE display error tag \n[!...] ? R!G!B!
       // OR display file as a NUL only
// OTHERWISE CAT THE FILE AS NORMAL
    }
    IF ($N GT 0 AND file NQ stdin) // stdin should only match once.. unless pipe is reopened
      { SHIFT_1; } // discard parameter if not processed as dir or file
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

