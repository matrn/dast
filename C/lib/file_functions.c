#include "dast.h"


/* -----save delimiters to global variables----- */
char OLPD[3] = { 00, '=', '\n' };   /* unused, =, newline */
char OLUD[3] = { 00, 02, '\n' };    /* unused, start of text, newline */
char MLUD[3] = { 01, 02, 03 };      /* start of heading, start of text, end of text */

char TPD = '-';   /* time printable delimiter */  
char TUD = 30;   /* time unprintable delimiter */
/* -----save delimiters to global variables----- */



s_byte open_rw(char * filename, FILE ** file){
	/* return values:
	 0  = OK
	 -1 = error while openning
	*/

	if(access(filename, F_OK) != -1 ){	/* test if file exists */
		/* file exists */
		if(!(*file = fopen(filename,"r+"))){	/* use r+ option for file open */
			perror("error while opennig file");
			return -1;
		}
	}else{
		/* file doesn't exist */
		if(!(*file = fopen(filename,"w+"))){	/* use option w+, it creates file */
			perror("error while opennig file");
			return -1;
		}
	}	

	return 0;
}


s_byte dast_open_rw(char * filename, DSFILE * dsfile){
	/* return values:
	 0  = OK
	 -1 = error while openning
	*/

	char * pidfile_name;
	DSFILE local;

	if(open_rw(filename, &local.file) != 0) return -1;	

	pidfile_name = generate_pidfile_name(filename);
	if(open_rw(pidfile_name, &local.pidfile) != 0) return -1;
	free(pidfile_name);

	*dsfile = local;

	return 0;
}


s_byte dast_open_rw_s(char * filename, char * pidfile_name, DSFILE * dsfile){   /* specific pidfile name */
	/* return values:
	 0  = OK
	 -1 = error while openning
	*/

	DSFILE local;

	if(open_rw(filename, &local.file) != 0) return -1;	
	if(open_rw(pidfile_name, &local.pidfile) != 0) return -1;

	*dsfile = local;

	return 0;
}


void dast_close(DSFILE dsfile){
	fclose(dsfile.file);
	fclose(dsfile.pidfile);
}


s_byte dast_write_pid(pid_t pid, FILE * file){
	/* return values:
	 0  = all is OK
	 -1 = error
	*/

	if(flock(fileno(file), LOCK_EX) != 0) return -1;   /* lock file for other programs write */

	rewind(file);	/* rewind back to the beginning */	
	fprintf(file, "%d", pid);
	fflush(file);   /* flush file buffer */
	ftruncate(fileno(file), ftell(file));   /* truncate file - end file */

	if(flock(fileno(file), LOCK_UN) != 0) return -1;   /* unlock file */

	return 0;
}


pid_t dast_read_pid(FILE * file){
	/* return values:
	 -1  = on error
	 0-X = PID
	*/
	char * pid_str;
	pid_t pid;
	
	
	if(flock(fileno(file), LOCK_SH) != 0) return -1;   /* lock file with shared lock */
	rewind(file);

	if((pid_str = malloc(10)) == NULL) return -1;

	if(fgets(pid_str, 10, file) == NULL) return -1;
	pid = atoi(pid_str);

	free(pid_str);

	if(flock(fileno(file), LOCK_UN) != 0) return -1;   /* unlock file */

	return pid;		
}


s_byte dast_write(char * data, DSFILE dsfile){
	/* return values:
	 0  = OK
	 -1 = error
	*/
	FILE * file;
	file = dsfile.file;

	if(flock(fileno(file), LOCK_EX) != 0){   /* lock file for other programs write */
		perror("flock");
		return -1;
	}

	rewind(file);	/* rewind back to the beginning */

	
	if(fprintf(file, "%s", data) < 0){   /* write data to file */
		perror("frpintf");
		return -1;
	}
	//free(data);	/* it's polite to free alocated memory */

	if(fflush(file) != 0){   /* flush file buffer */
		perror("fflush");
		return -1;
	}

	if(flock(fileno(file), LOCK_UN) != 0) return -1;   /* unlock file */

	return 0;
}


/*
s_byte dast_read(char ** data, FILE * file){
	return 0;
}
*/


