
CC = gcc
CFLAGS = -g -Wall -I..

all : dlight-compile

dlight-compile : compiler.o lexer.o token.o ../buffer.o
	$(CC) $(LDFLAGS) -o $@ $^

clean :
	$(RM) *.o dlight-compile