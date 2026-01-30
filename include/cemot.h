#ifndef CEMOT_H
#define CEMOT_H

#include <stdio.h>
#include "usfstd.h"
#include "usfescape.h"
#include "usfio.h"
#include "usfmath.h"
#include "usfhashmap.h"
#include "usflist.h"

#define PROMPT_STREAM stderr
#define ANSWER_STREAM stdout
#define QUERY_STREAM stdin

#define QUERY_PROMPT ">"
#define QUERY_SIZE 512

#define DICT_PATH "dict/littre.txt"
#define SEPARATOR_QUERY "—————"
#define SEPARATOR_ENTRY "_____"
#define NENTRYSECTIONS 3

typedef enum Exitcode : i32 {
	CMT_EXIT_NORMAL,
	CMT_EXIT_SOURCEDICTERROR,
	CMT_EXIT_INPUTERROR
} Exitcode;

typedef struct DictEntry {
	char *name;
	char **pronunciation;
	char **definition;

} DictEntry;

void cmt_build(char **sourcetext, u64 nlines);
void cmt_query(void);

#endif
