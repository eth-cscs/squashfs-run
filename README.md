# bwrap wrapper to add root-level mount

`bwrap` is nice, but would be even nicer if it allowed you to conveniently add
a new root level directory to the current filesystem.

The problem is `bwrap --dev-bind / / --bind hello /hello` does not work,
because it binds `hello` only after `/` is already bound, meaning that `mkdir
/hello` fails with permission problems.

The alternative is to own the new root, and bind the old root's dir entries directly:

```
bwrap --dev-bind /bin /bin --dev-bind /usr /usr ... --bind hello /hello
```

But this is a pain, so instead `bwrapwrap` does this, so you can simply write:

```
bwrapwrap --bind hello /hello
```

Install instructions:

```
curl -Lfs ... | cc -o bwrapwrap -x c -
```
