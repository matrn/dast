#include "dast.h"


char OLPD[3] = { 00, '=', '\n' };   /* unused, =, newline */
char OLUD[3] = { 00, 02, '\n' };    /* unused, start of text, newline */
char MLUD[3] = { 01, 02, 03 };      /* start of heading, start of text, end of text */




s_byte dast_open_rw(char * filename, FILE ** file){
	if(access(filename, F_OK) != -1 ){	/* test if file exists */
		/* file exists */
		if(!(*file = fopen(filename,"r+"))){	/* use r+ option for file open */
			perror("error while opennig svar file");
			exit(5);
		}
	}else{
		/* file doesn't exist */
		if(!(*file = fopen(filename,"w+"))){	/* use option w+, it creates file */
			perror("error while opennig svar file");
			exit(5);
		}
	}	

	return 0;
}



void dast_close(FILE ** file){
	fclose(*file);
}



s_byte dast_write(char * data, FILE ** file){	
	flock(fileno(*file), LOCK_EX);	/* lock file for other programs write */

	rewind(*file);	/* rewind back to the beginning */

	
	fprintf(*file, "%s", data);	/* write data to file */
		
	//free(data);	/* it's polite to free alocated memory */

	fflush(*file);	/* flush file buffer */
	
	flock(fileno(*file), LOCK_UN);	/* unlock file */

	return 0;
}


/*
s_byte dast_read(char ** data, FILE ** file){
	return 0;
}
*/


ssize_t dast_read_var(char separators[2], char * var_name, char ** var_data, FILE ** file){
	/*
	 Return values:
	 -1 = unknown variable
	 0-X = size
	 */
	char delimiter, data_end;

	/* variables for getline function */
	size_t len = 0;   /* size of alocated buffer */
	ssize_t nread;   /* for saving length of readed line */
	char * line = NULL;   /* variable for saving current line, if this var is NULL and len is 0, getline will automatically allocate memory for it */


	delimiter = separators[0];
	data_end = separators[1];

	rewind(*file);	/* rewind to the beginning of file */

	while((nread = getdelim(&line, &len, data_end, *file)) != -1){   /* read file line by line */
		char * name;   /* for saving parsed name */
		unsigned int pos = 0;   /* for finding correct position of delimiter */

		line[nread -1] = 0;   /* remove newline character from readed line */

		while(pos < nread){   /* let's find position of delimiter */
			if(line[pos] == delimiter) break;
			pos ++;
		}

		// printf("pos: %d, nread %ld\n", pos, nread);
		if(pos != nread){   /* check if there is delimiter in the string */
			name = malloc(pos + 1);   /* allocate memory for name of variable */
			strncpy(name, line, pos);   /* copy name of the variable to the variable name */
			
			if(strcmp(name, var_name) == 0){   /* check if name from file is same as passed var_name */
				*var_data = line + pos + 1;   /* save data from variable */


				free(name);   /* free name variable */
				free(line);   /* free line variable */

				return nread - pos - 2;   /* length of return str from getline - position of delimiter - delimiter + newline */
			}

			free(name);   /* free name variable */
		}
	}

	free(line);   /* free line variable */

	return -1;
}



