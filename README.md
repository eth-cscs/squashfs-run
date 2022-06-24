# squashfs-run

A small utility around `squashfs-mount` and `bwrap` that runs a command with
squashfs file mounted directly under `/`.

The squashsfs file should be prepended with an absolute path `/mount-point`
followed by `\0`s padded to 4096 bytes.

Then `squashfs-run file.squashfs [command...]` will:

1. First mount the squashfs file in `/tmp` through `squashfs-mount`.
2. Then use `bwrap` to bind it to the provided path in the metada.

