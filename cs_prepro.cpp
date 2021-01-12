
#include "cs_prepro.h"

#include "script_common.h"
#include "cs_parser_common.h"
#include "fmem.h"
#include "util_string_compat.h"
#include "cc_error.h"
#include "cs_compiler.h"

#include <string.h>

/* hunk here is from ags rev 89b367eda29b47a2c7163e39e0baa2c3286837aa */

// ******* BEGIN PREPROCESSOR CODE ************





int if_level = 0, if_level_active = 0, if_level_satisfied = 0;
#define all_levels_active() (if_level_active == if_level)
#define prev_level_active() (if_level_active == if_level-1)
#define set_level(X, V) do { \
		if(if_level_active > X) if_level_active = X; \
		if(if_level_satisfied > X) if_level_satisfied = X; \
		if(V != -1) { \
			if(V) if_level_active = X; \
			else if(if_level_active == X) if_level_active = X-1; \
			if(V && if_level_active == X) if_level_satisfied = X; \
		} \
		if_level = X; \
	} while(0)
#define skip_conditional_block (if_level > if_level_active)
#define deletingCurrentLine() skip_conditional_block

void get_next_word(char*stin,char*wo, bool includeDots = false) {
  if (stin[0] == '\"') {
    // a string
    int breakout = 0;
//    char*oriwo=wo;
    wo[0] = stin[0];
    wo++; stin++;
    while (!breakout) {
      if ((stin[0] == '\"') && (stin[-1] != '\\')) breakout=1;
      if (stin[0] == 0) breakout = 1;
      wo[0] = stin[0];
      wo++; stin++;
      }
    wo[0] = 0;
//    printf("Word: '%s'\n",oriwo);
    return;
    }
  else if (stin[0] == '\'') {
    // a character constant
    int breakout = 0;
    wo[0] = stin[0];
    wo++; stin++;
    while (!breakout) {
      if ((stin[0] == '\'') && (stin[-1] != '\\')) breakout=1;
      if (stin[0] == 0) breakout = 1;
      wo[0] = stin[0];
      wo++; stin++;
      }
    wo[0] = 0;
    return;
  }

  while ((is_alphanum(stin[0])) || ((includeDots) && (stin[0] == '.'))) {
    wo[0] = stin[0];
    wo++;
    stin++;
  }

  wo[0]=0;
  return;
}

extern "C" {
extern int evaluate_if_statement(char *str);
int expand_macro(char* name, char* out) {
	int ret = macros.find_name(name);
	if(ret >= 0) strcat(out, macros.macro[ret]);
	return ret >= 0;
}
void c_cc_error(char *msg) { cc_error(msg); }
}

enum directive_id {
	di_include = 0,
	di_error, // 1
	di_warning, // 2
	di_define, // 3
	di_undef, // 4
	di_if, // 5
	di_elif, // 6
	di_else, // 7
	di_ifdef, // 8
	di_ifndef, // 9
	di_endif,  // 10
	di_line,   // 11
	di_pragma,  // 12
	di_sectionstart,
	di_sectionend,
	di_ifver,
	di_ifnver,
	di_last = di_ifnver,
};

static const char* directives[] = {
	"include",
	"error",
	"warning",
	"define",
	"undef",
	"if",
	"elif",
	"else",
	"ifdef",
	"ifndef",
	"endif",
	"line",
	"pragma",
	"sectionstart",
	"sectionend",
	"ifver",
	"ifnver",
	0
};

static int directive_to_id(char *dname) {
	for(int i = 0; i <= di_last; ++i)
		if(!ags_stricmp(dname, directives[i]))
			return i;
	return -1;
}

