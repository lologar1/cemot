#include "cemot.h"

i32 main(i32 nargs, char *argv[]) {
	/* Initialize CMT */

	(void) nargs;
	(void) argv;

	u64 nsourcelines;
	char **sourcetext;
	fprintf(stderr, "Reading from dict file %s... ", DICT_PATH);
	if ((sourcetext = usf_ftost(DICT_PATH, &nsourcelines)) == NULL) {
		fprintf(stderr, "Error while reading source file, aborting.\n");
		exit(CMT_EXIT_SOURCEDICTERROR);
	}
	fprintf(stderr, "Done!\n");

	cmt_build(sourcetext, nsourcelines); /* Parse dict */
	cmt_query(); /* Listen to user */

	usf_freetxt(sourcetext, 1); /* usf_ftost */
	exit(CMT_EXIT_NORMAL);
}
