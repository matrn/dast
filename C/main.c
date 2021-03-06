#include <stdio.h>
#include <dast.h>
#include <time.h>



void callback_1();
void callback_2();


int main(int argc, char ** argv){
	DSFILE file1, file2;
	char * content;
	long len = 0;
	s_byte rtn;
	long time = 0;


	/* initialize dast */
	if(dast_init() != 0){
		perror("dast_init");
		exit(5);
	}	
	
	/* add new directory to watch */
	if(dast_watch_dir("/home/matej") != 0){
		perror("dast_watch_dir");
		exit(5);
	}


	/* add callbacks for specific files */
	if(dast_watch("test*", callback_1) != 0){
		perror("dast_watch");
		exit(5);
	}
	if(dast_watch("test2", callback_2) != 0){
		perror("dast_watch");
		exit(5);
	}

	/* run inotify daemon */
	if(dast_run() != 0){
		perror("dast_run");
		exit(5);
	}

	/* open files for read and write */
	if(dast_open_rw("test1", &file1) != 0) perror("dast_open_rw");
	if(dast_open_rw("test2", &file2) != 0) perror("dast_open_rw");



	puts("\n-------------------read---------------------");
	/* try to read test_var variable */
	if((len = dast_read_var(OLPD, "test_var", &content, file1)) >= 0){
		printf("'test_var' content >%s<\n", content);
		//printf("LEN: %ld, reutrned len: %ld\n", strlen(content), len);
		free(content);
	}
	else if(len == UNKNOWN_VAR){
		puts("Unknown variable 'test_var'");
	}
	else{
		perror("dast_read_var");
	}
	puts("-------------------read---------------------");



	puts("\n-------------------write---------------------");
	if((rtn = dast_write_var(OLPD, "test_var", "Hello World!", file1)) != ERROR){
		if(rtn == 0) puts("rewritten only one line");
		if(rtn == 1) puts("rewritten file from position of variable to the end of file");
		if(rtn == 2) puts("added to the end of file");
	}else{
		perror("data_write_var");
	}
	puts("-------------------write---------------------");



	puts("\n-------------------read---------------------");
	/* try to read test_var variable */
	if((len = dast_read_var(OLPD, "test_var", &content, file1)) >= 0){
		printf("'test_var' content >%s<\n", content);
		//printf("LEN: %ld, reutrned len: %ld\n", strlen(content), len);
		free(content);
	}
	else if(len == UNKNOWN_VAR){
		puts("Unknown variable 'test_var'");
	}
	else{
		perror("dast_read_var");
	}
	puts("-------------------read---------------------");



	puts("\n-------------------writing variable with time---------------------");
	puts("adding time to variable `test_var`");
	if(dast_add_time(TPD, "Hellow World with time stamp!", &content) != 0) perror("dast_add_time");
	if(dast_write_var(OLPD, "test_var", content, file1) == ERROR) perror("dast_write_var");
	free(content);
	puts("-------------------writing variable with time---------------------");


	puts("\n-------------------reading variable with time---------------------");
	if((len = dast_read_var(OLPD, "test_var", &content, file1)) != UNKNOWN_VAR){
		char * data;
		
		if((rtn = dast_parse_time(TPD, content, &time, &data)) < 0){
			if(rtn == -1) perror("dast_parse_time");
			if(rtn == -2) puts("delimiter not found");
			if(rtn == -3) puts("no time");
		}
		else{
			printf("'test_var' content >%s< and time %ld and it's ", data, time);

			struct tm ts;
			char buf[80];


			// Format time, "ddd yyyy-mm-dd hh:mm:ss zzz"
			ts = *localtime(&time);
			strftime(buf, sizeof(buf), "%d.%m.%Y %H:%M:%S %Z", &ts);
			printf("%s\n", buf);
		}
		//printf("LEN: %ld, reutrned len: %ld\n", strlen(content), len);
		free(data);
		free(content);
	}
	else{
		puts("Unknown variable 'test_var'");
	}
	puts("-------------------reading variable with time---------------------");
	


	puts("\n-------------------writing multiple variables---------------------");
	dict * vars;
	if(dict_init(&vars) != 0) perror("dict_init");
	if(dict_set(vars, "var_A", "Oh,") != 0) perror("dict_set");
	if(dict_set(vars, "var_B", "Hi") != 0) perror("dict_set");
	if(dict_set(vars, "var_C", "Mark") != 0) perror("dict_set");
	if(dict_set(vars, "wtf", "d̷̝͈͎̀͜a̷̻̘͔̬͈̣̓̄̈́͗ḓ̶̨͔̜͍͉̪͊̑̀̈̇͑ ̴̲͓̇̽̕w̷̙̋ͅa̶̗̅̀͌k̶̫͚̰̟͙̜͑̒̋̇̎̓ͅě̷̢̧̖͔̝͚̳͙̙̈̈́́̊͝ ̵͖̥̽̒͛̎ȕ̶̡̡̙̦̦͂́̀̔̉͂̕̚͜p̴̫̜͚͓̩̪̠͌̇͛̀̀̆̈́͠") != 0) perror("dict_set");

	if((rtn = dast_write_vars(OLPD, vars, file1)) != ERROR){
		if(rtn == 0) puts("writing multiple lines ok");
	}else{
		perror("data_write_vars");
	}
	dict_print_all(vars);
	dict_free(vars);
	puts("-------------------writing multiple variables---------------------");


	while(1){
		//puts("tick");
		sleep(1);
	}
	
	sleep(1);
	puts("killing");

	dast_close(file1);
	dast_close(file2);

	dast_cleanup();
	return 0;
}



void callback_1(char * dir_name, char * name, pid_t pid){
	puts("");
	puts("-------");
	printf("Callback_1 written by pid %d to file %s in directory %s\n", pid, name, dir_name);
}


void callback_2(char * dir_name, char * name, pid_t pid){
	puts("");
	puts("-------");
	printf("Callback_2 written by pid %d to file %s in directory %s\n", pid, name, dir_name);
}