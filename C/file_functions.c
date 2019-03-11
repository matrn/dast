#include "dast.h"

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

s_byte dast_read(char ** data, FILE ** file){
	return 0;
}


ssize_t dast_read_var(char delimiter, char * var_name, char ** var_data, FILE ** file){
	/*
	 Return values:
	 -1 = unknown variable
	 0-X = size
	 */

	/* variables for getline function */
	size_t len = 0;   /* size of alocated buffer */
	ssize_t nread;   /* for saving length of readed line */
	char * line = NULL;   /* variable for saving current line, if this var is NULL and len is 0, getline will automatically allocate memory for it */

	unsigned int pos = 0;   /* for finding correct position of delimiter */


	rewind(*file);	/* return to the beginning of file */

	while((nread = getline(&line, &len, *file)) != -1){   /* read file line by line */
		char * name;   /* for saving parsed name */

		line[nread -1] = 0;   /* remove newline character from readed line */

		pos = 0;   /* null delimiter position variable */

		while(pos < nread){
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


s_byte dast_write_var(char delimiter, char * var_name, char * var_data, FILE ** file){
	/*
	 Return values:
	 -1 = unknown variable
	 0-X = size
	 */
	///!!!!!přidat flock

	/* variables for getline function */
	size_t len = 0;   /* size of alocated buffer */
	ssize_t nread;   /* for saving length of readed line */
	char * line = NULL;   /* variable for saving current line, if this var is NULL and len is 0, getline will automatically allocate memory for it */

	unsigned int pos = 0;   /* for finding correct position of delimiter */

	unsigned long file_pos = 0;

	byte var_found = 0;

	char ** vars_all;

	vars_all = malloc(sizeof(char *));
	unsigned int lines_num = 0;

	rewind(*file);	/* return to the beginning of file */

	while((nread = getline(&line, &len, *file)) != -1){   /* read file line by line */
		char * name;   /* for saving parsed name */

		//line[nread -1] = 0;   /* remove newline character from readed line */

		pos = 0;   /* null delimiter position variable */
		
		if(var_found == 0){
			while(pos < nread){
				if(line[pos] == delimiter) break;
				pos ++;
			}

			// printf("pos: %d, nread %ld\n", pos, nread);
			if(pos != nread){   /* check if there is delimiter in the string */
				name = malloc(pos + 1);   /* allocate memory for name of variable */
				strncpy(name, line, pos);   /* copy name of the variable to the variable name */
				
				if(strcmp(name, var_name) == 0){   /* check if name from file is same as passed var_name */
					printf("Found var on position %ld\n", file_pos);
					size_t name_len, data_len = 0;
				
					name_len = strlen(name);
					data_len = strlen(var_data);

					if(name_len + data_len + 2 == strlen(line)){
						//puts("writing only current line");
						fseek(*file, file_pos, SEEK_SET);

						sprintf(line, "%s%c%s\n", name, delimiter, var_data);
						printf("Writing only current data >%s<\n", line);
						fputs(line, *file);
						fflush(*file);
						free(vars_all);
						free(name);
						free(line);
						return 0;
					}
					var_found = 1;

					lines_num ++;
					vars_all = realloc(vars_all, lines_num * sizeof(char *));
					vars_all[lines_num - 1] = malloc(name_len + data_len + 3);

					sprintf(vars_all[lines_num -1], "%s%c%s\n", name, delimiter, var_data);
					printf("HERE >%s<\n", vars_all[lines_num -1]);
					//strcpy(vars_all[lines_num - 1], line);

					//*vars_all = line + pos + 1;   /* save data from variable */


					//free(name);   /* free name variable */
					//free(line);   /* free line variable */

					//return nread - pos - 2;   /* length of return str from getline - position of delimiter - delimiter + newline */
				}

				free(name);   /* free name variable */
			}

			if(var_found == 0) file_pos += nread;
		}
		else{
			lines_num ++;
			vars_all = realloc(vars_all, lines_num * sizeof(char *));
			vars_all[lines_num - 1] = malloc(strlen(line));
			strcpy(vars_all[lines_num - 1], line);
		}

		//printf("File Pos: %ld\n", file_pos);

	}
	if(var_found == 0) return -1;
	printf("Pos: %d\n", pos);

	fseek(*file, file_pos, SEEK_SET);

	for(int a = 0; a < lines_num; a ++){
		printf("Writing >%s<\n", vars_all[a]);
		fputs(vars_all[a], *file);
		free(vars_all[a]);
	}
	fflush(*file);
	ftruncate(fileno(*file), ftell(*file) + 1);
	free(vars_all);
	free(line);   /* free line variable */

	return 0;
}