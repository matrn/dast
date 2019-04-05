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
	  -2 = string doesn't contain time
	  -1 = delimiter not found
	  0 = success
	*/

	//char format[10];
	char * time_str;
	char delim_str[2];

	delim_str[0] = delimiter;
	delim_str[1] = 0;

	if(in_str(input, delimiter)){
		time_str = strtok_r(input, delim_str, &*data);

		errno = 0;	/* To distinguish success/failure after call */
		if(time_str) *time = strtol(time_str, NULL, 10);
		else return -2;   /* no time is in input string */

		/* Check for various possible errors */

		if ((errno == ERANGE && (*time == LONG_MAX || *time == LONG_MIN)) || (errno != 0 && *time == 0)) {
			//perror("strtol");
			//exit(EXIT_FAILURE);
			return -3;   /* strtol error */
		}

		
		/*
		sprintf(format, "%%lu%c%%ms", delimiter);
		printf("input >%s<\n", input);
		printf("sscanf: %d\n", sscanf(input, "%lu-%ms", &*time, &*data));
		*/
		//printf("Time >%s< data >%s<\n", time_str, *data);
		return 0;   /* success */
	}
	else{
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