
#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
	NONE = 0,
	NUM,		/* Numeric constant */
	BOOL,		/* Boolean constant */
	STRING, 	/* String */
	ID,
	SEP,
	TAB,
	ASSIGN, 	/* Assignment operator */
	DEFINE,		/* Definition operator */
	RBRACKET, 	/* ] */
	LBRACKET, 	/* [ */
	VAL,		/* Arbitrary value */
	EOL, 		/* end-of-line */
	EOI 		/* end-of-input */
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