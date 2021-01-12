extern int expand_macro(char* name, char* out);
extern int defined_macro(char *name);
extern void c_cc_error(char* msg);

#define error(X) c_cc_error(X)

#include "tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int x_tokenizer_next_of(struct tokenizer *t, struct token *tok, int fail_unk) {
	int ret = tokenizer_next(t, tok);
	if(tok->type == TT_OVERFLOW) {
		error("max token length of 4095 exceeded!");
		return 0;
	} else if (fail_unk && ret == 0) {
		error("tokenizer encountered unknown token");
		return 0;
	} else if (tok->type == TT_SEP && tok->value == '\t') {
		tok->value = ' ';
	}
	return 1;
}

#define tokenizer_next(T, TOK) x_tokenizer_next_of(T, TOK, 0)
#define x_tokenizer_next(T, TOK) x_tokenizer_next_of(T, TOK, 1)

static int is_whitespace_token(struct token *token)
{
	return token->type == TT_SEP &&
		(token->value == ' ' || token->value == '\t');
}

/* skips until the next non-whitespace token (if the current one is one too)*/
static int eat_whitespace(struct tokenizer *t, struct token *token, int *count) {
	*count = 0;
	int ret = 1;
	while (is_whitespace_token(token)) {
		++(*count);
		ret = x_tokenizer_next(t, token);
		if(!ret) break;
	}
	return ret;
}

/* return index of matching item in values array, or -1 on error */
static int expect(struct tokenizer *t, enum tokentype tt, const char* values[], struct token *token)
{
	int ret;
	do {
		ret = tokenizer_next(t, token);
		if(ret == 0 || token->type == TT_EOF) goto err;
	} while(is_whitespace_token(token));

	if(token->type != tt) {
err:
		error("unexpected token");
		return -1;
	}
	int i = 0;
	while(values[i]) {
		if(!strcmp(values[i], t->buf))
			return i;
		++i;
	}
	return -1;
}

/* fetches the next token until it is non-whitespace */
static int skip_next_and_ws(struct tokenizer *t, struct token *tok) {
	int ret = tokenizer_next(t, tok);
	if(!ret) return ret;
	int ws_count;
	ret = eat_whitespace(t, tok, &ws_count);
	return ret;
}

#define TT_LAND TT_CUSTOM+0
#define TT_LOR TT_CUSTOM+1
#define TT_LTE TT_CUSTOM+2
#define TT_GTE TT_CUSTOM+3
#define TT_SHL TT_CUSTOM+4
#define TT_SHR TT_CUSTOM+5
#define TT_EQ TT_CUSTOM+6
#define TT_NEQ TT_CUSTOM+7
#define TT_LT TT_CUSTOM+8
#define TT_GT TT_CUSTOM+9
#define TT_BAND TT_CUSTOM+10
#define TT_BOR TT_CUSTOM+11
#define TT_XOR TT_CUSTOM+12
#define TT_NEG TT_CUSTOM+13
#define TT_PLUS TT_CUSTOM+14
#define TT_MINUS TT_CUSTOM+15
#define TT_MUL TT_CUSTOM+16
#define TT_DIV TT_CUSTOM+17
#define TT_MOD TT_CUSTOM+18
#define TT_LPAREN TT_CUSTOM+19
#define TT_RPAREN TT_CUSTOM+20
#define TT_LNOT TT_CUSTOM+21

#define TTINT(X) X-TT_CUSTOM
#define TTENT(X, Y) [TTINT(X)] = Y

static int bp(int tokentype) {
	static const int bplist[] = {
		TTENT(TT_LOR, 1 << 4),
		TTENT(TT_LAND, 1 << 5),
		TTENT(TT_BOR, 1 << 6),
		TTENT(TT_XOR, 1 << 7),
		TTENT(TT_BAND, 1 << 8),
		TTENT(TT_EQ, 1 << 9),
		TTENT(TT_NEQ, 1 << 9),
		TTENT(TT_LTE, 1 << 10),
		TTENT(TT_GTE, 1 << 10),
		TTENT(TT_LT, 1 << 10),
		TTENT(TT_GT, 1 << 10),
		TTENT(TT_SHL, 1 << 11),
		TTENT(TT_SHR, 1 << 11),
		TTENT(TT_PLUS, 1 << 12),
		TTENT(TT_MINUS, 1 << 12),
		TTENT(TT_MUL, 1 << 13),
		TTENT(TT_DIV, 1 << 13),
		TTENT(TT_MOD, 1 << 13),
		TTENT(TT_NEG, 1 << 14),
		TTENT(TT_LNOT, 1 << 14),
		TTENT(TT_LPAREN, 1 << 15),
//		TTENT(TT_RPAREN, 1 << 15),
//		TTENT(TT_LPAREN, 0),
		TTENT(TT_RPAREN, 0),
	};
	if(TTINT(tokentype) < sizeof(bplist)/sizeof(bplist[0])) return bplist[TTINT(tokentype)];
	return 0;
}

