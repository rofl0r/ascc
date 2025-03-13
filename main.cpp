#include "cs_compiler.h"
#include "cc_error.h"
#include "cs_prepro.h"
#include "cc_options.h"
#include "script_common.h"
#include "cs_parser.h"
#include "myfilestream.h"

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <limits.h>

static const char* apiv[] = {
	"350", "341", "340", "335", "334", "330", "321", "261", 0
};

static int opt_freestanding;
static const struct {
	const char *name;
	const char *help;
	int *target;
	int need_value;
} cust_f_opts[] = {
	{ "free-standing", "do not include systemheaders", &opt_freestanding, 0},
	{ 0, 0 }
};

static int usage(char *argv0) {
	fprintf(stderr, "usage: %s [OPTIONS] FILE.asc\n"
	"compiles FILE.asc to FILE.o\n"
	"OPTIONS:\n"
	"-A API version : specify API version. this affects which functions\n"
	"   and data items are made available by global header agsdefns.sh.\n"
	"   supported values are:\n  ",
	argv0);
	for(unsigned i=0;apiv[i];++i) fprintf(stderr, " %s", apiv[i]);
	fprintf(stderr, "\n   default: 350\n"
	"-a ABI version: same as above, but restricts the emitted bytecode\n"
	"-f option=value: set a compiler option. recognized options:\n"
	);
	for(unsigned i=0; i < SCOPT_NUMOPTS; ++i)
		fprintf(stderr, "   %s (1/0) - %s\n",
			ccOptionNameString(1<<i),
			ccOptionHelpString(1<<i));
	for(unsigned i=0; cust_f_opts[i].name; ++i)
		fprintf(stderr, "   %s (---) - %s\n",
			cust_f_opts[i].name, cust_f_opts[i].help);
	fprintf(stderr,
	"-i systemheaderdir : provide path to system headers\n"
	"   this is the path containing implicitly included headers (atm only agsdefns.sh).\n"
	"-H 1.ash[:2.ash...] : colon-separated list of headers to include.\n"
	"   order matters. AGS editor automatically includes all headers\n"
	"   it finds in the game dir, but here you need to specify them.\n"
	"-o filename : explicitily specify output file name\n"
	"-P preprocessor : use `preprocessor` on files before running the built-in\n"
	"   one on them. example: -P \"cpp -P\". note that you if you use a\n"
	"   standard C preprocessor, linenumber directives need to be supressed\n"
	"   as in the example cpp command.\n"
	"-D macro[=val] : define preprocessor macro\n"
	"-E : run preprocessor only (create file with .i extension)\n"
	"-W : warn about non-fatal errors parsing system headers\n"
	"-g : turn on debug info\n"
	"-S : write textual assembly instead of binary (requires agsdisas)\n"
	);
	return 1;
}

char* tempfile(void) {
	char *res = tempnam("/tmp", "ascc");
	if(!res) res = tempnam(0, "ascc");
	return res;
}

char *slurp_file(FILE *f) {
	fseek(f, 0, SEEK_END);
	unsigned len = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *mem = (char*) malloc(len+1);
	fread(mem, 1, len, f);
	mem[len] = 0;
	fclose(f);
	return mem;
}

static char *getbasename(char *filename) {
	char *q = strrchr(filename, '/');
	/* for windoze folks... */
	if(!q) q = strrchr(filename, '\\');
	if(q) q++;
	else q = filename;
	return q;
}

struct buffer {
	size_t capa;
	size_t used;
	char *mem;
};

#define ALIGN(X, A) ((X+(A-1)) & -(A))
static int buffer_append(struct buffer *b, const char* data, size_t len) {
	if(b->used+len > b->capa) {
		size_t want = ALIGN(b->used+len, 4096);
		char *n = (char*) realloc(b->mem, want);
		if(!n) return 0;
		b->mem = n;
		b->capa = want;
	}
	memcpy(b->mem+b->used, data, len);
	b->used += len;
}

static void buffer_makestring(struct buffer *b) {
	buffer_append(b, "", 1);
}

