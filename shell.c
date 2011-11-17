#include "shell.h"


/*Lida com sinais*/
void handle_signal(int signum){
    puts("");
    if (childfgpid && !empty){
        /*ctrl-c*/
        if (signum == SIGINT){
            killpg(childfgpid, SIGINT);
            signal(SIGINT, handle_signal);
        }
            /*ctrl-z*/
        else if (signum == SIGTSTP){
                killpg(childfgpid, SIGTSTP);
                adicionajob(childfgpid, childname, 2);
                signal(SIGTSTP, handle_signal);
             }
             else printf("Sinal %d desconhecido\n", signum);
    }
    /*Lida com stdout*/
    if (empty) cmd_line();
    else empty=1;
    fflush(stdout);
}

/*Transfere para background um job (parado ou nao) em foreground*/
void envia_background(char *id){
    int pid, i, found, nullflag = found = 0;
    /*Se nao foi passado argumento, o primeiro job listado parado passa a bg*/
    nullflag = (id == NULL);
    if (!nullflag) sscanf(id, "%d", &pid);
    for(i=1; i < MAX_JOBS; i++){
        if (nullflag) found = (jobs[i].flag ==3);
        else found = (((jobs[i].p_id == pid) || (jobs[i].j_id == pid)) && (jobs[i].flag !=0));
        if (found) break;
    }
    if (found>0){
        if (jobs[i].flag == 3){
            kill(jobs[i].p_id, SIGCONT);
            strcpy(jobs[i].p_status, "Executando");
            jobs[i].flag=1;
            printf("[%d]\t", i);
            printf("%s &\n", jobs[i].p_name);
            //return;
        }
        else if (jobs[i].flag ==1)
                printf("Processo %%%d ja executando em background\n", found);
    }
    else printf("Processo nao encontrado em foreground\n");
}

/*Transfere um job background para foreground*/
void envia_foreground(char *id) {
    int pid, i;
    sscanf(id, "%d", &pid);
    for(i=1; i < MAX_JOBS; i++){
        if(jobs[i].flag !=0 && ((jobs[i].p_id == pid) || (jobs[i].j_id == pid))){
            /*Habilita o espaco no vetor de jobs para sobrescrita*/
            jobs[i].flag = 0;
            kill(jobs[i].p_id, SIGCONT);
            /*Setta nome e pid do processo filho em foreground*/
            childfgpid=jobs[i].p_id;
            strcpy(childname, jobs[i].p_name);
            puts(childname);
            empty=0;
            waitpid(childfgpid,NULL, WSTOPPED);
            empty=1;
            return;
        }
    }
    puts("Processo nao encontrado em background.");
}

/*Adiciona job numa posicao valida do vetor de jobs*/
void adicionajob(int pid, char *name, int status){
    int i=1;
    while((jobs[i].flag != 0) && (i<MAX_JOBS) && (jobs[i].p_id != pid)) i++;
    if (i >= MAX_JOBS) return;
    jobs[i].p_id=pid;
    jobs[i].j_id=i;
    jobs[i].p_name = malloc(256);
    jobs[i].p_status = malloc(10);
    strcpy(jobs[i].p_name, name);
    switch (status) {
        case 1: strcpy(jobs[i].p_status, "Executando ");
                jobs[i].flag = 1;
                printf("[%d]\t", i);
                printf("%d\n", jobs[i].p_id);
                break;
        case 2: strcpy(jobs[i].p_status, "Parado  ");
                jobs[i].flag = 3;
                printf("[%d]\t", jobs[i].j_id);
                printf("%s\t", jobs[i].p_status);
                printf("%s \n",jobs[i].p_name);
                break;
        case 3: break;
                //zombie nao implementado
    }
}

/*Checa se ha job terminado no ultimo contexto, imprimindo e liberando espaco*/
void checkjob(){
    siginfo_t signal;
    int i;
    for(i = 1; i < MAX_JOBS ; i++) {
        signal.si_pid = 0;
        if (!(!waitid(P_PID ,jobs[i].p_id, &signal, WNOHANG | WEXITED)) && signal.si_pid == 0){
            if (jobs[i].flag != 0){
                printf("[%d]\t", i);
                printf("Terminado\t");
                printf("%s \n",jobs[i].p_name);
                jobs[i].flag = 0;
            }
        }
    }
}
/*Copia variaveis de ambiente para o contexto do shell*/
void copia_env(char **envp){
    int i;
    for(i = 0; envp[i] != NULL; i++){
        envps[i] = malloc(strlen(envp[i])+1);
        memcpy(envps[i], envp[i], strlen(envp[i]));
    }
}
/*Imprime jobs em background*/
void imprimejob(){
    siginfo_t signal;
    int i;
    for(i = 1; i < MAX_JOBS ; i++) {
        if ((jobs[i].flag == 0) || (jobs[i].flag == 2)) continue;
        signal.si_pid = 0;
        if ((!waitid(P_PID ,jobs[i].p_id, &signal, WNOHANG | WEXITED)) && signal.si_pid == 0){
            if (jobs[i].flag !=3) jobs[i].flag = 1;
            printf("[%d]\t", jobs[i].p_id);
            printf("%%%d\t", jobs[i].j_id);
            printf("%s\t", jobs[i].p_status);
            printf("%s &\n",jobs[i].p_name);
        }
        else jobs[i].flag = 2;
    }
}

