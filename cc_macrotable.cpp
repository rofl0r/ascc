
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cc_macrotable.h"
#include "cc_error.h"

void MacroTable::init() {
    num = 0;
    memset(name, 0, sizeof(name));
    memset(macro, 0, sizeof(macro));
}
void MacroTable::shutdown() {
    int rr;
    for (rr=0;rr<num;rr++) {
        free(macro[rr]);
        free(name[rr]);
        macro[rr]=NULL;
        name[rr]=NULL;
    }
    num = 0;
}
void MacroTable::merge(MacroTable *others) {

    for (int aa = 0; aa < others->num; aa++) {
        this->add(others->name[aa], others->macro[aa]);
    }

}
int MacroTable::find_name(char* namm) {
    int ss;
    for (ss=0;ss<num;ss++) {
        if (strcmp(namm,name[ss])==0) return ss;
    }
    return -1;
}
void MacroTable::add(char*namm,char*mac) {
    if (find_name(namm) >= 0) {
        cc_error("macro '%s' already defined",namm);
        return;
    }
    if (num>=MAXDEFINES) {
        cc_error("too many macros defined");
        return;
    }
    name[num]=(char*)malloc(strlen(namm)+5);
    strcpy(name[num],namm);
    macro[num]=(char*)malloc(strlen(mac)+5);
    strcpy(macro[num],mac);
    size_t l = strlen(mac);
    // remove trailing whitespace, newlines and the like.
    // if a macro ends with a newline (as usual when passed from parser)
    // this would otherwise cause the expanded macro to mess up reported
    // linenumbers.
    while(l > 0 && isspace(macro[num][l-1]))
        macro[num][--l] = 0;
    num++;
}
void MacroTable::remove(int index) {
    if ((index < 0) || (index >= num)) {
        cc_error("MacroTable::Remove: index out of range");
        return;
    }
    // just blank out the entry, don't bother to remove it
    name[index][0] = 0;
    macro[index][0] = 0;
}

MacroTable macros;
