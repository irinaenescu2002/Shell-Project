#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <signal.h>

#define charSize sizeof(char)
#define intSize sizeof(int)
#define longSize sizeof(long)

#define maxSizeCommand 1000

/// Functia MAN ce ofera un manual de utilizare al shell-ului

void manual(){
    printf("MANUAL\n");
    printf("01. MAN: shows all the commands\n");
    printf("    Command: man\n");
    printf("02. HISTORY: shows all the commands that ran in the current session\n");
    printf("    Command: history\n");
    printf("03. CLEAR: clears the terminal\n");
    printf("    Command: clear\n");
    printf("04. PWD: prints the path of the current directory\n");
    printf("    Command: pwd\n");
    printf("05. LS: lists all files and directories of the current directory\n");
    printf("    Command: ls\n");
    printf("06. CD: changes the current directory\n");
    printf("    Command: cd new_path\n");
    printf("07. TOUCH: creates a new empty file\n");
    printf("    Command: touch file_name\n");
    printf("08. RM: removes a file\n");
    printf("    Command: rm file_name\n");
    printf("09. CP: copies the content of a file (first file in the command) to another file (second file in the command)\n");
    printf("    Command: cp source_file destination_file\n");
    printf("10. MKDIR: creates a new empty directory\n");
    printf("    Command: mkdir directory_name\n");
    printf("11. RMDIR: removes a directory\n");
    printf("    Command: rmdir directory_name\n");
    printf("12. ECHO: displays the string given as an argument\n");
    printf("    Command: echo string\n");
    printf("13. MV: moves a file to a specific destinatoin\n");
    printf("    Command: mv file_name destination\n");
    printf("14. DF: display current directory's system disk space usage\n");
    printf("    Command: df -m\n");
    printf("15. QUIT: exit\n");
    printf("    Command: quit\n");
}

/// Functie ce genereaza o noua linie in Shell

void printShellLine() {
    char *path = (char *)malloc(512 * charSize);
    getcwd(path, 500);
    printf("%s", path);
}

/// Functie ce citeste comanda introdusa

bool readInput(char *command, int file) {
    char *buff = (char *)malloc(strlen(command) * charSize);

    buff = readline("\n~ >> ");

    /// Daca nu avem o comanda, nu avem ce afisa

    if (strcmp(buff, "\n") == 0)
        return false;

    add_history(buff);

    strcpy(command, buff);
    strcpy(buff, "");

    int n = write(file, command, strlen(command));
    int m = write(file, "\n", sizeof(char));
    if( n < 0 || m < 0 ) {
        perror("Could not save the command in history");
        return errno;
    }

    return true;
}

/// Functie ce imparte comanda in cuvinte

int parseCommand (char *str, char arr[100][512]){
    char *token;
    int i=0;

    token = strtok(str, " ");

    while(token != NULL ) {
        strcpy(arr[i++], token);
        token = strtok(NULL, " ");
    }
    return i;
}

/// Functia CLEAR ce elibereaza Shell-ul

void clearCommand() {

    /// Pe Mac/Linux avem clear
    /// Pe Windows avem cls

    system("clear");
}

/// Functie ce gestioneaza suspendarea unui program (CTRL + C)

void ctrlchandler(){

    /// Setam pentru urmatorul semnal de intrerupere sa il gestioneze tot ctrlchandler

    signal(SIGINT, ctrlchandler);
    printf("\n");
}

/// Functie ce afiseaza istoricul comenzilor

void showHistory() {
    char buff[513];
    int n, index = 1;
    int file = open("shellHistory.txt", O_RDONLY);

    n = read(file, &buff, sizeof(buff));
    char *p = strtok(buff, "\n");
    while(p && strlen(p) > 0) {
        printf("%d %s\n", index++, p);
        p = strtok(NULL, "\n");
    }
}

/// Functia CD pentru schimbarea directorului

void cd(char* f){

    if (chdir(f))
    {
        perror("Could not change the directory");
        exit(0);
    }
}

/// Functie ce imparte comanda in cuvinte pentru a apela exec

void parseCommandForExec(char* str, char** command) {
    char *p = (char *)malloc(strlen(str) * charSize), *tok;
    int i = 1;
    strcpy(p, str);
    tok = strtok(p, " ");
    command[0] = tok;

    while(tok != NULL) {
        tok = strtok(NULL, " ");
        command[i++] = tok;
    }
}

/// Functie pentru tratarea cazurilor cu pipe (|)