char **saida(int argcn){
	if (argcn < 2) return 0;
	int cat, sob, err, sin;
	cat = strcmp(args[argcn-1], ">>");
	sob = strcmp(args[argcn-1], ">");
	err = strcmp(args[argcn-1], "2>");
	sin = strcmp(args[argcn-1], "<");
	if (cat && sob && err && sin) return 0;

	char **nstring = malloc(2*sizeof(char*));
	if (cat == 0){
		nstring[0] = malloc(3);
		strcpy(nstring[0], ">>");
	}
	else if (sob == 0){
			nstring[0] = malloc(2);
			nstring[0][0] = '>'; nstring[0][1] = 0;
		}
		else if (err == 0){
				nstring[0] = malloc(3);
				strcpy(nstring[0], "2>");
			}
			else if (sin == 0){
					nstring[0] = malloc(2);
					nstring[0][0] = '<'; nstring[0][1] = 0;
			}
			else return 0;

	nstring[1] = malloc(100);
	strncpy(nstring[1], args[argcn], 100);
	args[argcn-1] = 0;

	return nstring;
}

/*Verifica e adiciona entradas do PATH ao contexto*/
int pathparse(char *pp){
    char *pathstring = pp;
    int countsize, npath = 0; int i = 0;
    /*Itera ate o fim de PATH ou numero maximo atingido*/
    for (npath=0; npath<MAX_PATH; npath++){
        char tmp[MAX_PATH_LENGHT];
        bzero(tmp, MAX_PATH_LENGHT);
        countsize = 0;
        /*Testa divisores*/
        if (pathstring[i] == ':') i++;
        while(pathstring[i] != ':'){
            if ((pathstring[i] == '\0')) break;
            strncat(tmp, &pathstring[i],1);
            i++; countsize++;
        }
        if (countsize > MAX_PATH_LENGHT*MAX_PATH) return 1;

        /*Checa integridade de mypath[] e aloca*/
        if(mypath[npath] == NULL)
            mypath[npath] = malloc(strlen(tmp)+1);
        else bzero(mypath[npath], strlen(mypath[npath]));
        /*Copia parametro lido pra mypath[]]*/
        strncpy(mypath[npath], tmp, strlen(tmp));
        strncat(mypath[npath], "/\0", 2);
        if (pathstring[i] == '\0') break;
    }
    return 0;
}
/*Procura no PATH os comandos a serem executados*/
int encontra_path(char *op){
    char tmp[MAX_PAR_SIZE];
    bzero(tmp, MAX_PAR_SIZE);
    int i, fd;
    /*Pra cada um dos paths, concatena o caminho+comando*/
    for(i=0; mypath[i] != NULL; i++){
        strcpy(tmp, mypath[i]);
        strncat(tmp, op, strlen(op));
        /*Testa se encontrou o caminho do comando concatenado*/
        if((fd = open(tmp, O_RDONLY)) >= 0){
            strncpy(op, tmp, strlen(tmp));
            close(fd);
            return 0;
        }
    }
    return 1;
}

/*Faz o parse na string de entrada*/
int parse(char *input){
    /*Testa se a string esta vazia = enter limpo*/
    if(input[0]=='\n') return 5;
    /*Amputa o \n. Aponta pro input */
    input[strlen(input)-1]=0;
    char *s = input;
    /*Setta flag de processo em background = 0*/
    bgflag=0;
    int countsize, narg = 0; int i = 0;
    /*Roda ate o fim dos parametros ou numero max_par atingido*/
    for (narg = 0; narg < MAX_PAR; narg++){
        char tmp[MAX_PAR_SIZE] = {};
        countsize = 0;
        /*Ignora espaÃ§os repetidos*/
        while (s[i] == ' ') i++;
        if ((s[i] == '&')){
            /*Se encontrou &, ignora e setta flag pra nao dar wait no executa_processo*/
            if (i==0) return 3;
            bgflag=1;
            i++;
        }
        while(s[i] != ' '){
            if ((s[i] == '\0')) break;
            if ((s[i] != '&')){
                strncat(tmp, &s[i],1);
                i++; countsize++;
            }
            else {
            /*Se encontrou & grudado em comando, setta flag*/
                bgflag=1;
                break;
            }
        }
        /*Se estourou tamanho maximo de parametros*/
        if (countsize > MAX_PAR_SIZE) return 1;

        /*Se tmp = \0. Se caiu no caso do & no final da string*/
        if (!strcmp(tmp, "\0")) {
			iovet = saida(narg-1);
			if(iovet){
				iorflag=1;
			}
			return 0;
		}
        /*Checa integridade de argv[] e aloca*/
        if(args[narg] == NULL)
            args[narg] = malloc(strlen(tmp)+1);
        else bzero(args[narg], strlen(args[narg]));
        /*Copia parametro lido pra args[]*/
        strncpy(args[narg], tmp, strlen(tmp));
        strncat(args[narg], "\0", 1);
    }
    if (narg >= MAX_PAR) return 2;
    return 0;
}

