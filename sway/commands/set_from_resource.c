#define _POSIX_C_SOURCE 200809L
#include "log.h"
#include "stringop.h"
#include "sway/commands.h"
#include "sway/config.h"
#include <trawldb/client_api.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

struct cmd_results *cmd_set_from_resource(int argc, char **argv) {
  struct cmd_results *error = NULL;
  if ((error = checkarg(argc, "set", EXPECTED_AT_LEAST, 2))) {
    return error;
  }
  if (argv[0][0] != '$') {
    return cmd_results_new(CMD_INVALID, "variable '%s' must start with $",
                           argv[0]);
  }

  conf_client proxy = NULL;
  GError *err = NULL;
  conf_client_init(&proxy, &err);

  char *resource_value = NULL;
  conf_client_get(proxy, argv[1], &resource_value, &err);

  if (err || resource_value == NULL || strlen(resource_value) == 0) {
    if (argc < 3) {
      return cmd_results_new(CMD_FAILURE,
                             "failed to fetch value of resource '%s' and "
                             "fallback value not provided",
                             argv[1]);
    }
    resource_value = join_args(argv + 2, argc - 2);
  }
  char **argv_new = calloc(argc - 1, sizeof(char *));
  argv_new[0] = strdup(argv[0]);
  for (int i = 2; i < argc; i++) {
    argv_new[i - 1] = argv[i];
  }

  argv_new[1] = strdup(resource_value);

  free(resource_value);
  return cmd_set(2, argv_new);
}
