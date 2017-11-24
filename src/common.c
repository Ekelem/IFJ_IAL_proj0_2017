#include "common.h"

/* Initializes hash table before it has been used */
htab_t * initialization(token_buffer * token_buff)
{
	int err_line = 1, err_pos = 0;

	init_garbage_collector();

	int state = 0, prev_state = 0;
	while ( state != EOF) {
		token * t;
		t = get_token(stdin, &err_line, &err_pos);
		if (t->type == NEW_LINE) {
			err_line += 1;
			err_pos = 0;
		}
		prev_state = state;
		state = t->type;
		if (state != NEW_LINE || prev_state != NEW_LINE)
			add_token(token_buff, t);
	}

	return htab_init(((token_buff->len)/16)+8);		//wild guess (one sixteenth of all tokens)
}