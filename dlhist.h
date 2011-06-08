
#ifndef DLHIST_H
#define DLHIST_H

int dlhist_open();

int dlhist_lookup(const char *url);

void dlhist_update(const char *url);

void dlhist_purge(unsigned int timestamp);

void dlhist_flush();

void dlhist_close();

#endif /* DLHIST */