/*Depois do parse, multiplexa acoes a serem tomadas*/
int comandos_builtin(char *op){
    char dir [256];
    int i,fd, flag=0, sc=0;
    /*Le comando de args*/
    strncpy(op, args[0], strlen(args[0]));
    strncat(op, "\0", 1);

    /*Testa se nao ha caminho no comando*/
    if(index(op, '/') == NULL) {
        /*Se encontrou o caminho real do comando executa*/
        if(encontra_path(op) == 0) executa_processo(op);
        else flag = 1;
    } else
        /*Se foi dado o caminho inteiro*/
        if((fd = open(op, O_RDONLY)) > 0){
            close(fd);
            executa_processo(op);
        } else flag = 1;

    if      (!strcmp(op,"pwd"))     sc = 1;
    else if (!strcmp(op,"cd"))      sc = 2;
    else if (!strcmp(op,"exit"))    sc = 3;
    else if (!strcmp(op,"jobs"))    sc = 4;
    else if (!strcmp(op,"bg"))      sc = 5;
    else if (!strcmp(op,"fg"))      sc = 6;
    else if (!strcmp(op,"about"))   sc = 7;

    if (flag == 1){
        flag = 0;
        switch(sc){
            case 1: getcwd(dir, 256);
                    printf("%s\n", dir);
                    break;
            case 2: if (chdir(args[1]) != 0)
                        printf("Caminho invalido.\n");
                    break;
            case 3: for (i=1;i<MAX_JOBS && jobs[i].p_id != 0;i++)
                        kill(jobs[i].p_id, 15);
                    exit(0);
            case 4: imprimejob();
                    break;
            case 5: envia_background(args[1]);
                    break;
            case 6: if (args[1] != NULL) {
                        envia_foreground(args[1]);
                    }
                    else puts("Parametro (PID ou JID) requerido.");
                    break;
            default:flag = 1;
                    break;
        }
    }
    return flag;
}

/*Chama e executa o comando num processo filho*/
void executa_processo(char *exec){

    int d, tmpid, ioid, fflg;
    childname = malloc(strlen(exec));
    if((tmpid=fork()) == 0){
        setpgid(0,0);
        if (iorflag){
			if (!strcmp(iovet[0], ">>")){
				fflg = O_RDWR | O_CREAT | O_APPEND;
				d=1;
			}

			else if (!strcmp(iovet[0], ">")){
				fflg = O_RDWR | O_CREAT | O_TRUNC;
				d=1;
			}

			else if (!strcmp(iovet[0], "2>")){
				fflg = O_RDWR | O_CREAT | O_APPEND;
				d=2;
			}
			else if (!strcmp(iovet[0], "<")){
				fflg = O_RDONLY;
				d=0;
			}

			int fdesc = open(iovet[1], fflg, S_IRWXU);

	        ioid = fork();
          	if (ioid == 0){
	                dup2(fdesc, d);
                  	int i = execve(exec, args, envps);
        			if(i < 0) {
           				printf("%s: Operacao ilegal.\n", exec);
            			exit(1);
        			}
          		}
          		else {
					waitpid(ioid,NULL, WSTOPPED);
					exit(1);
				}
		/*Se nao ha redirecionamento de io*/
		} else {
				int i = execve(exec, args, envps);
				if(i < 0) {
					printf("%s: Operacao ilegal.\n", exec);
					exit(1);
				}
			}
    /*Se encontrou &, nao espera pra retornar ao contexto e adiciona lista jobs*/
    } else if (!bgflag) {
                childfgpid=tmpid;
                strcpy(childname, exec);
                empty=0;
                waitpid(childfgpid,NULL, WSTOPPED);
                empty=1;
            }
        else adicionajob(tmpid, exec,1);
    iorflag=0;
}
/*Funcao printa a linha de comando*/
void cmd_line(){
    char path[32];
    printf("[SUPER SHELL] %s$ ", getcwd(path, 32));
    fflush(stdin);
}

