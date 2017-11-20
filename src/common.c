#include "common.h"

/* Initializes hash table before it has been used */
htab_t * initialization(const char * file_path, token_buffer * token_buff)
{
	FILE * input_file;
	if ((input_file = fopen(file_path, "r")) == NULL) {
		fprintf(stderr, "We could not open the file: %s\n", file_path);
		exit(ERR_CODE_LEXICAL);
	}

	int err_line = 1, err_pos = 0;

	init_garbage_collector();

	int state = 0;
	while ( state != EOF) {
		token * t;
		t = get_token(input_file, &err_line, &err_pos);
		if (t->type == NEW_LINE) {
			err_line += 1;
			err_pos = 0;
		}
		add_token(token_buff, t);
		state = t->type;
	}

	fclose(input_file);
	return htab_init(token_buff->len);
}

/* Initializes output file and puts generated assembler code to it*/
void output_primal_code(const char * file_path, String * primal_code)
{
	FILE * output_file;
	if ((output_file = fopen(file_path, "w")) == NULL) {
		fprintf(stderr, "We could not open the file: %s\n", file_path);
		exit(100);
	}

	fputs(primal_code->str, output_file);
	fclose(output_file);
}