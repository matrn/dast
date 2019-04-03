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


ssize_t dast_read_var(char separators[3], char * var_name, char ** var_data, FILE ** file){
	/*
	 Return values:
	 -1 = unknown variable
	 0-X = size
	 */
	char start_char, delim_char, end_char;

	/* variables for getdelim function */
	size_t len = 0;   /* size of alocated buffer */
	ssize_t nread;   /* for saving length of readed line */
	char * line = NULL;   /* variable for saving current line, if this var is NULL and len is 0, getdelim will automatically allocate memory for it */



	start_char = separators[0];
	delim_char = separators[1];
	end_char = separators[2];

	rewind(*file);	/* rewind to the beginning of file */


	while((nread = getdelim(&line, &len, end_char, *file)) != -1){   /* read file line by line */
		char * name;   /* for saving parsed name */
		unsigned int start_pos = 0;   /* for finding correct position of start character */ 
		unsigned int delim_pos = 0;   /* for finding correct position of delimiter */ 
		
		
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
			name = malloc(delim_pos - start_pos + 1);   /* allocate memory for name of variable, delimiter position - start position is length of name between these two characters */
			
			strncpy(name, line + start_pos, delim_pos - start_pos);   /* copy name of the variable to the variable name */
			name[delim_pos - start_pos] = 0;   /* save 0 to the end of string */
			
			//printf("Name >%s<\n", name);
			
			if(strcmp(name, var_name) == 0){   /* check if name from file is same as passed var_name */
				*var_data = malloc(nread - delim_pos);
				strcpy(*var_data, line + delim_pos + 1);   /* save data from variable */


				free(name);   /* free name variable */
				free(line);   /* free line variable */

				return nread - delim_pos - 2;   /* length of return str from getdelim - position of delimiter - delimiter + newline */
			}

			free(name);   /* free name variable */
		}
	}

	free(line);   /* free line variable */

	return -1;
}