void pre_process_directive(char*dirpt,FMEM*outfl) {
	char *p, *shal=dirpt+1;
	while(is_whitespace(*shal)) ++shal;
	p = shal;
	// seek to the end of the first word (eg. #define)
	while ((!is_whitespace(shal[0])) && (shal[0]!=0)) shal++;
	// extract the directive name
	int shalwas = shal[0];
	shal[0] = 0;
	char dname[150];
	snprintf(dname, sizeof dname, "%s", p);
	ags_strlwr(dname);
	shal[0] = shalwas;

	// skip to the next word on the line
	skip_whitespace(&shal);

	// write a blank line to the output so that line numbers are correct
	fmem_puts("",outfl);

	size_t l = strlen(dname);
	while(l && strchr(" \t\r\n", dname[l-1])) dname[--l] = 0;

	int did = directive_to_id(dname);
	if(skip_conditional_block) switch(did) {
	case di_include: case di_error: case di_warning:
	case di_define: case di_undef:
	case di_line: case di_pragma:
		return;
	default: break;
	}
	switch(did) {
	case di_error:
		cc_error("User error: %s", shal);
		return;
	case di_define: {
		char nambit[100];
		int nin=0;
		while ((!is_whitespace(shal[0])) && (shal[0]!=0)) {
			nambit[nin]=shal[0];
			nin++;
			shal++;
		}
		nambit[nin]=0;
		skip_whitespace(&shal);
		macros.add(nambit,shal);
		} break;
	case di_undef: {
		char macroToCheck[100];
		get_next_word(shal, macroToCheck);
		if (macroToCheck[0] == 0) {
			cc_error("Expected: macro");
			return;
		}

		int idx = macros.find_name(macroToCheck);
		if (idx < 0) {
			cc_error("'%s' not defined", macroToCheck);
			return;
		}

		macros.remove(idx);
		} break;
	case di_if:
		if(all_levels_active()) {
			int wantDefined = evaluate_if_statement(shal);
			set_level(if_level + 1, wantDefined);
		} else
			set_level(if_level + 1, 0);
		break;
	case di_elif:
		if(prev_level_active() && if_level_satisfied < if_level) {
			int wantDefined = evaluate_if_statement(shal);
			if(wantDefined) {
				if_level_active = if_level;
				if_level_satisfied = if_level;
			}
		} else if(if_level_active == if_level) {
			--if_level_active;
		}
		break;
	case di_else:
		if(prev_level_active() && if_level_satisfied < if_level) {
			if(1) {
				if_level_active = if_level;
				if_level_satisfied = if_level;
			}
		} else if(if_level_active == if_level) {
			--if_level_active;
		}
		break;
	case di_ifdef:
	case di_ifndef:
	case di_ifver:
	case di_ifnver: {
		int isVersionCheck = (did == di_ifver || did == di_ifnver);
		int wantDefined, neg = (did == di_ifndef || did == di_ifnver);

		char macroToCheck[100];
		get_next_word(shal, macroToCheck, true);

		if (macroToCheck[0] == 0) {
			cc_error("Expected token after space");
			return;
		}

		if ((isVersionCheck) && (!is_digit(macroToCheck[0]))) {
			cc_error("Expected version number");
			return;
		}

		if (isVersionCheck)
			wantDefined = (strcmp(ccSoftwareVersion, macroToCheck) >= 0);
		else
			wantDefined = macros.find_name(macroToCheck) >= 0;

		if (neg) wantDefined = !wantDefined;

		if(all_levels_active()) {
			set_level(if_level + 1, wantDefined);
		} else {
			set_level(if_level + 1, 0);
		}
		} break;
	case di_endif:
		if (if_level < 1) {
			cc_error("#endif without #if");
			return;
		}
		set_level(if_level-1, -1);
		break;
	case di_sectionstart:
	case di_sectionend:
		// do nothing - markers for the editor, just remove them
		break;
	default:
		cc_error("unknown/unimplemented preprocessor directive '%s'",dname);
	}
}


