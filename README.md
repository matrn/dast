dast - data sharing tool using inotify - *Work in progress*
==========

Library for sharing variables and data through files using inotify events.


## File formats

### Whole file

Library will just read or write whole file without formatting.

### One line variables

 * prntd (printable delimiter) Delimited by `=` - For example: `test_var=Hello World`

 * unprd (unprintable delimiter) Delimited by `dec(30)` `hex(1E)` ASCII character (record separator) - For example: `test_var``0x1E``This is useful for special =-+*/!. characters`

### Multi-line data

 * munpd (multi-line, unprintable delimiters) Format: `0x01``data name``0x02``multi \n line \n data and normal characters =-+*/!``0x03`


## File operations

### Read/Write whole file

Functions `s_byte dast_read(char * data, FILE ** file);` and `s_byte dast_ẅrite(char ** data, FILE ** file);`

### Append to file

### Read one variable

### Write one variable

### Read all variables

### Write all variables


## Write realisation

If function for writting variable to file is called, program will call `flock(fileno(*file), LOCK_EX);` to lock file. This lock is called before file read to prevent length change. After read & write program will call `flock(fileno(*file), LOCK_UN);` for unlocking file.

Library can rewrite only part of file, you can get info about rewritten data from return value:

 - `-1` - error
 - `0` - rewritten only line with correct variable - this is only when existing and new lines have same line
 - `1` - rewritten file from line with correct variable to the end of file, after reaching end file will be truncated
 - `2` - variable added to the end of file

## ToDo

 - check string null ending
