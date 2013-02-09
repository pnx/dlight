
#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
	TOKEN_NONE = 0,
	TOKEN_NUM,	/* Numeric constant */
	TOKEN_BOOL,	/* Boolean constant */
	TOKEN_STRING, 	/* String */
	TOKEN_ID,
	TOKEN_SEP,
	TOKEN_TAB,
	TOKEN_ASSIGN, 	/* Assignment operator */
	TOKEN_DEFINE,	/* Definition operator */
	TOKEN_RBRACKET, /* ] */
	TOKEN_LBRACKET, /* [ */
	TOKEN_VAL,	/* Arbitrary value */
	TOKEN_EOL, 	/* end-of-line */
	TOKEN_EOI 	/* end-of-input */
} token_type;

extern const char *token_str_type[];

#define token_type2str(t) (token_str_type[t])

typedef struct {
	token_type type;
	unsigned line_nr;
	void *attr;
} token_t;

void token_print(token_t token);

#endif /* TOKEN_H */