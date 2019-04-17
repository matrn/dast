#include "dast.h"


void dast_add_time(char delimiter, char * data, char ** output){
	long unix_time;
	
	unix_time = (unsigned long)time(NULL);
	//printf("%lu", unix_time);
	*output = malloc(strlen(data) + 20);

	sprintf(*output, "%lu%c%s", unix_time, delimiter, data);
}


s_byte dast_parse_time(char delimiter, char * input, long * time, char ** data){
	/*
	 return values:
	  -3 = strtol error
	  -2 = string doesn't contain time - delimiter is here but time not
	  -1 = delimiter not found
	  0 = success
	*/

	char * time_str;

	unsigned int input_len = 0;
	unsigned int delim_pos = 0;


	input_len = strlen(input);
	
	if(in_str(input, delimiter)){   /* check delimiter in input string */
		delim_pos = get_pos(input, delimiter);   /* get position of delimiter, we can use unsigned int because if string contains delimiter was already checked by in_str() function */

		/* allocate memory for data and copy them */
		*data = malloc(input_len - delim_pos + 1);
		strcpy(*data, input + delim_pos + 1);


		if(delim_pos == 0) return -2;   /* return that delimiter is here but time not */


		/* allocate memory for time string and copy them into this string */
		time_str = malloc(delim_pos + 1);   /* allocate memory for time, +1 is for ending 0 */
		strncpy(time_str, input, delim_pos);   /* copy time without data */
		time_str[delim_pos] = 0;   /* end of str */

		errno = 0;	/* to distinguish success/failure after call */
		*time = strtol(time_str, NULL, 10);

		free(time_str);   /* free time string variable */


		/* check for various possible errors during strtol() */
		if ((errno == ERANGE && (*time == LONG_MAX || *time == LONG_MIN)) || (errno != 0 && *time == 0)) {
			//perror("strtol");
			//exit(EXIT_FAILURE);
			return -3;   /* strtol error */
		}

		return 0;   /* success */
	}
	else{
		/* these two lines are for prevent failure of strlen() or other during the function usage */
		*data = malloc(1);
		*data[0] = 0;

		return -1;   /* string doesn't contain delimiter */
	}
}



byte in_str(char * input, char character){
	/*
	 return values:
	  0 = if string doesn'ŧ contain any specified character
	  1 = if yes
	*/

	unsigned int pos;

	for(pos = 0; pos < strlen(input); pos ++){
		if(input[pos] == character) return 1;
	}

	return 0;
}



ssize_t get_pos(char * input, char character){
	/*
	 return values:
	  -1 = if character was not found 
	  0 - X = pos
	*/

	char * rtn;

	rtn = strchr(input, character);

	if(rtn == NULL){   /* check if there is at least one character in input */
		return -1;   /* return no character found */
	}
	else{
		return (ssize_t)(rtn - input);   /* return position of first character in input string */
	}
}