void handlePipe(char *command) {

    /// 0 (in) este pentru citire, iar 1 (out) este pentru scriere

    int fd[2], oldPipe = STDIN_FILENO;
    pid_t pid, pidOriginal;

    /// Daca introduci 101 comenzi, iti dau 10 lei

    char *clearPipe[100];
    int n;

    for(n = 0; n < 100; n++) {
        clearPipe[n] = strsep(&command, "|");

        if(clearPipe[n] == NULL)
            break;
    }

    pidOriginal = fork();

    if(pidOriginal < 0) {

        perror("Something went wrong!");
        exit(0);

    } else if(pidOriginal == 0) {

        for(int i = 0; i < n - 1; i++) {

            // Mergem pana la n - 1 deoarece ultima comanda se va executa in parinte

            char **commandExec = (char **)malloc(1024 * 1024 * charSize);
            parseCommandForExec(clearPipe[i], commandExec);

            if(pipe(fd) < 0) {
                perror("Could not open the pipe!");
                exit(0);
            }

            pid = fork();

            if(pid < 0) {

                perror("Fork error");
                exit(0);

            } else if(pid == 0) {

                /// 'Rotim' pipe-ul vechi

                if(oldPipe != STDIN_FILENO) {
                    dup2(oldPipe, STDIN_FILENO);
                    close(oldPipe);
                }

                /// Acest proces va scrie in pipe-ul curent, nu necesita citire

                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);

                if(execvp(commandExec[0], commandExec) < 0) {
                    perror("Error runing one of the commands!");
                    exit(0);
                }

            } else {
                close(oldPipe);
                close(fd[1]);
                oldPipe = fd[0];
            }
        }

        for(int i = 0; i < n - 1; i++)
            wait(NULL);

        if(oldPipe != STDIN_FILENO) {
            dup2(oldPipe, STDIN_FILENO);
            close(oldPipe);
        }

        char **commandExec = (char **)malloc(1024 * 1024 * charSize);
        parseCommandForExec(clearPipe[n - 1], commandExec);

        if(execvp(commandExec[0], commandExec) < 0) {
            perror("Error runing the last command!");
            exit(0);
        }
    } else {

        wait(NULL);
    }
}

/// Functie pentru a verifica daca avem cazul unui pipe (|)

bool checkForPipe(char *command) {
    char *p = strstr(command, "|");
    return !(p == NULL);
}

/// Functie pentru a verifica daca avem cazul ||

bool checkForOr (char *command) {
    char *p = strstr(command, "||");
    return !(p == NULL);
}

/// Functie pentru a verifica daca avem cazul &&

bool checkForAnd (char *command) {
    char *p = strstr(command, "&&");
    return !(p == NULL);
}

/// Functie ce verifica daca sunt mai multe comenzi (;)

bool checkForSemicolon(char *command){
    char *p = strstr(command, ";");
    return !(p == NULL);
}

/// Functie magica ce utilizeaza execvp pentru a trata comenzi precum ls etc.

int unlimitedPower(char command[100][512], int dim) {

    char *nameThis[100];
    int j;
    bool isBackground = false;

    for(j = 0; j < dim; j++) {
        nameThis[j] = command[j];
    }

    if(strcmp(nameThis[j-1],"&")==0){
        nameThis[j-1] = NULL;
        isBackground=true;
    }
    else
        nameThis[j] = NULL;

    pid_t pid = fork();

    if(pid < 0) {
        perror("Fork error sir");
        exit(0);

    } else if(pid == 0) {
        if(execvp(nameThis[0], nameThis) < 0) {
            perror("Command not found");
            exit(0);
        }
    } else {

        /// Asteptam terminarea procesului copil doar daca acesta nu este un proces de background

        if(isBackground==false) {
            int status;
            waitpid(pid,&status,0);

            if (status != 0) {
                return 13;
            }
        }
    }
}

/// Functie pentru tratarea cazurilor cu ;

