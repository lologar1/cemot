#ifndef CEMOT_H
#define CEMOT_H

#include <stdio.h>
#include <unistd.h>
#include <usfstd.h>

#include "usfaesc.h"
#include "usfio.h"
#include "usfmath.h"
#include "usfhashmap.h"
#include "usflist.h"

#include "main.h"

#define QUERY_PROMPT ">"
#define QUERY_SIZE 512

#define DICT_PATH "dict/littre.txt"
#define SEPARATOR_QUERY "—————"
#define SEPARATOR_ENTRY "_____"
#define NENTRYSECTIONS 2

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

extern usf_hashmap *entries_;

void cmt_build(char **sourcetext, u64 nlines);
void cmt_prompt(void);
void cmt_query(char *query);

#endif
