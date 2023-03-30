#define _XOPEN_SOURCE 700 
#include "sway/commands.h"
#include "sway/config.h"
#include "list.h"
#include "log.h"
#include "unistd.h"
#include <string.h>
#include <libgen.h>
#include <wordexp.h>

typedef struct {
	char *name;
	char *path;
} config_location_map;

int compare_paths( const config_location_map* item, char* abspath) {
	if (strcmp(item->name, basename(abspath))==0) {
		return 0;
	}
	return -1;
}
void free_config_location_map(config_location_map *config_loc) {
	free(config_loc->name);
	free(config_loc->path);
	free(config_loc);
}
void priority_configs(const char *path, const char *parent_dir, struct sway_config *config, list_t *locations) {
	char *wd = getcwd(NULL, 0);

	if (chdir(parent_dir) < 0) {
		sway_log(SWAY_ERROR, "failed to change working directory");
		goto cleanup;
	}

	wordexp_t p;
	if (wordexp(path, &p, 0) == 0) {
		char **w = p.we_wordv;
		size_t i;
		config_location_map *config_loc = malloc(sizeof (config_location_map));
		for (i = 0; i < p.we_wordc; ++i) {
			int index = list_seq_find(locations, (int (*)(const void *, const void *))compare_paths, w[i]);
      		char* matched_path = strdup(w[i]);
			config_loc->name = strdup(basename(matched_path));
			config_loc->path = matched_path;
			if (index == -1) {
				list_add(locations, config_loc);
				continue;
			}
			free_config_location_map(locations->items[index]);
			locations->items[index] = config_loc; 
		}
		
	}

	wordfree(&p);
	
	if (chdir(wd) < 0) {
		sway_log(SWAY_ERROR, "failed to change working directory");
	}
cleanup:
	free(wd);
}

struct cmd_results *cmd_include_one(int argc, char **argv) {

	struct cmd_results *error = NULL;
	char *parent_path = strdup(config->current_config_path);
	const char *parent_dir = dirname(parent_path);
	list_t *locs = create_list();
	
	if ((error = checkarg(argc, "include_one", EXPECTED_AT_LEAST, 1))) {
		return error;
	}
	
	for(int i=0; i<argc ; ++i){
		priority_configs(argv[i], parent_dir, config, locs);
	}

	config_location_map **locs_arr = (config_location_map**) locs->items;
	for(int i=0; i<locs->length; ++i) {
		load_include_configs (locs_arr[i]->path, config, &config->swaynag_config_errors);
	}
	for (int i = 0; i<locs->length; ++i) {
		free_config_location_map(locs_arr[i]);
	}
	list_free(locs);
  	free(parent_path);
	return cmd_results_new(CMD_SUCCESS, NULL);
}