static struct buffer macrobuf;
static void add_macro(const char *name, const char *value) {
	char buf[1024];
	sprintf(buf, "#define %s %s\n", name, value);
	buffer_append(&macrobuf, buf, strlen(buf));
	ccDefineMacro(name, value);
}

static void parse_macro(char *m) {
	char *q = strchr(m, '=');
	if(q) *(q++) = 0;
	add_macro(m, q ? q : "");
}

static void preproc_add(FILE *f, const char* name, int ext)
{
	char *hdr = slurp_file(f);
	if(!ext) ccAddDefaultHeader(hdr, (char*) name);
	else buffer_append(&macrobuf, hdr, strlen(hdr));

}

static char *run_cpp(char *cppcmd, char* fc) {
	char *fn = tempfile();
	if(!fn) return 0;
	FILE *f = fopen(fn, "w");
	if(!f) return 0;
	fwrite(macrobuf.mem, 1, macrobuf.used, f);
	fwrite(fc, 1, strlen(fc), f);
	fclose(f);
	char buf[4096];
	sprintf(buf, "%s %s", cppcmd, fn);
	FILE *p = popen(buf, "r");
	if(!p) {
	cpp_err:
		fprintf(stderr, "error creating cpp process:");
		perror("");
		return 0;
	}
	struct buffer out;
	while(fgets(buf, sizeof buf, p))
		buffer_append(&out, buf, strlen(buf));
	int status = pclose(p);
	if(status == -1 || !WIFEXITED(status)) {
		goto cpp_err;
	}
	unlink(fn);
	free(fn);
	buffer_makestring(&out);
	return out.mem;
}

static int load_headers(char* headers, int ext) {
	while(*headers) {
		char *e = strchr(headers, ':');
		if(e) *e = 0;
		FILE *f = fopen(headers, "r");
		if(!f) {
			fprintf(stderr, "error: could not load header %s: ", headers);
			perror("");
			return 0;
		}
		preproc_add(f, getbasename(headers), ext);
		if(!e) break;
		else headers = e+1;
	}
	return 1;
}

static int process_custom_option(char *option, char *value) {
	for(unsigned i = 0; cust_f_opts[i].name; ++i)
		if(!strcmp(cust_f_opts[i].name, option)) {
			if(cust_f_opts[i].need_value) {
				if(!value) {
					fprintf(stderr, "error: option %s requires value\n", option);
					exit(1);
				}
				*cust_f_opts[i].target = atoi(value);
			} else
				*cust_f_opts[i].target = 1;
			return 1;
		}
	return 0;
}

static void process_compiler_option(char *option) {
	char *p = strchr(option, '=');
	if(!p) {
		if(process_custom_option(option, 0)) return;
		fprintf(stderr, "error: option %s lacks equals sign\n", option);
		exit(1);
	}
	*p = 0;
	int bit = ccOptionStringToBit(option);
	if(!bit) {
		if(process_custom_option(option, p+1)) return;
		fprintf(stderr, "error: option %s unknown\n", option);
		exit(1);
	}
	ccSetOption(bit, atoi(p+1));
}

static void set_compiler_default_options(void) {
        ccSetSoftwareVersion("3.5.0.12");
	ccSetOption(SCOPT_LINENUMBERS, 0);
        ccSetOption(SCOPT_EXPORTALL, 1);
        ccSetOption(SCOPT_NOIMPORTOVERRIDE, 0);
        ccSetOption(SCOPT_LEFTTORIGHT, 1);
        ccSetOption(SCOPT_OLDSTRINGS, 1);
}

static int search_path_bin(const char *bin, char *buf, size_t buflen) {
	char *p = getenv("PATH"), *o;
	size_t l;
	for(;;) {
		o = buf;
		l = buflen;
		while(l && *p && *p != ':') {
			*(o++) = *(p++);
			l--;
		}
		snprintf(o, l, "/%s", bin);
		if(access(buf, X_OK) == 0) return 1;
		if(*p == ':') p++;
		else if(!p) break;
	}
	return 0;
}

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
static int resolve_realname(char *buf, size_t buflen) {
	char workbuf[PATH_MAX];
	if(realpath(buf, workbuf)) {
		snprintf(buf, buflen, "%s", workbuf);
		if(access(buf, X_OK) == 0) return 1;
	}
	return 0;
}

