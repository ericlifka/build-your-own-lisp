#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <string.h>

/* Fake readline and add_history functions to shim win32 */
static char buffer[2048];

char* readline(char* prompt) {
	fputs(prompt, stdout);
	fgets(buffer, 2048, stdin);
	char* cpy = malloc(strlen(buffer)+1);
	strcpy(cpy, buffer);
	cpy[strlen(cpy)-1] = '\0';
	return cpy;
}

void add_history(char* unused) { }

#else
#include <editline/readline.h>
#endif

int main(int argc, char** argv) {
	puts("Lispy Version 0.0.0.0.1");
	puts("Press Ctr+c to Exit\n");

	while (1) {
		char* input = readline("lispy> ");
		add_history(input);
		printf("No you're a %s\n", input);
		free(input);
	}

	return 0;
}
