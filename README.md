# shell

A small Unix shell I wrote in C++.

I wanted to understand how a shell actually works, quoting, redirects, builtins, fork/exec, tab completion, so I built one. It is not a full bash replacement. No pipes, no history, no job control.

## Build

Linux, g++ with C++20.

```
cd src
make -f makefile
./main
```

## What it supports

- builtins: `echo`, `pwd`, `cd` (including `~`), `type`, `exit`
- `cat` and `ls`
- redirects: `>`, `>>`, `1>`, `2>`, `2>>`
- single quotes, double quotes, backslash escapes
- tab completion for builtins, executables on PATH, files and directories
- running other programs with `fork` + `execvp`

Tab completion is custom (raw mode via termios, not readline). First tab completes if it can. If there are multiple matches it beeps; tab again to list them.

The REPL and builtins live in `src/main.cpp`. Tokenizing and PATH lookup are in `src/modules/utils.cpp`.
