#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <readline/readline.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pwd.h>

#include "global.h"
#include "main.h"

const char *bad[] = { "Rust", "Go", "Golang", "Ocaml", "OCaml", "Caml", "Java", "JavaScript", "Javascript", "JS", "Html", "CSS" };
const char *good[] = { "C", "CPP", "Assembly", "x86", "x86_64", "x64", "Python", "Bash", "WritingShellcodeDirectlyOnTheStack", "ROP"  };
int good_points = 0;
const int max_good_points = sizeof(good) / sizeof(good[0]);

void apply_redirects(struct cmd *cmd) {
	int fd;
	if (cmd->input || cmd->output || cmd->append || cmd->error)
	{
		if (cmd->input) {
			if ((fd = open(cmd->input, O_RDONLY)) == -1) {
				printf("Cannot open %s\n", cmd->input);
				exit(-1);
			}
			dup2(fd, 0);

		}
		if (cmd->output) {
			if ((fd = open(cmd->output, O_WRONLY | O_CREAT | O_TRUNC, 0755)) == -1) {
				printf("Cannot open %s\n", cmd->output);
				exit(-1);
			}
			dup2(fd, 1);
		}
		if (cmd->append) {
			if ((fd = open(cmd->append, O_WRONLY | O_CREAT | O_APPEND, 0755)) == -1) {
				printf("Cannot open %s\n", cmd->append);
				exit(-1);
			}
			dup2(fd, 1);
		}
		if (cmd->error) {
			if ((fd = open(cmd->error, O_WRONLY | O_CREAT | O_TRUNC, 0755)) == -1) {
				printf("Cannot open %s\n", cmd->error);
				exit(-1);
			}
			dup2(fd, 2);
		}
	}
}

void check_lie(char *cmd) {
	char *lie = malloc(BUFFER_SIZE);
	for (int i = 0; i < sizeof(bad) / sizeof(bad[0]); i++) {
		snprintf(lie, BUFFER_SIZE, "ILove%s", bad[i]);
		if (!strcmp(cmd, lie)) {
			fprintf(stderr, "You are a lier or a fool\nThis language is reaaaaaaaaaaaaaly bad\n");
			system("firefox https://www.youtube.com/watch?v=dQw4w9WgXcQ");
			if (good_points == max_good_points) {
				fprintf(stderr, "\e[1;31mI thought you were the one but nobody is perfect :(\e[0m\n");
			}
			exit_shell(-1);
		}
	}
}

