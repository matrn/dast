#include "dast.h"


/*
int main(){
	dict * test;
	char * value;
	s_byte rtn;


	if(dict_init(&test) != 0) perror("dict_init");
	if(dict_set(test, "test", "Hello World!") != 0) perror("dict_set");
	if(dict_set(test, "test2", "Hello World2!") != 0) perror("dict_set");

	dict_print_all(test);
	
	if((rtn = dict_get(test, "test", &value)) == 0){
		printf("Value: >%s<\n", value);
		free(value);
	}
	else if(rtn == -2){
		puts("key doesn't exists");
	}
	else{
		perror("dict_get");
	}

	dict_free(test);
	
	return 0;
}
*/


s_byte dict_init(dict ** head){
	/* return value:
		0  = all is OK
		-1 = error- probably malloc
	*/

	*head = NULL;
	if((*head = malloc(sizeof(dict))) == NULL) return -1;

	/* these two allocations are to prevent free() function fail */
	if(((*head)->key = malloc(1)) == NULL) return -1;
	if(((*head)->value = malloc(1)) == NULL) return -1;

	(*head)->key[0] = 0;   /* null key - nothing is there */
	(*head)->value[0] = 0;   /* null value */
	(*head)->next = NULL;   /* null next pointer */

	return 0;
}


s_byte dict_set(dict * head, char * key, char * value){
	/* return value:
		0  = all is OK
		-1 = error- probably malloc
	*/

	dict * current = head;
	
	while(1){
		//printf("current >%s< vs key >%s<\n", current->key, key);
		if(strcmp(current->key, key) == 0){   /* check if passed key is same as current key */
			current->value = realloc(current->value, strlen(value) + 1);   /* if yes, reallocate memory for new data */
			strcpy(current->value, value);   /* copy new value to this array value */

			#ifdef DEBUG
				printf("Found key in linked list, copied key >%s<\n", current->key);
			#endif

			return 0;
		}
		if(current->next == NULL) break;   /* check if there is next node */

		current = current->next;   /* save next node as current */
	}

	if(current->key[0] == 0){   /* if current key is blank, it's probably key from dict_init initialization */
		#ifdef DEBUG
			puts("added to current");
		#endif
		/* allocate memory and copy data to key & value variables */
		if((current->key = malloc(strlen(key) + 1)) == NULL) return -1;
		if((current->value = malloc(strlen(value) + 1)) == NULL) return -1;

		strcpy(current->key, key);
		strcpy(current->value, value);

		return 0;
	}

	/* let's create new node */
	if((current->next = malloc(sizeof(dict))) == NULL) return -1;
	if((current->next->key = malloc(strlen(key) + 1)) == NULL) return -1;
	if((current->next->value = malloc(strlen(value) + 1)) == NULL) return -1;

	strcpy(current->next->key, key);
	strcpy(current->next->value, value);
	//printf("key %s\n", current->next->key);
	current->next->next = NULL;   /* NULL next pointer from new node */

	#ifdef DEBUG
		puts("Added new node");
	#endif


	return 0;
}


s_byte dict_get(dict * head, char * key, char ** value){
	/* return value:
		0  = all is OK
		-1 = error- probably malloc
		-2 = key was not found
	*/

	dict * current = head;

	while(1){
		if(strcmp(current->key, key) == 0){
			if((*value = malloc(strlen(current->value) + 1)) == NULL) return -1;
			strcpy(*value, current->value);

			return 0;
		}

		if(current->next == NULL) break;
		current = current->next;
	}


	return -2;
}


void dict_print_all(dict * head){   /* function for printing all key & value from all nodes */
	dict * current = head;

	while(1){
		printf(">%s< : >%s<\n", current->key, current->value);
		if(current->next == NULL) break;
		current = current->next;
	}
}


void dict_free(dict * head){
	dict * current = head;
	dict * next;

	while(current->next != NULL){
		/* free key & value string variables */
		free(current->key);
		free(current->value);

		if(current->next != NULL){
			next = current->next;   /* save next pointer */
			free(current->next);   /* free next pointer */
			current = next;   /* go to next pointer */
		}
	}
}