#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <resdb/client_api.h>
#include "sway/commands.h"
#include "sway/config.h"
#include "stringop.h"
#include "log.h"

struct cmd_results *cmd_set_from_resource(int argc, char **argv) {
	struct cmd_results *error = NULL;
	if ((error = checkarg(argc, "set", EXPECTED_AT_LEAST, 3)) ||
				 (error = checkarg(argc, "set", EXPECTED_AT_MOST, 3))) 
	{
		return error;
	}
	if (argv[0][0] != '$') {
		return cmd_results_new(CMD_INVALID, "variable '%s' must start with $", argv[0]);
	}

	conf_client proxy = NULL;
	GError *err = NULL;
	conf_client_init(&proxy, &err);

	char *resource_value = NULL;
	conf_client_get(proxy, argv[1], &resource_value, &err);
	sway_log(SWAY_ERROR, "recieved: %s", resource_value);

	if (err || resource_value == NULL || strlen(resource_value) == 0) {
		resource_value = join_args(argv + 2, argc - 2);
	}
	char **argv_new = calloc(2, sizeof(char *));
	argv_new[0] = strdup(argv[0]);
	argv_new[1] = strdup(resource_value);
	
	sway_log(SWAY_ERROR, "new argv: %s", argv_new[1]);

	free(resource_value);
	return cmd_set(argc - 1, argv_new);
}