s_byte dast_write_var(char separators[3], char * var_name, char * var_data, FILE ** file){
	/*
	 Return values:
	 -1 = some kind of error
	 0 = rewritten only one line
	 1 = rewritten file from line with variable to the end
	 2 = variable added to the end of file
	 */

	char start_char, delim_char, end_char;

	/* variables for getdelim function */
	size_t len = 0;   /* size of alocated buffer */
	ssize_t nread;   /* for saving length of readed line */
	char * line = NULL;   /* variable for saving current line, if this var is NULL and len is 0, getdelim will automatically allocate memory for it */

	// unsigned int a = 0;   /* variable for everything */


	// char ** vars_all;   /* for saving all read variables */
	// unsigned int lines_num = 0;   /* for saving number of lines (variables) read from file */

	byte var_found = 0;   /* for saving if we found correct variable in read file */
	
	char * data_buf = NULL;   /* data from variable position to the end */
	long data_length;   /* position from ftell function */


	unsigned long file_pos = 0;   /* for saving position of read bytes from file */

	size_t len_for_write = 0;   /* length of name and data */
	

	start_char = separators[0];
	delim_char = separators[1];
	end_char = separators[2];


	len_for_write = strlen(var_name) + strlen(var_data) + 2;   /* length of var_name and var_data + delimiter + newline */
	if(start_char != 0) len_for_write ++;

	//vars_all = malloc(sizeof(char *));   /* allocate memory for only one variable, memory for more variables will be reallocated during run */
	//data_buf = malloc(500);


	flock(fileno(*file), LOCK_EX);   /* lock file for other programs write */

	rewind(*file);	/* rewind to the beginning of file */


	while((nread = getdelim(&line, &len, end_char, *file)) != -1){   /* read file line by line */
		printf("Current line >%s<", line);
		char * name;   /* for saving parsed name */

		//line[nread -1] = 0;   /* remove newline character from readed line */

		//if(var_found == 0){   /* this part will be called only if variable wasn't still found */
			unsigned int start_pos = 0;   /* for finding correct position of start character */ 
			unsigned int delim_pos = 0;   /* for finding correct position of delimiter */ 
			
			
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
				name = malloc(delim_pos - start_pos + 5);   /* allocate memory for name of variable, delimiter position - start position is length of name between these two characters */
				printf("STR>%s< DELIM POS: %d\n", line + start_pos, delim_pos);
				strncpy(name, line + start_pos, delim_pos - start_pos);   /* copy name of the variable to the variable name */
				name[delim_pos - start_pos] = 0;   /* save 0 to the end of string */

				printf("name >%s<\n", name);
				if(strcmp(name, var_name) == 0){   /* check if name from file is same as passed var_name */
					printf("Found var on position %ld\n", file_pos);
					

					/*-----only current line rewrite----*/
					if(len_for_write == strlen(line)){   /* check if existing length of variable is same as new, if yes, program will rewrite only current line */
						/* create new variable and save it to the line variable which is allocated by getdelim() function */
						if(start_char == 0) sprintf(line, "%s%c%s%c", name, delim_char, var_data, end_char);   /* without start_char */
						else sprintf(line, "%c%s%c%s%c", start_char, name, delim_char, var_data, end_char);   /* with start char */

						printf("Writing only current data >%s<\n", line);

						fseek(*file, file_pos, SEEK_SET);   /* seek to the start position of this variable */
						fputs(line, *file);   /* write data to the file */
						fflush(*file);   /* flush file */

						flock(fileno(*file), LOCK_UN);   /* unlock file */

						/* free allocated memory */
						//free(vars_all);
						free(name);
						free(line);

						return 0;   /* return rewritten only one line */
					}
					/*-----only current line rewrite----*/

					/*-----file was rewritten from current line to the end----*/
					else{
						fseek(*file, 0, SEEK_END);
						data_length = ftell(*file) - file_pos - nread;
						printf("LEN: %d\n", data_length);
						fseek(*file, file_pos + nread, SEEK_SET);
						printf("Curr: %d\n", ftell(*file));
						printf("File pos: %d, nread: %d, length: %d\n", file_pos, nread, data_length);
						data_buf = malloc(data_length + 5);
						puts("VV");
						if(data_buf){
							fread(data_buf, 1, data_length, *file);
						}
						puts("FF");
						break;

						// var_found = 1;   /* save that we found variable */

						// lines_num ++;   /* increment lines_num */
						// vars_all = realloc(vars_all, lines_num * sizeof(char *));   /* reallocate memory */
						// vars_all[lines_num - 1] = malloc(len_for_write + 1);   /* allocate new place in array */

  						
						/* save new data to current place in array */
						// if(start_char == 0) sprintf(vars_all[lines_num -1], "%s%c%s%c", name, delim_char, var_data, end_char);   /* without start_char */
						// else sprintf(vars_all[lines_num -1], "%c%s%c%s%c", start_char, name, delim_char, var_data, end_char);   /* with start char */
						

						//printf("HERE >%s<\n", vars_all[lines_num -1]);
						//strcpy(vars_all[lines_num - 1], line);

						//*vars_all = line + pos + 1;   /* save data from variable */


						//free(name);   /* free name variable */
						//free(line);   /* free line variable */

						//return nread - pos - 2;   /* length of return str from getdelim - position of delimiter - delimiter + newline */
					}
					/*-----file was rewritten from current line to the end----*/
				}

				free(name);   /* free allocated name variable */
			}

		file_pos += nread;
		//	if(var_found == 0) file_pos += nread;   /* add read bytes to position in file if we still didn't find correct variable */
		// }
		// else{
		// 	lines_num ++;   /* increment lines_num variable */
		// 	vars_all = realloc(vars_all, lines_num * sizeof(char *));   /* reallocate memory */
		// 	vars_all[lines_num - 1] = malloc(strlen(line));   /* allocate new place in array */

		// 	strcpy(vars_all[lines_num - 1], line);   /* cop data to this variable */
		// }
	}


	puts("A");
	if(data_buf){
		puts("B");
		fseek(*file, file_pos, SEEK_SET);   /* seek to the position of variable */

		if(start_char == 0) fprintf(*file, "%s%c%s%c", var_name, delim_char, var_data, end_char);   /* without start_char */
		else fprintf(*file, "%c%s%c%s%c", start_char, var_name, delim_char, var_data, end_char);   /* with start char */
		
		fputs(data_buf, *file);

		fflush(*file);   /* flush data to the file */
		ftruncate(fileno(*file), ftell(*file));   /* truncate file - end file */
	}
	else{   /* variable was not found so it will be added to the end of file */
		puts("C");
		/* write variable to the end of file */
		if(start_char == 0) fprintf(*file, "%s%c%s%c", var_name, delim_char, var_data, end_char);   /* without start_char */
		else fprintf(*file, "%c%s%c%s%c", start_char, var_name, delim_char, var_data, end_char);   /* with start char */

		fflush(*file);   /* flush data to the file */
	}
	puts("D");

	flock(fileno(*file), LOCK_UN);   /* unlock file */

	/* free allocated memore (varaibles) */
	//free(vars_all);
	free(line);
	puts("Qů");
	if(data_buf) free(data_buf);
	puts("Z");
	if(var_found == 0) return 2;   /* added to the end of file */
	else return 1;   /* rewritten from variabůe position to the end */
}
