#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ctype.h>

#define MAX_PATH_LEN 1024
#define MAX_INPUT_SIZE 255
char **path;
size_t path_size;
char error_message[30] = "An error has occurred\n";
void execute_cmd(char* cmd);

bool isWhitespace(char c) { //check if the input is just whitespace
	return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f';
}

bool isLineWhitespace(const char *line) {
       	while (*line) {
		if (!isWhitespace(*line)) {
			return false;  // Found a non-whitespace character
		}
		line++;  // Move to the next character
	}
	return true;  // All characters are whitespace
}

//remove white space
char * removeWhiteSpace(char* input){ //removes leading and ending white space from input
	size_t len = strlen(input);
	char *mutable_input = malloc(len+1);
	if(mutable_input == NULL){		//makes sure there is memory for the input
		perror("malloc");
		exit(1);
	}
	strcpy(mutable_input, input);		//saves it into the input that was given

	size_t i = 0;
	while(isspace(mutable_input[i]) && i < len){	//checks each individual character for whitespace
		i++;
	}

	//shift string to remove leading whitespace
	memmove(mutable_input, mutable_input+i, len-i+1);

	if(mutable_input[0] == '\0'){
		return mutable_input;	//if it starts with a null then just return
	}

	i =strlen(mutable_input)-1;
	//set strlen to right before the null
	while(i>0){
		if(mutable_input[i] == ' ' || mutable_input[i] == '\n' || mutable_input[i] == '\t') i--;
		//remove end white spaces
		else{ break; }
	}
	mutable_input[i + 1] = '\0';
	//place new null character

	return mutable_input;
}

//makes the default directory /bin
void initializePath(){
		path_size = 1; //initially, with only /bin
		path = (char **)malloc(path_size * sizeof(char *));
		path[0] = strdup("/bin");
}

//update path with path command
void updatePath(char *new_path){
	//clear path
	for(size_t i = 0; i < path_size; i++){
		free(path[i]);
	}
	
	//split up new path into directories
	char *split_path = strdup(new_path);
	char *token = strtok(split_path, " \t\n");
	path_size = 0;

    	while (token != NULL) {	//add new paths to the array and reallocate memory for each of them
		path_size++;
		path = (char **)realloc(path, path_size * sizeof(char *));
		path[path_size - 1] = strdup(token);
		token = strtok(NULL, " \t\n");
	}
	free(split_path); //free the copy variable when done
}

void skipConsecutiveAnds(char *input){
	int i, j;
	int count = 0;  // Count the number of consecutive '&' characters
	int hasNonSpace = 0;
	for (i = 0, j = 0; input[i] != '\0'; i++) {
		if (input[i] == '&') {
			count++;
			if (count == 1 || (input[i - 1] != ' ' && hasNonSpace)) {
				//if it's the first '&' or there's no space before it, keep it
				input[j++] = '&';
			}
		} else {
			// Reset count and update the nonspace flag
			count = 0;
			hasNonSpace = (input[i] != ' ');
			input[j++] = input[i];
		}
	}
	input[j] = '\0';  // Null-terminate the modified string
}


