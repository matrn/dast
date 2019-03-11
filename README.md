dast - data sharing tool using inotify
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