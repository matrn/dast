dast - data sharing tool using inotify
==========

Library for sharing variables and data through files using inotify events.

## File operations

### Read/Write whole file

### Append to file

### One line variables

#### Delimited by `=`

For example `test_var=Hello World`

#### Delimited by `dec(30)` `hex(1E)` ASCII character (record separator)
For example `test_var``0x1E``This is useful for special =-+*/!. characters`

### Multi-line data

#### Format:

`0x01``data name``0x02``multi \n line \n data and normal characters =-+*/!``0x03`