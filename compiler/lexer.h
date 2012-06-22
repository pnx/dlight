
#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include "token.h"

token_t lexer_getnext(FILE *fd);

#endif