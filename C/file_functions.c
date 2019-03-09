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
	/* variables for getline function */
	size_t len = 0;
	ssize_t nread;
	
	rewind(*file);	/* return to the beginning of file */
	while((nread = getline(&line, &len, svarFile)) != -1){	/* get number of lines in file */
		svarLines ++;
	}

	svarName = malloc(svarLines * sizeof(char *));	
	svarData = malloc(svarLines * sizeof(char *));

	rewind(svarFile);	/* return to the beginning of file */
	while((nread = getline(&line, &len, svarFile)) != -1){	/* read file line by line */
		if(strlen(line) > 2){	/* check if line has at least 3 chracters (name+=+\n) */
			svNp = strtok(line, "=");	/* parse svarName */
			svDp = strtok(NULL, "\n");	/* parse svarData */

			svarName[actLine] = malloc(strlen(svNp));	/* allocate memory */
			svarData[actLine] = malloc(strlen(svDp));	/* allocate memory */

			strcpy(svarName[actLine], svNp);	/* save name to svarName array */
			strcpy(svarData[actLine], svDp);	/* save data to svarData array */

			//printf("Line %s has %s\n", svarName[actLine], svarData[actLine]);
			actLine ++;
		}else{
			svarLines --;
		}
	}
}