/*
name	data before start_char	start_char	data before delim_char	delim_char	data before end_char	end_char
position			start_pos	delim_pos		0

https://ozh.github.io/ascii-tables/

+----------+------------------------+------------+------------------------+------------+----------------------+----------+
|   name   | data before start_char | start_char | data before delim_char | delim_char | data before end_char | end_char |
+----------+------------------------+------------+------------------------+------------+----------------------+----------+
| position |                        |            | start_pos              | delim_pos  |                      |        0 |
+----------+------------------------+------------+------------------------+------------+----------------------+----------+
*/


long dast_read_var(char separators[3], char * var_name, char ** var_data, DSFILE dsfile){
	/*
	 Return values:
	 0-X = size
	 -1  = error
	 -2  = unknown variable
	 */
	char start_char, delim_char, end_char;

	/* variables for getdelim function */
	size_t len = 0;   /* size of alocated buffer */
	ssize_t nread;   /* for saving length of readed line */
	char * line = NULL;   /* variable for saving current line, if this var is NULL and len is 0, getdelim will automatically allocate memory for it */

	FILE * file;

	file = dsfile.file;

	start_char = separators[0];
	delim_char = separators[1];
	end_char = separators[2];


	if(flock(fileno(file), LOCK_SH) != 0) return -1;   /* lock file with shared lock */

	rewind(file);	/* rewind to the beginning of file */

	while((nread = getdelim(&line, &len, end_char, file)) != -1){   /* read file line by line */
		int start_pos = 0;   /* for finding correct position of start character */ 
		int delim_pos = 0;   /* for finding correct position of delimiter */ 
		
		
		/* -----get position of start character if it's specified----- */
		if(start_char != 0){   /* if user specified start character we must find it and remove everything before it */
			while(start_pos < nread - 1){   /* nread - 1 because we don't have to read end character */
				if(line[start_pos] == start_char){   /* start character was found */
					start_pos ++;   /* we have to incerement 1 because we want position of actual data */
					break;
				}

				start_pos ++;
			}
		}
		/* -----get position of start character if it's specified----- */

		/* -----get position of delimiter----- */
		delim_pos = start_pos;   /* our search starts at start position */

		while(delim_pos < nread - 1){   /* let's find position of delimiter, nread - 1 because we don't have to read end character */
			if(line[delim_pos] == delim_char) break;

			delim_pos ++;
		}
		/* -----get position of delimiter----- */
	

		line[nread -1] = 0;   /* remove end character from readed line/data */

		//printf("Delim pos: %d, nread %ld\n", delim_pos, nread);

		if(delim_pos != nread - 1){   /* check if there is delimiter in the string */
			char * name;   /* for saving parsed name */
			unsigned int len = delim_pos - start_pos;   /* length of name */

			/* let's allocate memory and copy data */
			if((name = malloc(len + 1)) == NULL) return -1;   /* allocate memory for name of variable, len + 1 for termination string */
			strncpy(name, line + start_pos, len);   /* copy name of the variable to the variable name */
			name[len] = 0;   /* save 0 to the end of string */
			

			//printf("Name >%s<\n", name);
			
			/* -----found correct name of variable----- */
			if(strcmp(name, var_name) == 0){   /* check if name from file is same as passed var_name */
				len = nread - delim_pos - 2;   /* length of data, -1 is for delimiter and second -1 is for str terminator */
			
				//printf("Allocated for data: %d\n", len + 1);
				//printf("End position: %d\n", nread - delim_pos - 2);

				/* let's allocate memory and copy data */
				if((*var_data = malloc(len + 1)) == NULL) return -1;
				strcpy(*var_data, line + delim_pos + 1);   /* save data from variable - starting at delim_position + 1 */
				//*var_data[len] = 0;   /* end of string, -2 is for delimiter character and because array starts at 0 */


				if(flock(fileno(file), LOCK_UN) != 0) return -1;   /* unlock file */

				free(name);   /* free name variable */
				free(line);   /* free line variable */

				return len;   /* length of return str from getdelim - position of delimiter - delimiter + newline */
			}
			/* -----found correct name of variable----- */


			free(name);   /* free name variable */
		}
	}

	if(flock(fileno(file), LOCK_UN) != 0) return -1;   /* unlock file */

	free(line);   /* free line variable */


	return -2;   /* unknown variable */
}