static int expr(struct tokenizer *t, int rbp, int *err);

static int charlit_to_int(const char *lit) {
	if(lit[1] == '\\') switch(lit[2]) {
		case '0': return 0;
		case 'n': return 10;
		case 't': return 9;
		case 'r': return 13;
		case 'x': return strtol(lit+3, NULL, 16);
		default: return lit[2];
	}
	return lit[1];
}

static int nud(struct tokenizer *t, struct token *tok, int *err) {
	switch((unsigned) tok->type) {
		case TT_IDENTIFIER: return 0;
		case TT_WIDECHAR_LIT:
		case TT_SQSTRING_LIT:  return charlit_to_int(t->buf);
		case TT_HEX_INT_LIT:
		case TT_OCT_INT_LIT:
		case TT_DEC_INT_LIT:
			return strtol(t->buf, NULL, 0);
		case TT_NEG:   return ~ expr(t, bp(tok->type), err);
		case TT_PLUS:  return expr(t, bp(tok->type), err);
		case TT_MINUS: return - expr(t, bp(tok->type), err);
		case TT_LNOT:  return !expr(t, bp(tok->type), err);
		case TT_LPAREN: {
			int inner = expr(t, 0, err);
			if(0!=expect(t, TT_RPAREN, (const char*[]){")", 0}, tok)) {
				error("missing ')'");
				return 0;
			}
			return inner;
		}
		case TT_FLOAT_LIT:
			error("floating constant in preprocessor expression");
			*err = 1;
			return 0;
		case TT_RPAREN:
		default:
			error("unexpected token");
			*err = 1;
			return 0;
	}
}

static int led(struct tokenizer *t, int left, struct token *tok, int *err) {
	int right;
	switch((unsigned) tok->type) {
		case TT_LAND:
		case TT_LOR:
			right = expr(t, bp(tok->type), err);
			if(tok->type == TT_LAND) return left && right;
			return left || right;
		case TT_LTE:  return left <= expr(t, bp(tok->type), err);
		case TT_GTE:  return left >= expr(t, bp(tok->type), err);
		case TT_SHL:  return left << expr(t, bp(tok->type), err);
		case TT_SHR:  return left >> expr(t, bp(tok->type), err);
		case TT_EQ:   return left == expr(t, bp(tok->type), err);
		case TT_NEQ:  return left != expr(t, bp(tok->type), err);
		case TT_LT:   return left <  expr(t, bp(tok->type), err);
		case TT_GT:   return left >  expr(t, bp(tok->type), err);
		case TT_BAND: return left &  expr(t, bp(tok->type), err);
		case TT_BOR:  return left |  expr(t, bp(tok->type), err);
		case TT_XOR:  return left ^  expr(t, bp(tok->type), err);
		case TT_PLUS: return left +  expr(t, bp(tok->type), err);
		case TT_MINUS:return left -  expr(t, bp(tok->type), err);
		case TT_MUL:  return left *  expr(t, bp(tok->type), err);
		case TT_DIV:
		case TT_MOD:
			right = expr(t, bp(tok->type), err);
			if(right == 0)  {
				error("eval: div by zero");
				*err = 1;
			}
			else if(tok->type == TT_DIV) return left / right;
			else if(tok->type == TT_MOD) return left % right;
			return 0;
		default:
			error("eval: unexpect token");
			*err = 1;
			return 0;
	}
}

static int tokenizer_peek_next_non_ws(struct tokenizer *t, struct token *tok)
{
	int ret;
	while(1) {
		ret = tokenizer_peek_token(t, tok);
		if(is_whitespace_token(tok))
			x_tokenizer_next(t, tok);
		else break;
	}
	return ret;
}

static int expr(struct tokenizer *t, int rbp, int*err) {
	struct token tok;
	int ret = skip_next_and_ws(t, &tok);
	if(tok.type == TT_EOF) return 0;
	int left = nud(t, &tok, err);
	while(1) {
		ret = tokenizer_peek_next_non_ws(t, &tok);
		if(bp(tok.type) <= rbp) break;
		ret = tokenizer_next(t, &tok);
		if(tok.type == TT_EOF) break;
		left = led(t, left, &tok, err);
	}
	(void) ret;
	return left;
}

