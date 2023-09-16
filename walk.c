#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

const char *prog_name;
int no_file = 0;
int no_dir = 0;

// borrowed from tsoding
const char *shift(int *argc, char ***argv) {
	assert(*argc > 0);
	const char *result = *argv[0];
	*argc -= 1;
	*argv += 1;
	return result;
}

void err(char *msg) {
	fprintf(stderr, prog_name);
	fprintf(stderr, ": ");
	fprintf(stderr, msg);
}

void help() {
	fprintf(stdout, "walk (-d|-f|-n [nr]) (<path>)\n");
	fprintf(stdout, "\t-h\thelp\n");
	fprintf(stdout, "\t-d\tshow only directories\n");
	fprintf(stdout, "\t-f\tshow only files\n");
	fprintf(stdout, "\t-n [nr]\tdepth of search\n");
	fprintf(stdout, "\t<path>\tpath to start the walk (default: current working directory); only the last path is used\n");
}

void* dirr(const char *path, int depth) 
{ 
	DIR *dp;
	struct dirent *ep;

	dp = opendir (path);
	if (dp != NULL) {
		while (ep = readdir (dp)) {
			char abs_path[256];
			char *name = ep->d_name;
			if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0  || strcmp(name, ".git") == 0 ) {
				continue;
			}
			if (ep->d_type != DT_DIR && no_file) {
				continue;
			}
			memset(abs_path, 0, sizeof(abs_path));
			strcat(abs_path, path);
			strcat(abs_path, "/");
			strcat(abs_path, name);
			if (ep->d_type==DT_DIR) {
				if (!no_dir) {
					puts(abs_path);
				}
				if (depth) { 
					dirr(abs_path, depth - 1);
				}
			} else {
				puts(abs_path);
			}
		}
		(void) closedir (dp);
	} else {
		char msg[256] = "Couldn't open the directory: ";
		strcat(msg, path);
		err(msg);
	}
}

int main(int argc, char **argv){
	prog_name = shift(&argc, &argv); // shift prog name
	int depth = -1;
	char wd[256];
	getcwd(wd, sizeof(wd));
	while (argc > 0) {
		const char *a = shift(&argc, &argv);
		if (*a == '-') {
			a += 1;
			if (*a == 'h') {
				help();
				exit(0);
			} else if (*a == 'd') {
				no_file = 1;	
			} else if (*a == 'f') {
				no_dir = 1;
			} else if (*a == 'n') {
				if (argc <= 0) {
					err("depth not provided\n");
					exit(1);
				}
				a = shift(&argc, &argv);
				int ai = atoi(a); // MAYBE: do better
				if (ai == 0) {
					err("invalid depth provided");
					exit(1);
				} else {
					depth = ai;
				}
			} else {
				char msg[32] = "Unknown flag -";
				strcat(msg, a);
				err(msg);
				exit(1);
			}
		} else {
			char *path = realpath(a, NULL);
			strcpy(wd, a);
		}
	}

	if ( wd == NULL) {
		err("getcwd() error\n");
		exit(1);
	} else {
		dirr(wd, depth - 1);
	}
}