static int search_executable_path(const char *bin, char *buf, size_t buflen) {
	snprintf(buf, buflen, "%s", bin);
	if(access(buf, X_OK) == 0 || search_path_bin(bin, buf, buflen))
		return resolve_realname(buf, buflen);
	return 0;
}


#ifdef _WIN32
#define DIRSEP '\\'
#else
#define DIRSEP '/'
#endif
static int search_executable_dir(const char *bin, char *buf, size_t buflen) {
	if(search_executable_path(bin, buf, buflen)) {
		char *p = strrchr(buf, DIRSEP);
		if(p) {
			*p = 0;
			return 1;
		}
	}
	return 0;
}

int main(int argc, char** argv) {
	char exepath[1024];
	int c;
	int only_preprocess = 0;
	int disas_flag = 0;
	char oname_buf[2048], *oname = oname_buf;
	char *headers = 0;
	char *cppcmd = 0;
	const char *req_api = "350";
	char *systemhdr_dir = NULL;
	int abi = 350;

	set_compiler_default_options();

	while((c = getopt(argc, argv, "gWESD:A:a:f:i:H:o:P:")) != EOF) switch(c) {
		case 'a': abi = atoi(optarg); break;
		case 'A': req_api = optarg; break;
		case 'i': systemhdr_dir = optarg; break;
		case 'f': process_compiler_option(optarg); break;
		case 'o': oname = optarg; break;
		case 'H': headers = optarg; break;
		case 'P': cppcmd = optarg; break;
		case 'D': parse_macro(optarg); break;
		case 'E': only_preprocess = 1; break;
		case 'W': ccPrintAllErrors = 1; break;
		case 'S': disas_flag = 1; break;
		case 'g':
			add_macro("DEBUG", "1");
			ccSetOption(SCOPT_LINENUMBERS, 1);
			ccSetOption(SCOPT_CHECKBOUNDS, 1);
			break;
		default: return usage(argv[0]);
	}
	if(!argv[optind]) return usage(argv[0]);
	if(optind +1 != argc) {
		fprintf(stderr, "error: only one filename can be passed.\n"
				"arguments after first filename:\n");
		for(c=optind+1;c<argc;c++)
			fprintf(stderr, "%s\n", argv[c]);
		return usage(argv[0]);
	}
	char *filename = argv[optind];

	add_macro("__ASCC__", "3");
	add_macro("__ASCC_MINOR__", "5");

	// Editor/AGS.Editor/AGSEditor.cs
	add_macro("AGS_SUPPORTS_IFVER", "1");

	// FIXME: these should all be toggled by options
	//add_macro("STRICT", "1");
	add_macro("LRPRECEDENCE", "1");
	//add_macro("STRICT_STRINGS", "1");
	//add_macro("STRICT_AUDIO", "1");
	add_macro("NEW_DIALOGOPTS_API", "1");

        // Don't allow them to override imports in the room script
	// FIXME needs better check for whether it's a roomfile
        ccSetOption(SCOPT_NOIMPORTOVERRIDE, !!strstr(filename, "room") );

	if(abi < 271) {
		cc_set_max_command(SCMD_SHIFTRIGHT);
		ccSetOption(SCOPT_CHECKBOUNDS, 0);
		//ccSetOption(SCOPT_EXPLICITRET, 0);
	} else {
		add_macro("AGS_NEW_STRINGS", "1");
	}

	unsigned i;
	{
		int enable_api = 0;
		char abuf[16];
		snprintf(abuf, sizeof abuf, "%s000", req_api);
		add_macro("SCRIPT_API", abuf);
		for(i=0;apiv[i];++i) {
			char buf[64];
			// enable only requested and lower APIs
			if(!enable_api) {
				if(!strcmp(req_api, apiv[i]))
					enable_api = 1;
			} else
			// do not activate 3.x APIs for 2.x and vice versa
				if(req_api[0] != apiv[i][0])
					enable_api = 0;

			if(enable_api) {
				sprintf(buf, "SCRIPT_API_v%s", apiv[i]);
				add_macro(buf, "1");
				sprintf(buf, "SCRIPT_COMPAT_v%s", apiv[i]);
				add_macro(buf, "1");
			}
		}
	}

	FILE *f;
	char filename_wo_ext[2048];
	char *p = filename, *e = strrchr(filename, '.');
	char *q = filename_wo_ext;
	if(!e) return usage(argv[0]);
	while(p < e) *(q++) = *(p++);
	*q = 0;
	q = getbasename(filename_wo_ext);

	if(!opt_freestanding) {
		snprintf(oname_buf, sizeof oname_buf, "%s/agsdefns.sh", systemhdr_dir?systemhdr_dir:"systemhdrs");
		f = fopen(oname_buf, "r");
		if(!f && !systemhdr_dir && search_executable_dir(argv[0], exepath, sizeof exepath)) {
			snprintf(oname_buf, sizeof oname_buf, "%s/%s/agsdefns.sh", exepath, "systemhdrs");
			f = fopen(oname_buf, "r");
		}
		if(!f) fprintf(stderr, "warning: default header agsdefns.sh not found, you may want to set -i to the dir containing it!\n");
		else preproc_add(f, "_BuiltInScriptHeader.ash", !!cppcmd);
	}
	if(headers && !load_headers(headers, !!cppcmd)) return 1;

	if(oname == oname_buf) {
		if(only_preprocess)
			sprintf(oname, "%s.i", filename_wo_ext);
		else if(disas_flag)
			sprintf(oname, "%s.s", filename_wo_ext);
		else
			sprintf(oname, "%s.o", filename_wo_ext);
	}

	f = fopen(filename, "r");
	if(!f) {
		fprintf(stderr, "error opening %s\n", filename);
		return 1;
	}
	char *mem = slurp_file(f);
	if(only_preprocess) {
		char *outp;
		if(cppcmd) {
			outp = run_cpp(cppcmd, mem);
			if(!outp) return 1;
		} else {
			// FIXME, buf may be to small for elab. macro expansions
			outp = (char*)malloc(strlen(mem)*2+5000);
			cc_preprocess(mem, outp);
		}
		f = fopen(oname, "w");
		if(!f) {
			fprintf(stderr, "error opening %s\n", oname);
			return 1;
		}
		fwrite(outp, 1, strlen(outp), f);
		fclose(f);
		return ccError != 0;
	} else if(cppcmd) {
		char *outp = run_cpp(cppcmd, mem);
		if(!outp) return 1;
		free(mem);
		mem = outp;
	}
	ccScript* obj = ccCompileText(mem, q);
	if(!obj || (ccError != 0)) {
		std::cerr << ccErrorString;
		std::cerr << std::endl;
		return 1;
		//std::cout << ccErrorLine;
	} else {
		char *fn;
		if(disas_flag) fn = tempfile();
		else fn = strdup(oname);
		FileWriteStream fout;
		if(!fout.Open(fn)) {
			fprintf(stderr, "error opening %s\n", fn);
			return 1;
		}
		obj->Write(&fout);
		fout.Close();
		if(disas_flag) {
			char cmd[2048 + 64], *out_tmp = tempfile();
			sprintf(cmd, "agsdisas %s %s", fn, out_tmp);
			int status = system(cmd);
			if(status == -1 || !WIFEXITED(status)) {
				fprintf(stderr, "agsdisas failed! : ");
				perror("");
				return 1;
			}
			if(rename(out_tmp, oname) == -1) {
				perror("rename");
				return 1;
			}
			unlink(fn);
		}
	}
	return 0;
}
