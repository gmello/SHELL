#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_JOBS 10
#define MAX_PAR 10
#define MAX_PAR_SIZE 100
#define MAX_PATH 16
#define MAX_PATH_LENGHT 16
#define MAX_COMANDOS 100

static char *args[100], *envps[100], *mypath[10], bgflag, empty=1;
int childfgpid;
char *childname, **iovet;
int iorflag;

typedef void (*sighandler_t)(int);

struct job{
    int flag;
    char *p_name;
    int p_id;
    int j_id;
    char *p_status;
    int size;
};
struct job jobs[10];

void handle_signal(int signum);
void envia_background(char *id);
void envia_foreground(char *id);
void checkjob();
void adicionajob(int pid, char *name, int status);
void imprimejob();
void copia_env(char **envp);
char **saida(int argcn);
int parse(char *input);
int pathparse(char *pp);
int encontra_path(char *op);
void executa_processo(char *exec);
int comandos_builtin(char *op);
void cmd_line();
int main (int argc, char **argv, char **envp);

#endif SHELL_H