s_byte dast_write_var(char separators[2], char * var_name, char * var_data, FILE ** file){
	/*
	 Return values:
	 -1 = some kind of error
	 0 = rewritten only one line
	 1 = rewritten file from line with variable to the end
	 2 = variable added to the end of file
	 */

	char delimiter, data_end;

	/* variables for getline function */
	size_t len = 0;   /* size of alocated buffer */
	ssize_t nread;   /* for saving length of readed line */
	char * line = NULL;   /* variable for saving current line, if this var is NULL and len is 0, getline will automatically allocate memory for it */

	unsigned int a = 0;   /* variable for everything */


	char ** vars_all;   /* for saving all read variables */
	unsigned int lines_num = 0;   /* for saving number of lines (variables) read from file */

	byte var_found = 0;   /* for saving if we found correct variable in read file */
	

	unsigned long file_pos = 0;   /* for saving position of read bytes from file */

	size_t len_for_write = 0;   /* length of name and data */
	

	delimiter = separators[0];
	data_end = separators[1];


	len_for_write = strlen(var_name) + strlen(var_data) + 2;   /* length of var_name and var_data + delimiter + newline */

	vars_all = malloc(sizeof(char *));   /* allocate memory for only one variable, memory for more variables will be reallocated during run */
	


	flock(fileno(*file), LOCK_EX);   /* lock file for other programs write */

	rewind(*file);	/* rewind to the beginning of file */


	while((nread = getdelim(&line, &len, data_end, *file)) != -1){   /* read file line by line */
		//printf("Current line >%s<", line);
		char * name;   /* for saving parsed name */

		//line[nread -1] = 0;   /* remove newline character from readed line */

		if(var_found == 0){   /* this part will be called only if variable wasn't still found */
			unsigned int pos = 0;   /* for finding correct position of delimiter */
	
			while(pos < nread){   /* find position of delimiter */
				if(line[pos] == delimiter) break;
				pos ++;
			}

			//printf("pos: %d, nread %ld\n", pos, nread);
			if(pos != nread){   /* check if there is delimiter in the string */
				name = malloc(pos + 2);   /* allocate memory for name of variable */
		
				strncpy(name, line, pos);   /* copy name of the variable to the variable name */
				name[pos] = 0;   /* end of string */

				//printf("name >%s<\n", name);
				if(strcmp(name, var_name) == 0){   /* check if name from file is same as passed var_name */
					//printf("Found var on position %ld\n", file_pos);
					

					if(len_for_write == strlen(line)){   /* check if existing length of variable is same as new, if yes, program will rewrite only current line */
						sprintf(line, "%s%c%s%c", name, delimiter, var_data, data_end);   /* create new variable and save it to the line variable which is allocated by getline() function */
						
						//printf("Writing only current data >%s<\n", line);

						fseek(*file, file_pos, SEEK_SET);   /* seek to the start position of this variable */
						fputs(line, *file);   /* write data to the file */
						fflush(*file);   /* flush file */

						flock(fileno(*file), LOCK_UN);   /* unlock file */

						/* free allocated memory */
						free(vars_all);
						free(name);
						free(line);

						return 0;   /* return rewritten only one line */
					}
					else{
						var_found = 1;   /* save that we found variable */

						lines_num ++;   /* increment lines_num */
						vars_all = realloc(vars_all, lines_num * sizeof(char *));   /* reallocate memory */
						vars_all[lines_num - 1] = malloc(len_for_write + 1);   /* allocate new place in array */

						sprintf(vars_all[lines_num -1], "%s%c%s%c", name, delimiter, var_data, data_end);   /* save new data to current place in array */
						//printf("HERE >%s<\n", vars_all[lines_num -1]);
						//strcpy(vars_all[lines_num - 1], line);

						//*vars_all = line + pos + 1;   /* save data from variable */


						//free(name);   /* free name variable */
						//free(line);   /* free line variable */

						//return nread - pos - 2;   /* length of return str from getline - position of delimiter - delimiter + newline */
					}
				}

				free(name);   /* free allocated name variable */
			}

			if(var_found == 0) file_pos += nread;   /* add read bytes to position in file if we still didn't find correct variable */
		}
		else{
			lines_num ++;   /* incrment lines_num variable */
			vars_all = realloc(vars_all, lines_num * sizeof(char *));   /* reallocate memory */
			vars_all[lines_num - 1] = malloc(strlen(line));   /* allocate new place in array */

			strcpy(vars_all[lines_num - 1], line);   /* cop data to this variable */
		}
	}



	if(var_found == 0){   /* variable was not found so it will be added to the end of file */
		fprintf(*file, "%s%c%s%c", var_name, delimiter, var_data, data_end);   /* write variable to the end of file */

		fflush(*file);   /* flush data to the file */
	}

	else{
		fseek(*file, file_pos, SEEK_SET);   /* seek to the position of variable */

		for(a = 0; a < lines_num; a ++){   /* iterate over vars_all */
			//printf("Writting >%s<\n", vars_all[a]);

			fputs(vars_all[a], *file);   /* write current variable to the file */
			free(vars_all[a]);   /* free current variable */
		}

		fflush(*file);   /* flush data to the file */
		ftruncate(fileno(*file), ftell(*file));   /* truncate file - end file */
	}



	flock(fileno(*file), LOCK_UN);   /* unlock file */

	/* free allocated memore (varaibles) */
	free(vars_all);
	free(line);

	if(var_found == 0) return 2;   /* added to the end of file */
	else return 1;   /* rewritten from variabůe position to the end */
}