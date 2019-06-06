dast - data sharing tool using inotify
==========

# __Work in progress__

Library for sharing variables and data through files using inotify events.


# Content
 * [installation](#installation)
 * [C](#c)
    * [Watch functions](#watch-functions)
        * [Initialize dast - `dast_init()`](#initialize-dast)
        * [Watch new directory - `dast_watch_dir(char * dir_name)`](#watch-new-directory)
        * [Setup callback - `dast_watch(char * filename, callback_func func)`](#setup-callback)
        * [Run inotify watch daemon - `dast_run()`](#run-inotify-watch-daemon)
        * [Cleanup variables and inotify events - `dast_cleanup()`](#cleanup-variables-and-inotify-events)

    * [File functions](#file-functions)
        * [Read one variable - `dast_read_var(char separators[3], char * var_name, char ** var_data, DSFILE dsfile)`](#read-one-variable)
        * [Write one variable - `dast_write_var(char separators[3], char * var_name, char * var_data, DSFILE dsfile)`](#write-one-variable)
        * [Write multiple variables - `dast_write_vars(char separators[3], dict * vars, DSFILE dsfile)`](#write-multiple-variables)
        * [Write realisation](#write-realisation)
        * [Predefined variables separators](#predefined-variables-separators)

-----

# Installation
 - C - got to the dast directory, `cd C`, call `make` and run example with `./main`



# __C__



# Watch functions
-----


## Initialize dast
__`dast_init()`__
This functions will setup inotify event.

### Return value
 - `0`  = OK
 - `-1` = `inotify_init()` error

-----

## Watch new directory
__`dast_watch_dir(char * dir_name)`__
Function adds new inotify watch dir using `inotify_add_watch()`.

### Arguments
 - `char * dir_name` - name of watched directory

### Return value
 - `0`  = OK
 - `-1` = `inotify_add_watch()` error

-----

## Setup callback
__`dast_watch(char * filename, callback_func func)`__
Function for binding callback to specific filename.

### Arguments
 - `char * filename` - name of file - if filename ends with `*` (`\*` is escape sequence, you have to write `"\\*"` to string) after name before `*` can be anything, so if you setup filename as `test*`, `testFile` or `test` or `test5` will trigger callback function.
 - `callback_func func` - callback function

### Return value
 - `0`  = OK
 - `-1` = error

-----

## Run inotify watch daemon
__`dast_run()`__
This function will `fork()` code, in child process will be readed `inotify` events, function is non-blocking and it's necessary to call `dast_cleanup()` for killing child process.

### Return value
 - `0`  = OK
 - `-1` = error

-----

## Cleanup variables and inotify events
__`dast_cleanup()`__
Function for calling `free()` for global dast variables and destroying child. No value is returned.

-----
# File functions
-----
-----

## Read one variable
__`dast_read_var(char separators[3], char * var_name, char ** var_data, DSFILE dsfile)`__

### Arguments:
 - `char separators[3]` - separators of variable, library has predefined separators already, more about it at [predefined variables separators](#predefined-variables-separators)
 - `char *var_name` - pointer for name of the variable
 - `char ** var_data` - here will be saved pointer for read data. Before calling this function pointer must be clear and after function it should be freed using `free()` function.
 - `DSFILE dsfile` - file structure which contains fd of file and pid file


### Return value

 - `0-X` = size of data if variable exists (X is long max value)
 - `-1`  = error
 - `-2`  = unknown variable

-----

## Write one variable
__`dast_write_var(char separators[3], char * var_name, char * var_data, DSFILE dsfile)`__

### Arguments:
 - `char separators[3]` - separators of variable, library has predefined separators already, more about it at [predefined variables separators](#predefined-variables-separators)
 - `char *var_name` - pointer for name of the variable
 - `char * var_data` - pointer for data which will be written to the file
 - `DSFILE dsfile` - file structure which contains fd of file and pid file

This function can write only part of file, more about it and also about return values in the section [write realisation](#write-realisation)


## Write multiple variables
__`dast_write_vars(char separators[3], dict * vars, DSFILE dsfile)`__

### Arguments:
 - `char separators[3]` - separators of variable, library has predefined separators already, more about it at [predefined variables separators](#predefined-variables-separators)
 - `dict * vars` - dictionary of variables using functions from `dict.c`, usage:
 ```
 dict * vars;   /* declare dictionary */
 if(dict_init(&vars) != 0) perror("dict_init");   /* initialize dictionary */
 if(dict_set(vars, "key", "value") != 0) perror("dict_set");   /* set value to the dictionary indentified by key */
 //here call function for writing variables to the file
 dict_free(vars);   /* free dictionary */
 ```
 - `DSFILE dsfile` - file structure which contains fd of file and pid file

This function can write only part of file, more about it and also about return values in the section [write realisation](#write-realisation)


## Write realisation

If function for writting variable to file is called, program will call `flock(fileno(*file), LOCK_EX);` to lock file. This lock is called before file read to prevent length change. After read & write program will call `flock(fileno(*file), LOCK_UN);` for unlocking file.

Library can rewrite only part of file, you can get info about rewritten data from __return value__:

 - `-1` - error
 - `0` - rewritten only line with correct variable - this is only when existing and new lines have same line
 - `1` - rewritten file from line with correct variable to the end of file, after reaching end file will be truncated
 - `2` - variable added to the end of file

----

## Predefined variables separators

 * OLPD = One Line Printable Delimiter - name is delimited from data using `=` and line is ended with `\n`, for example: `test_var=Hello World``\n`
 * OLUD = One Line Unprintable Delimiter - name is delimited from data using `dec(02)` ASCII `start of text` and line is ended with `\n`, for example: `test_var``0x1E``This is useful for special =-+*/!. characters``\n`
 * MLUD = Multi Line Unprintable Delimiter - name is delimited from data using `dec(02)` ASCII `start of text` and ended with `dec(03)` ASCII `end of text`, for example:  `test_var``0x1E``multi \n line \n data and normal characters =-+*/!``0x03`
 * TPD = Time Printable Delimiter - for separating time from data, represented by `-`
 * TUD = Time Unprintable Delimiter - for separating time from data, represented by `dec(30)` ASCII `record separator`

*Note that mixing different separators can cause problems.*

-----
-----
-----

## ToDo

 - [x] C cleanup
 - [ ] C refactoring
 - [x] C - add flock for reading
 - [x] C - add name to callback function
 - [ ] C - get dir from wd, pid file in correct location
 - [ ] README update - dast_watch and functions like this
 - [x] add ignoring inotify events from current programe
 - [ ] add MLUD to C example
 - [ ] C example
 - [ ] Python support
 - [ ] Python example
 - [x] modify dast_write_var for new type of delimiters and cleanup dast_read_var


