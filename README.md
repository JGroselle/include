# Useful bash function to include

`bash_common.inc` file include two functions:
* debug
* f_notify

## Getting started

Simply include this file at the begining of your bash script.

Example:

```
# includes common
if [ -x /usr/local/include/bash_common.inc ] ; then
  source /usr/local/include/bash_common.inc
else
  echo "[ERR] Impossible to load bash_common.inc"
  exit 2
fi
```

## debug

This function is a simple wrapper to help debugging by printing function name and line in error.

Example:

```
debug "--- MARK ---"
```

## f_notify

This function is a bash logging utility.

Features:
* print defined message in a file
* print defined message to stdout
* print defined message to X
* define exite code
* define if script exit or not

Example:

```
f_notify -m "---MARK---" -sf -c 2 -e n
```


