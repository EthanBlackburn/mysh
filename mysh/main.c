//
//  main.c
//  mysh
//
//  Created by Ethan Blackburn on 10/5/14.
//  Copyright (c) 2014 Ethan Blackburn. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_ARGS 15
#define MAX_COMMANDS 50
#define MAX_CHARS 256
#define DELIMETERS " \t\n"

int run = 1;

void exitHandler(int sig) {
    signal(SIGINT, SIG_DFL);
    exit(sig);
}

int main(int argc, const char * argv[])
{
    
    char inputCommand[MAX_CHARS];
    char * commands[MAX_COMMANDS];
    char ** commandList;
    char * curCommand[MAX_ARGS];
    char ** curArgs;
    char * prompt = "λ> ";
    
    while(1){
        
        signal(SIGINT, exitHandler);
        
        while(!feof(stdin)){
            fputs(prompt, stdout);
            fgets(inputCommand, MAX_CHARS, stdin);
            
            if(inputCommand != NULL){
                commandList = commands;
                *commandList++ = strtok(inputCommand, "|");
                
                while((*commandList++ = strtok(NULL, "|")));
                
                for (int i = 0; i < sizeof(commands)/sizeof(commands[0]); i++){
                    if (!commands[i]){
                        break;
                    }
                    curArgs = curCommand;
                    *curArgs++ = strtok(commands[i], DELIMETERS);
                    
                    while((*curArgs++ = strtok(NULL, DELIMETERS)));
                    
                    if (*curCommand && !strcmp(curCommand[0], "cd")){
                        char cwd[1024];
                        getcwd(cwd, sizeof(cwd));
                        if (cwd != NULL){
                            if (curCommand[1]){
                                chdir(curCommand[1]);
                            }
                            system("ls");
                        }else{
                            perror("could not change directories.");
                        }
                    }
                    
                    else if (*curCommand && !strcmp(curCommand[0], "exit"))
                        exit(0);
                    
                    else{
                        int parallelCommands[15];
                        char *parCommands[15][15];
                        int pc = 0;
                        int k = 0;
                        int j = 0;
                        memset(parCommands, 0, sizeof parCommands);

                        while(curCommand[k]){
                            if(!strcmp(curCommand[k], "&&")){
                                parallelCommands[pc++] = k; //to parse our commands
                                j = 0;
                            }
                            else{
                                parCommands[pc][j] = curCommand[k];
                                j++;
                            }
                            k++;
                        }
                        int c = 0;
                        int status;
                        if(pc > 0){
                            while(parallelCommands[c]){
                                pid_t pid = fork();
                                if (pid == 0){
                                    execvp(parCommands[c][0], parCommands[c]);
                                    exit(3);
                                }
                                else if (pid < 0){
                                    printf("fork failed");
                                    status = -1;
                                }
                                else{
                                    if (waitpid (pid, &status, 0) != pid){
                                        status = -1;
                                    }
                                }
                                c++;
                            }
                        }
                        //execute last command
                        pid_t pid = fork();
                        if (pid == 0){
                            execvp(parCommands[c][0], parCommands[c]);
                            exit(3);
                        }
                        else if (pid < 0){
                            printf("fork failed");
                            status = -1;
                        }
                        else{
                            if (waitpid (pid, &status, 0) != pid){
                                status = -1;
                            }
                        }
                    }
                }
            }
        }
    }
}