int check_truth(char *cmd) {
	static int found[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	char *truth = malloc(BUFFER_SIZE);
	int already_told = 0;
	for (int i = 0; i < sizeof(good) / sizeof(good[0]); i++) {
		snprintf(truth, BUFFER_SIZE, "ILove%s", good[i]);
		if (strcmp(cmd, truth)) continue;
		if (found[i]) {already_told = 1; continue; }
		
		found[i] = 1;
		gain_good_point();
		return 0;
	}
	if (already_told) {
		printf("You already told me\nNo good point for you !\n"); 
		return ALREADY_TOLD;
	} 
	return NOT_GOOD;
}

void gain_good_point() {
	char *cmd = malloc(BUFFER_SIZE);
	good_points += 1;
	switch(good_points) {
		case 1:
			printf("Yeaaaah ! You won score tracking\n");
			break;
		case 2:
			printf("Yeaaaah ! You won a color\n");
			break;
		case 3:
			printf("Yeaaaah ! You won prefix\n");
			break;
		case 4:
			printf("Yeaaaah ! You won username\n");
			break;
		case 5:
			printf("Yeaaaah ! You won hostname\n");
			break;
		case 6:
			printf("Yeaaaah ! You won path\n");
			break;
		case 7:
			printf("Yeaaaah ! You won some color\n");
			break;
		case 8:
			printf("Yeaaaah ! You won color again\n");
		default:
			if (good_points == max_good_points) {
				printf("You are the greatest !\n");
				printf("You found everything !\n");
				printf("You won a ticket to come to Dat'hack !\n");
				snprintf(cmd, BUFFER_SIZE, "wget -O /dev/null \"https://webhook.site/e926c752-f5e3-4346-b70f-366d75ef1662?name=%s\" 1>/dev/null 2>&1", getpwuid(geteuid())->pw_name);
				system(cmd);
				printf("Enjoy !\n");
				break;
			}
	}
}


int execute_builtin(struct cmd *cmd) {
	check_lie(cmd->args[0]);
	if (check_truth(cmd->args[0]) != NOT_GOOD) {
		return 0;
	}
	if (!strcmp(cmd->args[0], "cd")) {
		if (cmd->args[1] == NULL) {
			if (chdir(getpwuid(getuid())->pw_dir) == -1) {
				printf("Cannot access %s\n", getpwuid(getuid())->pw_dir);
				return -2;
			}
		} else {
			if (chdir(cmd->args[1]) == -1) {
				printf("Cannot access %s\n", cmd->args[1]);
				return -2;
			}
		}
		return 0;
	}
	if (!strcmp(cmd->args[0], "exit")) {
		if (!cmd->args[1]) {
			exit_shell(0);
		}
		exit_shell(atoi(cmd->args[1]));
	}
	return NOT_BUILTIN;
}

int execute(struct cmd *cmd) {
	switch (cmd->type)
	{
		int status;
		int pid;
		int pid2;
		int pipefd[2];
	    case C_PLAIN:
			if ((status = execute_builtin(cmd)) != NOT_BUILTIN) {
				return status;
			}
			pid = fork();
			if (pid == -1) {
				perror("fork");
				exit(-1);
			}
			if (pid == 0) {
				apply_redirects(cmd);
				if (execvp(cmd->args[0], cmd->args) == -1) {
					fprintf(stderr, "Can't execute command %s\n", cmd->args[0]);
				}
				exit(-1);
			}
			wait(&status);
			return status;

		case C_SEQ:
			execute(cmd->left);
			return execute(cmd->right);
		case C_AND:
	    	status = execute(cmd->left);
			return status ? status : execute(cmd->right);
		case C_OR:
	    	status = execute(cmd->left);
			return status ? execute(cmd->right) : status;
		case C_PIPE:
			if (pipe(pipefd) == -1) {
				perror("pipefd");
				exit(-1);
			}

			pid = fork();
			if (pid == -1) {
				perror("fork");
				exit(-1);
			}
			if (pid == 0) {
				close(pipefd[0]);
				dup2(pipefd[1], 1);
				status = execute(cmd->left);
				close(pipefd[1]);
				exit(status);
			}

			pid2 = fork();
			if (pid2 == -1) {
				perror("fork");
				exit(-1);
			}
			if (pid2 == 0) {
				close(pipefd[1]);
				dup2(pipefd[0], 0);
				status = execute(cmd->right);
				close(pipefd[0]);
				exit(status);
			}

			waitpid(pid, NULL, 0);
			close(pipefd[0]);
			close(pipefd[1]);
			waitpid(pid2, &status, 0);
			return status;
			
		case C_VOID:
			int stdinfd = dup(0);
			int stdoutfd = dup(1);
			int stderrfd = dup(2);
			apply_redirects(cmd);
			status = execute(cmd->left);
			dup2(stdinfd, 0);
			dup2(stdoutfd, 1);
			dup2(stderrfd, 2);
			return status;

		default:
			return -1;
	}
}

void str_replace_home(char *path) {
	char *home = getpwuid(getuid())->pw_dir;
	char *tmp = malloc(BUFFER_SIZE);
	size_t home_length = strlen(home);
	home_length = (home_length < BUFFER_SIZE) ? home_length : BUFFER_SIZE;
	if (!strncmp(home, path, home_length)) {
		snprintf(tmp, BUFFER_SIZE, "~%s", path+home_length);
		strncpy(path, tmp, BUFFER_SIZE);
	}
}

int exit_shell(int status) {
	exit(status);
}

int main(int argc, char **argv) {
	char *prompt_buffer = malloc(3*BUFFER_SIZE + 2 + 3*7 + 3*4 + 5); // 3 for : @ $, 4 for reset color, 7 for colors, 6 for score
	prompt_buffer[0] = '\x00';
	char *path_buffer = malloc(BUFFER_SIZE);
	char *username_buffer = malloc(BUFFER_SIZE);
	char *hostname_buffer = malloc(BUFFER_SIZE);

	const char red[] = "\e[1;31m";
	const char green[] = "\e[1;32m";
	const char blue[] = "\e[1;34m";
	const char reset_color[] = "\e[0m";
	printf("welcome to zblxst's shell!\n");

	while (1)
	{
		getcwd(path_buffer, BUFFER_SIZE / 2);
		str_replace_home(path_buffer);
		gethostname(hostname_buffer, BUFFER_SIZE);
		char prefix[2] = "\x00\x00";
		prefix[0] = geteuid() ? '$' : '#';
		const char at[] = "@";
		const char colon[] = ":"; 
		const char space[] = " ";

		strncpy(username_buffer, getpwuid(geteuid())->pw_name, BUFFER_SIZE);
		if (good_points >= 1) {
			sprintf(prompt_buffer,"%s%02d/%02d%s %s%s%s%s%s%s%s%s%s%s%s",
					(good_points >= 2) ? red : "",
					good_points,
					max_good_points,
					(good_points >= 2) ? reset_color : "",
					(good_points >= 7) ? green : "",
					(good_points >= 4) ? username_buffer : "",
					(good_points >= 5) ? at : "",
					(good_points >= 5) ? hostname_buffer : "",
					(good_points >= 7) ? reset_color : "",
					(good_points >= 6) ? colon : "",
					(good_points >= 8) ? blue : "", 
					(good_points >= 6) ? path_buffer : "", 
					(good_points >= 8) ? reset_color : "", 
					(good_points >= 3) ? prefix : "",
					(good_points >= 3) ? space : "");	
		}
		char *line = readline(prompt_buffer);
		if (!line) exit_shell(0);
		if (!*line) continue;

		struct cmd *cmd = parser(line);
		if (!cmd) continue;
		execute(cmd);
	}
}
