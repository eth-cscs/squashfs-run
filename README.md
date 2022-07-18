# squashfs-run

A small utility around `squashfs-mount` and `bwrap` that runs a command with
a squashfs file mounted at `/user-environment`.

Usage: `squashfs-run file.squashfs [command...]`.

If `/user-environment` does not exist as a path, this utility will

1. First mount the squashfs file in `/tmp` through `squashfs-mount`.
2. Then use `bwrap` to bind it to `/user-environment` in the metada.

Otherwise it mounts to `/user-environment` directly.

