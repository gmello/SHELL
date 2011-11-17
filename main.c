#include "shell.h"

int main (int argc, char **argv, char **envp){

    /*Declaracoes e alocacao de variaveis de ambiente*/
    char str[200], command[MAX_PAR_SIZE];
    int i, errcode, errcomandos_builtin;
    copia_env(envp);
    iorflag = 0;

    /*Sinais*/
    signal(SIGTSTP, handle_signal);
    signal(SIGINT, handle_signal);

    /*Limpa tela antes de tudo*/
    if(fork() == 0){
        execve("/usr/bin/clear", argv, envp);
        exit(1);
    } else wait(NULL);

    /*Lida com PATH*/
    char *path = getenv("PATH");
    if (pathparse(path)!=0){
        printf("Nao foi possivel exportar PATH.\n");
        exit(1);
    }
    while(1){
        /*A cada iteracao, limpa strings*/
        bzero(command, MAX_PAR_SIZE);
        bzero(str, 200);
        cmd_line();
        fgets(str, LINE_MAX, stdin);
        errcode = parse(str);
        /*Trata erros do parsing*/
        switch (errcode){
            case 0: errcomandos_builtin = comandos_builtin(command);
                    if (errcomandos_builtin!=0) printf("%s : Comando nao encontrado.\n", command);
                    break;
            case 1: printf("Tamanho (%d) de parametro violado. \n", MAX_PAR_SIZE);
                    break;
            case 2: printf("Numero maximo (%d) de parametros violado. \n", MAX_PAR);
                    break;
            case 3: printf("Sintaxe invalida.\n");
                    break;
            default: break;
        }
        /*Limpa e desaloca args a cada execucao*/
        for(i = 0; args[i]!= NULL; i++){
            bzero(args[i], strlen(args[i])+1);
            args[i] = NULL;
            free(args[i]);
        }
        checkjob();
    }
    return 0;
}
