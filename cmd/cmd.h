#ifndef CMD_CMD_H
#define CMD_CMD_H

// XXX something to properly parse flags is needed

#include <io/io.h>

extern char *cmd_disk;
extern char *cmd_cmd_name;

int cmd_format(int, char**);
int cmd_cat(int, char**);
int cmd_cp(int, char**);

char *cmd_islocal(char *name);
IoFile *cmd_open_disk(void);
IoFile *cmd_create_disk(void);

IoFile *cmd_walk_at(IoFile *root, char *name);
IoFile *cmd_create_at(IoFile *root, char *name);
int cmd_mkdir_at(IoFile *root, char *name);

#endif