int main(int argc, char* argv[]) {
	if(argc > 1){ //make sure rush isnt called with extra arguments
		write(STDERR_FILENO, error_message, strlen(error_message));
		exit(EXIT_FAILURE);
		fflush(stderr);
	}

	char *user_input = NULL;	//variable for user input
	size_t length = 255;	//at most 255 characters
	initializePath();

	while(1){			//while loop
		printf("rush> ");	//call the prompt to the stdout
		fflush(stdout);		//make sure solution prints everything
		
		//read user input
		getline(&user_input, &length, stdin);
		if(isLineWhitespace(user_input)){
			continue;
		}

		user_input = removeWhiteSpace(user_input); //remove leading and ending white space from input
		char *cmd;
		skipConsecutiveAnds(user_input); //skip extra ands for cases with multiple in a row
		//parse the user input and run the code
		cmd = strtok(user_input, "&\n"); //read the first command for parallel commands 

		while(cmd!=NULL){
			char *cmd_copy = strdup(cmd); //make a copy in case u change cmd

			cmd_copy = removeWhiteSpace(cmd_copy); //remove leading and ending white space
	
			char* phrase = strsep(&cmd_copy, " \t\n");  //read the first word in command
			
			//check for built in commands
			if(strcmp(phrase, "exit") == 0){        //check for exit
				if(strsep(&cmd_copy, " \n\t")==NULL){ //check for extra arguments
					exit(0);
				}
				else{
					write(STDERR_FILENO, error_message, strlen(error_message)); //if exit has extra args, print error
					break;
				}
			}
			else if(strcmp(phrase, "path") == 0) { // Handle path command
				char *new_path = strsep(&cmd_copy, "\n");
				if(new_path == NULL){	//if path is called with no arguments make path empty
					updatePath(" ");
					break;
				} else {
					updatePath(new_path); //change path to new arguments 
					break;
				}
			}
			else if(strcmp(phrase, "cd") == 0){
				//does cd have one arg
				int cdcount = 0;
				char *file;
				char *ptr = strsep(&cmd_copy, " \t\n"); //read argument
				while(ptr!=NULL){
					cdcount++;
					file = ptr;
					ptr = strsep(&cmd_copy, " \n\t");
					//reads extra args
				}
				if(cdcount==0){	//if cd has no arguments error
					write(STDERR_FILENO, error_message, strlen(error_message));
					fflush(stderr);
					break;
				}
				if(cdcount==1){ //if cd has only one argument change the directory
					if(chdir(file)!=0){
						perror("chdir");
					}
					break;
				}
				if(cdcount>1){ //if cd has more than one argument error
					write(STDERR_FILENO, error_message, strlen(error_message));
					fflush(stderr);
					break;
				}
			}	
			else{			
				cmd = removeWhiteSpace(cmd); //remove leading and ending white space from the command
		
				int pid = fork(); //fork to execute the external command 
				if(pid < 0){
					write(STDERR_FILENO, error_message, strlen(error_message));
				}
				else if(pid==0){
					//child execution
					execute_cmd(cmd);
				}
				else{
					cmd = strtok(NULL, "&\n");
					//parent continue parsing
				}
			}
		}
		while(wait(NULL) > 0) {} //wait for all processes to end before reprinting rush
	}
	free(user_input);				//free memory
	for(size_t i = 0; i < path_size; i++){
		free(path[i]);
	}	
	free (path);
	return 0;
}


void execute_cmd(char *cmd){ //function for external commands
	char *cmd_copy = strdup(cmd);
	char* phrase = strtok(cmd, " \t\n"); //read the command
	char* test;
	int redir = 0; //variables to check for redirection
	int count = 0;
	char* new_input = strdup(cmd_copy); //copies for when i change input
	char* save_input = strdup(new_input);
	while((test=strsep(&new_input, " \t\n"))!=NULL){ //check for redirection
		if(strcmp(test, ">")==0){
			redir++; //if found, break and test for redirection conditions
			if(count==0){
				break;
			}
		}
		count++;
	}
	if(count==0){ //if redir is the first argument, error
		 write(STDERR_FILENO, error_message, strlen(error_message));
		 exit(1);
	}
	if(redir>1){ //if redir has more than one output file error
		write(STDERR_FILENO, error_message, strlen(error_message));
		exit(1);
	}
	char full_path[MAX_PATH_LEN]; //array for checking the path
	//external commands
	for(size_t i = 0; i < path_size; i++){
		snprintf(full_path, sizeof(full_path), "%s/%s", path[i], phrase); //check to see if the command exists in a path
		if(access(full_path, X_OK)==0){
			//if executable exists
			char *token;
			char *external[MAX_INPUT_SIZE];
			int i = 0;
			while((token = strsep(&save_input, " \t"))!=NULL){ //double check for redirection
				token = removeWhiteSpace(token);
				if(strcmp(token, ">")==0){ //if redirection is found, only save the input before the > sign
					redir = 1;
					break;
				}
				external[i] = token; //set each word of the command into the external array to be used for execv
				i++;
			}
			external[i] = 0;	//make sure it is null terminating
			if(redir==1){		//if redirection, handle it
				char *output_filename = strsep(&save_input, " \t\n"); //save output after the > sign
				char *check = strsep(&save_input, " \t\n"); 	//check to make sure there is nothing after output
				if(output_filename==NULL || check != NULL){ 
					write(STDERR_FILENO, error_message, strlen(error_message));	//if too many things after > sign, error
					exit(1);
				}
				int output_file = open(output_filename, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU); //else open the file to be redirected to
				if(output_file < 0){
					write(STDERR_FILENO, error_message, strlen(error_message)); //if file fails, error
				}
				dup2(output_file, STDOUT_FILENO); //redirect stdout to the file
				close(output_file);
			}
			execv(full_path, external);	//execute external commands to the shell or to the output file
			write(STDERR_FILENO, error_message, strlen(error_message));
			exit(1);
		}
	}
	if(strcmp(phrase, "&")!=0){ //ignore extra & signs as they should not be considered commands
		write(STDERR_FILENO, error_message, strlen(error_message));
	}
	exit(1); //exit the function
}