int handleSemicolon (char* command) {
    char *p = strtok(command, ";");
    char commands[100][256];
    int i=0, last;

    while (p){
        strcpy(commands[i++],p);
        p= strtok(NULL,";");
    }

    for(int j=0;j<i;j++){
        char parsed[100][512];
        int dim = 0;
        dim = parseCommand(commands[j], parsed);

        if (strcmp(parsed[0], "clear") == 0) {
            if (dim != 1){
                printf("Incorrect command! Check our manual -> MAN\n");
                manual();
            }
            else clearCommand();

        } else if(strcmp(parsed[0], "history") == 0) {
            if (dim != 1){
                printf("Incorrect command! Check our manual -> MAN\n");
                manual();
            }
            else showHistory();

        } else if (strcmp(parsed[0], "manual") == 0){
            if (dim != 1){
                printf("Incorrect command! Check our manual -> MAN\n");
                manual();
            }
            else manual();

        } else if (strcmp(parsed[0], "cd") == 0){
            if (dim != 2){
                printf("Incorrect command! Check our manual -> MAN\n");
                manual();
            }
            else cd(parsed[1]);

        } else if (strcmp(parsed[0], "quit") == 0){
            if (dim != 1){
                printf("Incorrect command! Check our manual -> MAN\n");
                manual();
            }
            else { printf("\n"); exit(0);}

        } else {
            last = unlimitedPower(parsed, dim);
        }
    }
    return last;
}

/// Functie pentru tratarea cazurilor cu ||

void handleOr(char *command){

    char *clearComm[100];

    int i;

    for(i=0; i<100; i++){
        clearComm[i] = strsep(&command, "||");
        if (clearComm[i] == NULL)
            break;
    }

    for (int j=0; j<i; j++) {
        if(checkForSemicolon(clearComm[j])){
            int val = handleSemicolon(clearComm[j]);
            if (val == 13) continue; else return;
        }
        else {

            char parsed[100][512];
            int dim = 0;
            dim = parseCommand(clearComm[j], parsed);

            if (dim) {
                if (strcmp(parsed[0], "clear") == 0) {
                    if (dim != 1) {
                        continue;
                    } else {
                        clearCommand();
                        return;
                    }

                } else if (strcmp(parsed[0], "history") == 0) {
                    if (dim != 1) {
                        continue;
                    } else {
                        showHistory();
                        return;
                    }

                } else if (strcmp(parsed[0], "manual") == 0) {
                    if (dim != 1) {
                        continue;
                    } else {
                        manual();
                        return;
                    }

                } else if (strcmp(parsed[0], "cd") == 0) {
                    if (dim != 2) {
                        continue;
                    } else {
                        cd(parsed[1]);
                        return;
                    }

                } else if (strcmp(parsed[0], "quit") == 0) {
                    if (dim != 1) {
                        continue;
                    } else {
                        printf("\n");
                        exit(0);
                        return;
                    }

                } else {
                    int val = unlimitedPower(parsed, dim);
                    if (val == 13) continue; else return;
                }
            }
        }
    }

}

/// Functie pentru tratarea cazurilor cu &&

void handleAnd(char *command){

    char *clearComm[100];

    int i;

    for(i=0; i<100; i++){
        clearComm[i] = strsep(&command, "&&");
        if (clearComm[i] == NULL)
            break;
    }

    for (int j=0; j<i; j++) {
        if(checkForSemicolon(clearComm[j])){
            int val = handleSemicolon(clearComm[j]);
            if (val == 13) return; else continue;
        }
        else {

            char parsed[100][512];
            int dim = 0;
            dim = parseCommand(clearComm[j], parsed);

            if (dim) {
                if (strcmp(parsed[0], "clear") == 0) {
                    if (dim != 1) {
                        return;
                    } else {
                        clearCommand();
                        continue;
                    }

                } else if (strcmp(parsed[0], "history") == 0) {
                    if (dim != 1) {
                        return;
                    } else {
                        showHistory();
                        continue;
                    }

                } else if (strcmp(parsed[0], "manual") == 0) {
                    if (dim != 1) {
                        return;
                    } else {
                        manual();
                        continue;
                    }

                } else if (strcmp(parsed[0], "cd") == 0) {
                    if (dim != 2) {
                        return;
                    } else {
                        cd(parsed[1]);
                        continue;
                    }

                } else if (strcmp(parsed[0], "quit") == 0) {
                    if (dim != 1) {
                        return;
                    } else {
                        printf("\n");
                        exit(0);
                        continue;
                    }

                } else {
                    int val = unlimitedPower(parsed, dim);
                    if (val == 13) return; else continue;
                }
            }
        }
    }

}

/// Functie pentru cazurile combinate && ||