static int do_eval(struct tokenizer *t, int *result) {
	tokenizer_register_custom_token(t, TT_LAND, "&&");
	tokenizer_register_custom_token(t, TT_LOR, "||");
	tokenizer_register_custom_token(t, TT_LTE, "<=");
	tokenizer_register_custom_token(t, TT_GTE, ">=");
	tokenizer_register_custom_token(t, TT_SHL, "<<");
	tokenizer_register_custom_token(t, TT_SHR, ">>");
	tokenizer_register_custom_token(t, TT_EQ, "==");
	tokenizer_register_custom_token(t, TT_NEQ, "!=");

	tokenizer_register_custom_token(t, TT_LT, "<");
	tokenizer_register_custom_token(t, TT_GT, ">");

	tokenizer_register_custom_token(t, TT_BAND, "&");
	tokenizer_register_custom_token(t, TT_BOR, "|");
	tokenizer_register_custom_token(t, TT_XOR, "^");
	tokenizer_register_custom_token(t, TT_NEG, "~");

	tokenizer_register_custom_token(t, TT_PLUS, "+");
	tokenizer_register_custom_token(t, TT_MINUS, "-");
	tokenizer_register_custom_token(t, TT_MUL, "*");
	tokenizer_register_custom_token(t, TT_DIV, "/");
	tokenizer_register_custom_token(t, TT_MOD, "%");

	tokenizer_register_custom_token(t, TT_LPAREN, "(");
	tokenizer_register_custom_token(t, TT_RPAREN, ")");
	tokenizer_register_custom_token(t, TT_LNOT, "!");

	int err = 0;
	*result = expr(t, 0, &err);
#ifdef DEBUG
	dprintf(2, "eval result: %d\n", *result);
#endif
	return !err;
}

char* emit_token(char* out, struct token *tok, char *strbuf) {
	if(tok->type == TT_SEP)
		sprintf(out, "%c", tok->value);
	else if(strbuf)
		strcat(out, strbuf);
	return out + strlen(out);
}

static FILE* tokenizer_from_string(struct tokenizer *t, char* str) {
	FILE *mstream = fmemopen(str, strlen(str), "r");
	tokenizer_init(t, mstream, 0);
	return mstream;
}


static int evaluate_condition(struct tokenizer *t, int *result) {
	int ret, backslash_seen = 0;
	struct token curr;
	char buf[1024], *bufp = buf;
	*bufp = 0;
#if 0
	ret = tokenizer_next(t, &curr);
	if(!ret) return ret;
	if(!is_whitespace_token(&curr)) {
		error("expected whitespace after if/elif");
		return 0;
	}
#endif
	// first pass: expand all macros and defined(foo)
	while(1) {
		ret = tokenizer_next(t, &curr);
		if(!ret) return ret;
		if(curr.type == TT_IDENTIFIER) {
			if(!strcmp(t->buf, "defined")) {
				ret = tokenizer_next(t, &curr);
				if(!ret || !(curr.type == TT_SEP && curr.value == '(')) {
					error("expected ( after defined");
					return ret;
				}
				ret = tokenizer_next(t, &curr);
				if(!ret || curr.type != TT_IDENTIFIER) {
					error("expected identifier after defined(");
					return ret;
				}
				char *ou = defined_macro(t->buf) ? "1" : "0";
				strcat(bufp, ou);
				ret = tokenizer_next(t, &curr);
				if(!ret || !(curr.type == TT_SEP && curr.value == ')')) {
					error("expected ) after defined(ID");
					return ret;
				}
			} else
				expand_macro(t->buf, bufp);
			bufp += strlen(bufp);
		} else if(curr.type == TT_SEP) {
			if(curr.value == '\\')
				backslash_seen = 1;
			else {
				if(curr.value == '\n') {
					if(!backslash_seen) break;
				} else {
					bufp = emit_token(bufp, &curr, t->buf);
				}
				backslash_seen = 0;
			}
		} else {
			bufp = emit_token(bufp, &curr, t->buf);
		}
	}
	if(strlen(buf) == 0) {
		error("#(el)if with no expression");
		return 0;
	}
#ifdef DEBUG
	dprintf(2, "evaluating condition %s\n", bufp);
#endif
	// second pass: evaluate expanded contents
	struct tokenizer t2;
	FILE *f = tokenizer_from_string(&t2, buf);
	ret = do_eval(&t2, result);
	fclose(f);
	return ret;
}


int evaluate_if_statement(char *str) {
	struct tokenizer t;
	FILE *m = tokenizer_from_string(&t, str);
	int res, ret = evaluate_condition(&t, &res);
	fclose(m);
	return ret ? res : 0;
}
