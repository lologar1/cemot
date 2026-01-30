#include "cemot.h"

usf_hashmap *entries_;

static void parseEntry(char **sourcetext, u64 nlines);
static void aescopt(FILE *stream, char *sequence);

void cmt_build(char **sourcetext, u64 nlines) {
	/* Parse source dictionary and populate structures */

	entries_ = usf_newhm(); /* Maps string -> DictEntry */

	struct timespec start, end;
	fprintf(INFOSTREAM, "Parsing entries... ");
	clock_gettime(CLOCK_MONOTONIC, &start);

	char **entry;
	u64 nline, entrystart;
	for (nline = 0; nline < nlines; nline++) {
		entry = &sourcetext[entrystart = nline]; /* Get entry beginning */
		while (strcmp(sourcetext[nline], SEPARATOR_ENTRY)) nline++; /* Skip to next entry */
		parseEntry(entry, nline++ - entrystart);
	}

	clock_gettime(CLOCK_MONOTONIC, &end);
	fprintf(INFOSTREAM, "Done! (Took %f seconds)\n", usf_elapsedtimes(start, end));
}

void cmt_prompt(void) {
	/* Waits for user input and queries it */

	char query[QUERY_SIZE];
	for (;;) {
		aescopt(INFOSTREAM, AESC_COLOR_FG_GREEN); /* Query prompt color */
		fprintf(INFOSTREAM, QUERY_PROMPT);
		aescopt(INFOSTREAM, AESC_COLOR_FG_BRIGHT_GREEN); /* User input color */

		fgets(query, QUERY_SIZE, INSTREAM); /* Read user input */
		if (ferror(INSTREAM)) {
			aescopt(INFOSTREAM, AESC_COLOR_FG_BRIGHT_RED);
			fprintf(INFOSTREAM, "Error while reading input stream, aborting.\n");
			exit(CMT_EXIT_INPUTERROR);
		}
		if (feof(INSTREAM)) {
			aescopt(INFOSTREAM, AESC_COLOR_FG_YELLOW);
			fprintf(INFOSTREAM, "*** EOF ***\n");
			break;
		}
		aescopt(INFOSTREAM, AESC_COLOR_FG_DEFAULT); /* User input finished */

		if (*query) query[strlen(query) - 1] = '\0'; /* Remove newline */
		else continue; /* Empty query (shouldn't happen with buffered input) */

		cmt_query(query); /* Output answer */
	}
}

void cmt_query(char *query) {
	/* Reads and outputs query from dictionary */

	usf_supper(query); /* Keys are in UPPERCASE in entries_ */

	usf_listptr *homonyms;
	if ((homonyms = usf_strhmget(entries_, query).p) == NULL) {
		fprintf(INFOSTREAM, "No entry for \"%s\".\n", query);
		return;
	}

	u64 i;
	DictEntry *entry;
	for (i = 0; i < homonyms->size; i++) {
		entry = usf_listptrget(homonyms, i);

		aescopt(OUTSTREAM, AESC_COLOR_SET_BOLD);
		fprintf(OUTSTREAM, "%s\n", entry->name);
		aescopt(OUTSTREAM, AESC_COLOR_RESET_BOLD);

		char **line;
		aescopt(OUTSTREAM, AESC_COLOR_SET_ITALIC);
		for (line = entry->pronunciation; *line; line++) fprintf(OUTSTREAM, "%s\n", *line);
		aescopt(OUTSTREAM, AESC_COLOR_RESET_ITALIC);

		aescopt(OUTSTREAM, AESC_COLOR_FG_RGB(224, 224, 224));
		for (line = entry->definition; *line; line++) fprintf(OUTSTREAM, "%s\n", *line);
		fprintf(OUTSTREAM, SEPARATOR_QUERY"\n");
		aescopt(OUTSTREAM, AESC_COLOR_FG_DEFAULT);

	}
	fprintf(OUTSTREAM, "\n");
}

static void parseEntry(char **sourcetext, u64 nlines) {
	/* Parse and add a dictionary entry to entries_ */

	DictEntry *entry; /* Alloc DictEntry and body in the same slab */
	/* Since the name field is part of nlines but not pointed to in a section, we can subtract 1 */
	entry = malloc(sizeof(DictEntry) + ((nlines - 1 + NENTRYSECTIONS) * sizeof(char *)));

	char *entryalloc; /* Pointer to content slab */
	entryalloc = (char *) entry + sizeof(DictEntry);
#define _ENTRYALLOC(_PTR, _SIZE) \
	_PTR = (__typeof__(_PTR)) entryalloc; entryalloc += (_SIZE);

	/* Append to dictionary */
	char *entryname, *nameseparator;
	entryname = *sourcetext++;
	if ((nameseparator = strchr(entryname, ','))) *nameseparator = '\0'; /* Cut name variants */
	if ((nameseparator = strchr(entryname, ' '))) *nameseparator = '\0';

	usf_listptr *homonyms;
	if ((homonyms = usf_strhmget(entries_, entryname).p) == NULL) /* entryname is UPPERCASE from source file */
		usf_strhmput(entries_, entryname, USFDATAP((homonyms = usf_newlistptr())));
	usf_listptradd(homonyms, (void *) entry);

	/* Display name */
	usf_slower(entryname + (entryname[0] & 128 ? 2 : 1)); /* Test ASCII to skip first char */
	entry->name = entryname;

	/* Entry sections */
	u64 len, i;
#define _PARSEENTRY(_ENTRYSECTION, _LINETEST) \
	for (len = 0; _LINETEST(sourcetext[len]); len++); \
	_ENTRYALLOC(_ENTRYSECTION, (len + 1) * sizeof(char *)); \
	for (i = 0; _LINETEST(*sourcetext); i++) _ENTRYSECTION[i] = *sourcetext++; \
	_ENTRYSECTION[i] = NULL; /* End of section */

	/* Pronunciation */
#define _PRONUNCIATIONTEST(_LINE) (*(_LINE) == '\t')
	_PARSEENTRY(entry->pronunciation, _PRONUNCIATIONTEST);
#undef _PRONUNCIATIONTEST

	/* Definition */
#define _DEFINITIONTEST(_LINE) (strcmp((_LINE), SEPARATOR_ENTRY))
	_PARSEENTRY(entry->definition, _DEFINITIONTEST);
#undef _DEFINITIONTEST

#undef _PARSEENTRY
#undef _ENTRYALLOC
}

static void aescopt(FILE *stream, char *sequence) { if (isatty(fileno(stream))) fputs(sequence, stream); }
