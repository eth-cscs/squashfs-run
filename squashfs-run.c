#define _POSIX_C_SOURCE 200809

#include <alloca.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define exit_with_error(...)                                                   \
  do {                                                                         \
    fprintf(stderr, __VA_ARGS__);                                              \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

struct args_t {
  char **args;
  size_t capacity;
  size_t size;
};

static void init_args(struct args_t *args) {
  args->size = 0;
  args->capacity = 10;
  args->args = malloc(args->capacity * sizeof(char *));
}

static void push_arg(struct args_t *args, char *p) {
  if (args->size == args->capacity) {
    args->capacity *= 2;
    args->args = (char **)realloc(args->args, args->capacity * sizeof(char *));
    if (args->args == NULL)
      exit_with_error("realloc failed\n");
  }
  args->args[args->size++] = p;
}

int main(int argc, char **argv) {
  char tmp_dir[] = "/tmp/squashfs-mnt-XXXXXX";

  struct args_t args;

  if (argc >= 2 &&
      (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0)) {
    puts(VERSION);
    exit(EXIT_SUCCESS);
  }

  if (argc < 3)
    exit_with_error(
        "Usage: %s <squashfs file> <command> [args...]\n\n "
        " Set SQUASHFS_RUN_DEBUG for dry-run, dumping the effective call.\n",
        argv[0]);

  init_args(&args);

  char mountpoint[] = "/user-environment";

  struct stat st;
  if (lstat(mountpoint, &st) == 0) {
    // If the mountpoint already exists, use it, no need for bwrap.
    push_arg(&args, "squashfs-mount");
    push_arg(&args, argv[1]);
    push_arg(&args, mountpoint);
  } else {
    // Otherwise create a temporary dir to mount to, and then bind
    // it with bwrap to mountpoint in a new root.

    if (mkdtemp(tmp_dir) == NULL)
      exit_with_error("Failed to create a temporary directory\n");

    // Setup `squashfs-mount`.
    push_arg(&args, "squashfs-mount");
    push_arg(&args, argv[1]);
    push_arg(&args, tmp_dir);

    // Setup `bwrap`
    push_arg(&args, "bwrap");

    DIR *folder = opendir("/");
    if (folder == NULL)
      exit_with_error("Could not open /\n");
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
        exit_with_error("alloca failed\n");
      src[0] = '/';
      dst[0] = '/';
      memcpy(src + 1, entry->d_name, len + 1); // include the trailing null
      memcpy(dst + 1, entry->d_name, len + 1);
      push_arg(&args, "--dev-bind");
      push_arg(&args, src);
      push_arg(&args, dst);
    }
    if (closedir(folder) != 0)
      exit_with_error("Could not close /\n");

    push_arg(&args, "--bind");
    push_arg(&args, tmp_dir);
    push_arg(&args, mountpoint);
  }

  for (int i = 2; i < argc; ++i)
    push_arg(&args, argv[i]);

  push_arg(&args, NULL);

  if (getenv("SQUASHFS_RUN_DEBUG") != NULL) {
    for (size_t i = 0; i < args.size - 1; ++i) {
      fputs(args.args[i], stdout);
      putchar(' ');
    }
    putchar('\n');
    exit(EXIT_SUCCESS);
  }

  execvp(args.args[0], args.args);
}
