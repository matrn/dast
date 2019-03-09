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

	int pos = 0;   /* for finding correct position of delimiter */


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