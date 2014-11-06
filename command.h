/* command.h
*
*   Copyright (C) 2014       Henrik Hautakoski <henrik@fiktivkod.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
*   MA 02110-1301, USA.
*/
#ifndef COMMAND_H
#define COMMAND_H

typedef int (*cmdptr)(int argc, char **argv);

struct command {
	const char 	*cmd;
	cmdptr 		fn;
};

/* commands */
extern int cmd_compile(int argc, char **argv);
extern int cmd_run(int argc, char **argv);
extern int cmd_dlhist(int argc, char **argv);
extern int cmd_read_config(int argc, char **argv);
extern int cmd_filter_check(int argc, char **argv);
extern int cmd_version(int argc, char **argv);

#endif /* COMMAND_H */
