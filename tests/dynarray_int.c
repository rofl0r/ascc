int sv_count=0;
int sv[];
export sv_count, sv;

void SaveVariable(int value,int id) {
        int val;
        int sv_new[];
        if(id+1 > sv_count) {
                sv_new = new int[id+100];
                for(val = 0; val < sv_count; val++)
                        sv_new[val] = sv[val];
                sv = null;
                sv_count = id+100;
                sv = sv_new;
        }
        sv[id] = value;
}

int ReadVariable(int id) {
        int val;
        if(id >= sv_count || id < 0) return 0;
	val = sv[id];
	return val;
}

