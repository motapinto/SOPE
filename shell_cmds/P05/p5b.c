#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char * argv[], char * envp[]) {
	
	char * username = (char *) malloc(20);

	int i = 0;
	while (envp[i] != NULL) {

		if (strncmp(envp[i], "USER", 4) == 0) {

			strncpy(username, envp[i]+5, 20);
		}

		i++;
	}

	printf("Hello %s!\n", username);

	free(username);
	return 0;
}