void handleAndOr(char *command){

    /// In vector vom tine minte ce operator logic se afla inainte comenzii i

    /// logicalType[i] == true => inaintea comenzii i+1 se afla &&, altfel ||

    bool logicalType[200];

    char commands[200][256],aux[256]="";

    int current=0,i=0,j=0;

    while (current < strlen(command)){

        while(current < strlen(command) && command[current] != '&' && command[current] != '|'){
            aux[i++]=command[current++];
        }

        aux[i]='\0';

        logicalType[j] = command[current] == '&' ? true : false;

        strcpy(commands[j++],aux);
        i=0;
        ++current;

        if(command[current] == '&' || command[current] == '|')
            ++current;
    }

    /// nextCommand va decide ce comanda va fi executata in continuare, in functie de succesul executarii comenzii anterioare

    bool nextCommand;
    int val;

    for (int k = 0; k < j; ++k) {
        if(checkForSemicolon(commands[k])){
            val = handleSemicolon(commands[k]);
            nextCommand = val != 13 ? true : false;
        }
        else{
            char parsed[100][512];
            int dim = 0;
            dim = parseCommand(commands[k], parsed);

            if (dim) {
                if (strcmp(parsed[0], "clear") == 0) {
                    if (dim != 1) {
                        nextCommand=false;
                    } else {
                        clearCommand();
                        nextCommand=true;
                    }

                } else if (strcmp(parsed[0], "history") == 0) {
                    if (dim != 1) {
                        nextCommand=false;
                    } else {
                        showHistory();
                        nextCommand=true;
                    }

                } else if (strcmp(parsed[0], "manual") == 0) {
                    if (dim != 1) {
                        nextCommand=false;
                    } else {
                        manual();
                        nextCommand=true;
                    }

                } else if (strcmp(parsed[0], "cd") == 0) {
                    if (dim != 2) {
                        nextCommand=false;
                    } else {
                        cd(parsed[1]);
                        nextCommand=true;
                    }

                } else if (strcmp(parsed[0], "quit") == 0) {
                    if (dim != 1) {
                        nextCommand=false;
                    } else {
                        printf("\n");
                        exit(0);
                        nextCommand=true;
                    }

                } else {
                    val = unlimitedPower(parsed, dim);
                    nextCommand = val != 13 ? true : false;
                }
            }
        }

        while(k < j - 1 && nextCommand != logicalType[k]){
            ++k;
        }

    }
}


/// Functie ce decide ce comanda va fi executata la fiecare instructiune

void allCommands(char *command, int history)
{
    if (readInput(command, history)) {

        //Daca avem pipe, apelam functia ce trateaza acest caz, altfel verificam ce comanda avem

        if(checkForOr(command) && checkForAnd(command)){
            handleAndOr(command);

        } else if(checkForOr(command)) {
            handleOr(command);

        } else if(checkForPipe(command)) {
            handlePipe(command);

        } else if(checkForAnd(command)) {
            handleAnd(command);

        } else if (checkForSemicolon(command)) {
            handleSemicolon(command);

        } else {
            char parsed[100][512];
            int dim = 0;
            dim = parseCommand(command, parsed);

            if (strcmp(parsed[0], "clear") == 0) {
                if (dim != 1){
                    printf("Incorrect command! Check our manual -> MAN\n");
                    manual();
                }
                else clearCommand();

            } else if(strcmp(parsed[0], "history") == 0) {
                if (dim != 1){
                    printf("Incorrect command! Check our manual -> MAN\n");
                    manual();
                }
                else showHistory();

            } else if (strcmp(parsed[0], "manual") == 0){
                if (dim != 1){
                    printf("Incorrect command! Check our manual -> MAN\n");
                    manual();
                }
                else manual();

            } else if (strcmp(parsed[0], "cd") == 0){
                if (dim != 2){
                    printf("Incorrect command! Check our manual -> MAN\n");
                    manual();
                }
                else cd(parsed[1]);

            } else if (strcmp(parsed[0], "quit") == 0){
                if (dim != 1){
                    printf("Incorrect command! Check our manual -> MAN\n");
                    manual();
                }
                else { printf("\n"); exit(0);}

            } else {
                unlimitedPower(parsed, dim);
            }
        }
    }
}


/// Functie frumoasa

void welcome()
{
    printf("Welcome to Hedgehog Shell!\n\n");
    printf("  .|||||||||.          .|||||||||.\n");
    printf(" |||||||||||||        |||||||||||||\n");
    printf("|||||||||||' .\\      /. `|||||||||||\n");
    printf("`||||||||||_,__o    o__,_||||||||||'\n\n");
    sleep(2);
}

int main(int arg, char **argv) {

    signal(SIGINT, ctrlchandler);

    clearCommand();
    welcome();

    int history = open("shellHistory.txt", O_RDWR | O_CREAT, S_IRWXU);

    char *command = (char *)malloc(512 * charSize);

    while (true) {
        printShellLine();
        allCommands(command, history);
    }

    return 0;
}
