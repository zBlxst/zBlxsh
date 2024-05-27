#ifndef MAIN_H
#define MAIN_H

int execute(struct cmd *cmd);
void apply_redirects(struct cmd *cmd);
int execute_builtin(struct cmd *cmd);
int exit_shell(int status);
void check_lie(char *cmd);
int check_truth(char *cmd);
void gain_good_point();

#define NOT_BUILTIN 0xab
#define NOT_GOOD 0x1
#define ALREADY_TOLD 0x2
#define BUFFER_SIZE 0x400

#endif 