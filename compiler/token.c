
#include <stdio.h>
#include "token.h"

const char *token_str_type[] = {
	"NONE",
	"NUMBER",
	"BOOL",
	"STRING",
	"IDENTIFIER",
	"SEPARATOR",
	"TAB",
	"ASSIGN",
	"DEFINE",
	"LEFT-BRACE",
	"RIGHT-BRACE",
	"VALUE",
	"NEWLINE",
	"END-OF-INPUT"
};

void token_print(token_t token) {

	printf("%i: %s", token.line_nr, token_type2str(token.type));

	switch (token.type) {
	case STRING :
	case VAL :
	case ID :
		if (token.attr)
			printf(" [%s]", (char*)token.attr);
	default: break;
	}

	printf("\n");
}