s_byte dast_just_write_var(char separators[3], char * var_name, char * var_data, FILE * file){
	/*
	 Return values:
	 -1 = some kind of error
	 0  = rewritten only one line
	 1  = rewritten file from line with variable to the end
	 2  = variable added to the end of file
	 */

	char start_char, delim_char, end_char;

	/* variables for getdelim function */
	size_t len = 0;   /* size of alocated buffer */
	ssize_t nread;   /* for saving length of readed line */
	char * line = NULL;   /* variable for saving current line, if this var is NULL and len is 0, getdelim will automatically allocate memory for it */

	// unsigned int a = 0;   /* variable for everything */

	char * data_buf = NULL;   /* data from variable position to the end */
	long data_length;   /* position from ftell function */


	unsigned long file_pos = 0;   /* for saving position of read bytes from file */

	size_t len_for_write = 0;   /* length of name and data */
	

	start_char = separators[0];
	delim_char = separators[1];
	end_char = separators[2];


	len_for_write = strlen(var_name) + strlen(var_data) + 2;   /* length of var_name and var_data + delimiter + end character */
	if(start_char != 0) len_for_write ++;

	
	//if(flock(fileno(file), LOCK_EX) != 0){   /* lock file for other programs write */
	//	perror("flock");
	//	return -1;
	//}
	

	//dast_write_pid(parent_pid, dsfile.pidfile);

	rewind(file);	/* rewind to the beginning of file */


	while((nread = getdelim(&line, &len, end_char, file)) != -1){   /* read file line by line */
		int start_pos = 0;   /* for finding correct position of start character */ 
		int delim_pos = 0;   /* for finding correct position of delimiter */ 
		
		
		/* -----get position of start character if it's specified----- */
		if(start_char != 0){   /* if user specified start character we must find it and remove everything before it */
			while(start_pos < nread - 1){   /* nread - 1 because we don't have to read end character */
				if(line[start_pos] == start_char){   /* start character was found */
					start_pos ++;   /* we have to incerement 1 because we want position of actual data */
					break;
				}

				start_pos ++;
			}
		}
		/* -----get position of start character if it's specified----- */

		/* -----get position of delimiter----- */
		delim_pos = start_pos;   /* our search starts at start position */

		while(delim_pos < nread - 1){   /* let's find position of delimiter, nread - 1 because we don't have to read end character */
			if(line[delim_pos] == delim_char) break;

			delim_pos ++;
		}
		/* -----get position of delimiter----- */
	

		line[nread -1] = 0;   /* remove end character from readed line/data */

		//printf("Start pos: %d, Delim pos: %d, nread %ld\n", start_pos, delim_pos, nread);

		if(delim_pos != nread - 1){   /* check if there is delimiter in the string */
			char * name;   /* for saving parsed name */
			unsigned int len = delim_pos - start_pos;   /* length of name */
			
			/* let's allocate memory and copy data */
			if((name = malloc(len + 1)) == NULL) return -1;   /* allocate memory for name of variable, len + 1 for termination string */
			strncpy(name, line + start_pos, len);   /* copy name of the variable to the variable name */
			name[len] = 0;   /* save 0 to the end of string */
			
			
			//printf("Name >%s<\n", name);
			
			/* -----found correct name of variable----- */
			if(strcmp(name, var_name) == 0){   /* check if name from file is same as passed var_name */
				unsigned int line_len_true = nread - start_pos;
				if(start_char != 0) line_len_true ++;
				//printf("Found var on position %ld\n", file_pos);
				
				//printf("Len for write: %ld, len of line: %d\n", len_for_write, line_len_true);
				//printf("LINE read >%s<\n", line);

				/*-----only current line rewrite----*/
				if(len_for_write == line_len_true){   /* check if existing length of variable is same as new, if yes, program will rewrite only current line */
					/* create new variable and save it to the line variable which is allocated by getdelim() function */
					if(start_char == 0) sprintf(line, "%s%c%s%c", name, delim_char, var_data, end_char);   /* without start_char */
					else sprintf(line, "%c%s%c%s%c", start_char, name, delim_char, var_data, end_char);   /* with start char */

					//printf("Writing only current data >%s<\n", line);
					//printf("Setting position to: %ld\n", file_pos + start_pos - ((start_char == 0) ? 0 : 1));

					fseek(file, file_pos + start_pos - ((start_char == 0) ? 0 : 1), SEEK_SET);   /* seek to the start position of this variable but keep garabge before start_char in file */
					fputs(line, file);   /* write data to the file */
					//if(fflush(file) != 0) return -1;   /* flush file */


					//if(flock(fileno(file), LOCK_UN) != 0) return -1;   /* unlock file */
					

					/* free allocated memory */
					//free(vars_all);
					free(name);
					free(line);

					return 0;   /* return rewritten only one line */
				}
				/*-----only current line rewrite----*/

				/*-----file was rewritten from current line to the end----*/
				else{
					//puts("From curr to the end");


					fseek(file, 0, SEEK_END);   /* go to the end of file */
					data_length = ftell(file) - file_pos - nread;   /* get end length of data from current position to the end */	
					fseek(file, file_pos + nread, SEEK_SET);   /* get to position after current var in file */

					//printf("File pos: %ld, nread: %ld, length: %ld\n", file_pos, nread, data_length);
					if((data_buf = malloc(data_length + 1)) == NULL) return -1;   /* allocate memory for data after this variable */
	
					if(data_buf){   /* check if we allocated something */
						/*if(*/fread(data_buf, 1, data_length, file);/* == 0){
							perror("fread");
							return -1;
						}*/
					}

					data_buf[data_length] = 0;

					//printf("to the end>%s<\n", data_buf);

					file_pos += start_pos - ((start_char == 0) ? 0 : 1);
					break;
				}
				/*-----file was rewritten from current line to the end----*/
			}

			free(name);   /* free allocated name variable */
		}

		file_pos += nread;
	}


	if(data_buf){
		//puts("Rewritting from current var to the end");
		//printf("Position: %ld\n", file_pos);

		fseek(file, file_pos, SEEK_SET);   /* seek to the position of variable */

		if(start_char == 0) fprintf(file, "%s%c%s%c", var_name, delim_char, var_data, end_char);   /* without start_char */
		else fprintf(file, "%c%s%c%s%c", start_char, var_name, delim_char, var_data, end_char);   /* with start char */
		
		fputs(data_buf, file);

		//if(fflush(file) != 0) return -1;   /* flush data to the file */
		

		ftruncate(fileno(file), ftell(file));   /* truncate file - end file */
	}
	else{   /* variable was not found so it will be added to the end of file */
		//puts("Added to the ned of file");
		/* write variable to the end of file */
		if(start_char == 0) fprintf(file, "%s%c%s%c", var_name, delim_char, var_data, end_char);   /* without start_char */
		else fprintf(file, "%c%s%c%s%c", start_char, var_name, delim_char, var_data, end_char);   /* with start char */

		//if(fflush(file) != 0) return -1;   /* flush data to the file */
	}


	//if(flock(fileno(file), LOCK_UN) != 0) return -1;   /* unlock file */


	/* free allocated memory (variables) */
	free(line);
	if(data_buf) free(data_buf);   /* free data buf if it was allocated */
	

	if(!data_buf) return 2;   /* added to the end of file */
	else return 1;   /* rewritten from variabůe position to the end */
}





