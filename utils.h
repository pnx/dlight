
#ifndef UTILS_H
#define UTILS_H

/* compare two files at a system level.
   Returns a non zero value if a and b are the same file. zero otherwise.
   ("same" in this context is the same location on the filesystem) */
int file_cmp(const char *a, const char *b);

#endif /* UTILS_H */