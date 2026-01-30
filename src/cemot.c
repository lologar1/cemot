#include "cemot.h"

usf_hashmap *entries_;

static void parseEntry(char **sourcetext, u64 nlines);

void cmt_build(char **sourcetext, u64 nlines) {
	/* Parse source dictionary and populate structures */

	entries_ = usf_newhm(); /* Maps string -> DictEntry */

	struct timespec start, end;
	fprintf(PROMPT_STREAM, "Parsing entries... ");
	clock_gettime(CLOCK_MONOTONIC, &start);

	char **entry;
	u64 nline, entrystart;
	for (nline = 0; nline < nlines; nline++) {
		entry = &sourcetext[entrystart = nline]; /* Get entry beginning */
		while (strcmp(sourcetext[nline], SEPARATOR_ENTRY)) nline++; /* Skip to next entry */
		parseEntry(entry, nline++ - entrystart);
	}

	clock_gettime(CLOCK_MONOTONIC, &end);
	fprintf(PROMPT_STREAM, "Done! (Took %f seconds)\n", usf_elapsedtimes(start, end));
}

void cmt_query(void) {
	/* Waits for user input and queries it into entries_ */

	char query[QUERY_SIZE];
	for (;;) {
		fprintf(PROMPT_STREAM, AESC_COLOR_FG_GREEN QUERY_PROMPT AESC_COLOR_FG_BRIGHT_GREEN);
		fgets(query, QUERY_SIZE, QUERY_STREAM);
		fprintf(PROMPT_STREAM, AESC_COLOR_FG_DEFAULT);

		if (ferror(QUERY_STREAM)) {
			fprintf(PROMPT_STREAM, "Error while reading input stream, aborting.\n");
			exit(CMT_EXIT_INPUTERROR);
		}
		if (feof(QUERY_STREAM)) {
			fprintf(PROMPT_STREAM, "*** EOF ***\n");
			break;
		}

		query[strlen(query) - 1] = '\0'; /* QUERY_STREAM buffered input: newline guaranteed */
		usf_supper(query); /* Keys are in UPPERCASE in entries_ */

		usf_listptr *homonyms;
		if ((homonyms = usf_strhmget(entries_, query).p) == NULL) {
			fprintf(PROMPT_STREAM, "No entry for \"%s\".\n", query);
			continue;
		}

		u64 i;
		DictEntry *entry;
		for (i = 0; i < homonyms->size; i++) {
			entry = usf_listptrget(homonyms, i);

			fprintf(ANSWER_STREAM, AESC_COLOR_SET_BOLD"%s\n"AESC_COLOR_RESET_BOLD, entry->name);

			char **line;
			for (line = entry->pronunciation; *line; line++)
				fprintf(ANSWER_STREAM, AESC_COLOR_SET_ITALIC"%s\n"AESC_COLOR_RESET_ITALIC, *line);

			for (line = entry->definition; *line; line++)
				fprintf(ANSWER_STREAM, AESC_COLOR_FG_RGB(224, 224, 224)"%s\n"AESC_COLOR_FG_DEFAULT, *line);
			fprintf(ANSWER_STREAM, "\n"SEPARATOR_QUERY"\n");
		}
		fprintf(ANSWER_STREAM, "\n");
	}
}

static void parseEntry(char **sourcetext, u64 nlines) {
	/* Parse and add a dictionary entry to entries_ */

	u64 entrysz; /* Allocate at least as many bytes as in the original text, plus size for pointer arrays */
	entrysz = (u64) (*(sourcetext + nlines) - *sourcetext); /* Valid since sourcetext is contiguous */
	entrysz += (nlines + NENTRYSECTIONS) * sizeof(char *); /* Pointer arrays */

	DictEntry *entry; /* Alloc DictEntry and body in the same slab */
	entry = malloc(sizeof(DictEntry) + entrysz);

	char *entryalloc; /* Pointer to content slab */
	entryalloc = (char *) entry + sizeof(DictEntry);
	u64 len, i;
#define _ENTRYALLOC(_PTR, _SIZE) \
	_PTR = (__typeof__(_PTR)) entryalloc; entryalloc += (_SIZE);

	/* Name */
	char *entryname, *nameseparator;
	entryname = *sourcetext++; /* Entry titles are in UPPERCASE in the source file */
	if ((nameseparator = strchr(entryname, ','))) *nameseparator = '\0'; /* Cut name variants */

	len = strlen(entryname) + 1; /* Include \0 terminator */
	_ENTRYALLOC(entry->name, len);
	strcpy(entry->name, entryname);
	usf_slower(entry->name + (entry->name[0] & 128 ? 2 : 1)); /* Test ASCII to skip first char */

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

	/* Append to dictionary */
	usf_listptr *homonyms;
	if ((homonyms = usf_strhmget(entries_, entryname).p) == NULL)
		usf_strhmput(entries_, entryname, USFDATAP((homonyms = usf_newlistptr())));

	usf_listptradd(homonyms, (void *) entry);
}