s_byte dast_write_var(char separators[3], char * var_name, char * var_data, DSFILE dsfile){
	/*
	 Return values:
	 -1 = some kind of error
	 0  = rewritten only one line
	 1  = rewritten file from line with variable to the end
	 2  = variable added to the end of file
	 */

	
	FILE * file;

	file = dsfile.file;
	s_byte rtn = 0;


	if(flock(fileno(file), LOCK_EX) != 0){   /* lock file for other programs write */
		perror("flock");
		return -1;
	}

	dast_write_pid(parent_pid, dsfile.pidfile);


	rtn = dast_just_write_var(separators, var_name, var_data, file);
	

	if(fflush(file) != 0) return -1;   /* flush data to the file */	

	if(flock(fileno(file), LOCK_UN) != 0) return -1;   /* unlock file */


	return rtn;
}


s_byte dast_write_vars(char separators[3], dict * vars, DSFILE dsfile){
	/*
	 Return values:
	 -1 = some kind of error
	 0  = ok
	 */

	
	FILE * file;

	file = dsfile.file;
	dict * current = vars;


	if(flock(fileno(file), LOCK_EX) != 0){   /* lock file for other programs write */
		perror("flock");
		return -1;
	}

	dast_write_pid(parent_pid, dsfile.pidfile);

	

	while(1){
		printf(">%s< : >%s<\n", current->key, current->value);
		if(dast_just_write_var(separators, current->key, current->value, file) == -1) return -1;

		if(current->next == NULL) break;
		current = current->next;
	}
	
	

	if(fflush(file) != 0) return -1;   /* flush data to the file */	

	if(flock(fileno(file), LOCK_UN) != 0) return -1;   /* unlock file */


	return 0;
}