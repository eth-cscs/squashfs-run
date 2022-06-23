#include <alloca.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define exit_with_error(str)                                                   \
  do {                                                                         \
    fputs(str, stderr);                                                        \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

/** this utility is almost equivalent to `bwrap --dev-bind / / args...`, except
 * that it --dev-binds the direct children of / so that another root-level
 * directory can be added added: bwrapwrap --bind example /example.
 **/

struct args_t {
  char **args;
  size_t capacity;
  size_t size;
};

static void init_args(struct args_t *args) {
  args->size = 0;
  args->capacity = 6;
  args->args = malloc(args->capacity * sizeof(char *));
}

static void push_arg(struct args_t *args, char *p) {
  if (args->size == args->capacity) {
    args->capacity *= 2;
    args->args = (char **)realloc(args->args, args->capacity * sizeof(char *));
    if (args->args == NULL)
      exit_with_error("realloc failed");
  }
  args->args[args->size++] = p;
}

int main(int argc, char **argv) {
  struct args_t args;
  init_args(&args);

  char *exe = "bwrap";
  push_arg(&args, exe);

  DIR *folder = opendir("/");
  if (folder == NULL)
    exit_with_error("Could not open /");
  struct dirent *entry;
  while ((entry = readdir(folder))) {
    // Skip `.` and `..`
    if (entry->d_name[0] == '.' &&
        (entry->d_name[1] == '\0' ||
         (entry->d_name[1] == '.' && entry->d_name[2] == '\0')))
      continue;

    // store the `--dev-bind src dst` args
    size_t len = strlen(entry->d_name) + 2; // slash + name + null
    char *src = (char *)alloca(len * sizeof(char));
    char *dst = (char *)alloca(len * sizeof(char));
    if (src == NULL || dst == NULL)
      exit_with_error("alloca failed");
    src[0] = '/';
    dst[0] = '/';
    memcpy(src + 1, entry->d_name, len + 1); // include the trailing null
    memcpy(dst + 1, entry->d_name, len + 1);
    push_arg(&args, "--dev-bind");
    push_arg(&args, src);
    push_arg(&args, dst);
  }
  if (closedir(folder) != 0)
    exit_with_error("Could not close /");

  for (int i = 1; i < argc; ++i)
    push_arg(&args, argv[i]);
  push_arg(&args, NULL);

  execvp(exe, args.args);
}
