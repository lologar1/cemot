#include "main.h"

static void freeentrylist(void *ptr);

i32 main(i32 nargs, char *argv[]) {
	/* Initialize CMT */

	if (nargs > 1 && strcmp(argv[1], "-v")) freopen(NULLPATH, "w", INFOSTREAM);
	else { nargs--; argv++; } /* Safe since, if nargs < 2, argv is ignored as cmt_prompt is used instead */

	u64 nsourcelines;
	char **sourcetext;
	fprintf(INFOSTREAM, "Reading from dict file %s... ", DICT_PATH);
	if ((sourcetext = usf_ftost(DICT_PATH, &nsourcelines)) == NULL) {
		fprintf(INFOSTREAM, "Error while reading source file, aborting.\n");
		exit(CMT_EXIT_SOURCEDICTERROR);
	}
	fprintf(INFOSTREAM, "Done!\n");

	cmt_build(sourcetext, nsourcelines); /* Parse dict */

	i64 i; /* Signed to match legacy signed i32 type of nargs */
	if (nargs < 2) cmt_prompt(); /* Interactive */
	else for (i = 1; i < nargs; i++) cmt_query(argv[i]); /* Command-line */

	usf_freetxt(sourcetext, 1); /* usf_ftost */
	usf_freestrhmfunc(entries_, freeentrylist);

	exit(CMT_EXIT_NORMAL);
}

static void freeentrylist(void *ptr) {
	/* Frees a usf_listptr of DictEntries */

	u64 i;
	usf_listptr *list = ptr;
	for (i = 0; i < list->size; i++)
		free((DictEntry *) usf_listptrget(list, i));
	usf_freelistptr(list);
}
