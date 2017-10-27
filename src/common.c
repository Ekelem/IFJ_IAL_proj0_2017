#include "common.h"

htab_t * initialization(const char * file_path, token_buffer * token_buff)
{
	FILE * input_file;
	if ((input_file = fopen(file_path, "r")) == NULL) {
		fprintf(stderr, "We could not open the file: %s\n", file_path);
		exit(ERR_CODE_LEXICAL);
	}

	init_garbage_collector();

	int state = 0;
	while ( state != EOF) {
		token * t;
		t = get_token(input_file);
		add_token(token_buff, t);
		state = t->type;
	}

	//fprintf(stderr, "%d\n", token_buff->len);
	fclose(input_file);

	/*for (int i = 0; i < (token_buff->len-1); ++i)
	{
		print_token(token_buff->arr[i]->type, token_buff->arr[i]);
	}*/

	return htab_init(token_buff->len);
}