int in_comment=0;
void remove_comments(char*trf) {
  char tbuffer[MAX_LINE_LENGTH];
  if (in_comment) {  // already in a multi-line comment
    char*cmend=strstr(trf,"*/");
    if (cmend!=NULL) {
      cmend+=2;
      strcpy(tbuffer,cmend);
      strcpy(trf,tbuffer);
      in_comment=0;
      }
    else {
      trf[0]=0;
      return;   // still in the comment
      }
    }
  char*strpt = trf;
  char*comment_start = trf;
  while (strpt[0]!=0) {
    if ((strpt[0] == '\"') && (in_comment == 0)) {
      strpt++;
      while ((strpt[0] != '\"') && (strpt[0]!=0)) strpt++;
      if (strpt[0]==0) break;
      }

    if ((strncmp(strpt,"//",2)==0) && (in_comment == 0)) {
      strpt[0]=0;  // chop off the end of the line
      break;
      }
    if ((strncmp(strpt,"/*",2)==0) && (in_comment == 0)) {
      in_comment = 1;
      comment_start = strpt;
      strpt += 2;
    }
    if ((strncmp(strpt,"*/",2)==0) && (in_comment == 1)) {
      comment_start[0]=0;
      strcpy(tbuffer,trf);
      int carryonat = strlen(tbuffer);
      strcat(tbuffer,&strpt[2]);
      strcpy(trf,tbuffer);
      strpt = &trf[carryonat];
      in_comment = 0;
    }
    else if (strpt[0] != 0)
      strpt++;
  }
  if (in_comment) comment_start[0] = 0;

  // remove leading spaces and tabs on line
  strpt = trf;
  while ((strpt[0] == ' ') || (strpt[0] == '\t'))
    strpt++;
  strcpy(tbuffer,strpt);
  strcpy(trf,tbuffer);

  // remove trailing spaces on line
  if (strlen(trf) > 0) {
    while (trf[strlen(trf)-1]==' ') trf[strlen(trf)-1]=0;
    }
  }

void reset_compiler() {
  ccError=0;
  in_comment=0;
  currentline=0;
  }


void pre_process_line(char*lin) {

  if (deletingCurrentLine()) {
    lin[0] = 0;
    return;
  }

  char*oldl=(char*)malloc(strlen(lin)+5);
  strcpy(oldl,lin);
  char*opt=oldl;
  char charBefore = 0;

  while (opt[0]!=0) {
    while (is_alphanum(opt[0])==0) {
      if (opt[0]==0) break;
      lin[0]=opt[0]; lin++; opt++; }

    if (opt > oldl)
      charBefore = opt[-1];
    else
      charBefore = 0;

    char thisword[MAX_LINE_LENGTH];
    get_next_word(opt,thisword);
    opt+=strlen(thisword);

    int fni = -1;
    if (charBefore != '.') {
      // object.member -- if member is a #define, it shouldn't be replaced
      fni = macros.find_name(thisword);
    }

    if (fni >= 0) {
      strcpy(lin,macros.macro[fni]);
      lin+=strlen(macros.macro[fni]);
    }
    else {
      strcpy(lin,thisword);
      lin+=strlen(thisword);
    }

  }
  free(oldl);
  lin[0]=0;
  }

// preprocess: takes source INPU as input, and copies the preprocessed output
//   to outp. The output has all comments, #defines, and leading spaces
//   removed from all lines.
void cc_preprocess(const char *inpu,char*outp) {
  reset_compiler();
  FMEM*temp=fmem_create();
  FMEM*iii=fmem_open(inpu);
  char linebuffer[1200];
  currentline=0;
  if_level = if_level_active = if_level_satisfied = 0;

  while (!fmem_eof(iii)) {
    fmem_gets(iii,linebuffer);
    currentline++;
    if (strlen(linebuffer) >= MAX_LINE_LENGTH) {  // stop array overflows in subroutines
      cc_error("line too long (%d chars max)", MAX_LINE_LENGTH);
      break; 
    }

    remove_comments(linebuffer);
    // need to check this, otherwise re-defining a macro causes
    // a big problem:
    // #define a int, #define a void  results in #define int void
    if (linebuffer[0]!='#')
      pre_process_line(linebuffer);
    if (ccError) break;
    if (linebuffer[0]=='#')
      pre_process_directive(linebuffer,temp);
    else {  // don't output the #define lines
//      printf("%s\n",linebuffer);
      fmem_puts(linebuffer,temp);
      }
    if (ccError) break;
    }

  strcpy(outp,temp->data);
  fmem_close(temp);
  fmem_close(iii);

  if ((if_level) && (!ccError))
    cc_error("missing #endif");

}




// ***** END PREPROCESSOR CODE *********


void preproc_startup(MacroTable *preDefinedMacros) {
    macros.init();
    if (preDefinedMacros)
        macros.merge(preDefinedMacros);
}

void preproc_shutdown() {
    macros.shutdown();
}
