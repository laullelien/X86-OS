#include "debug.h"
#include "string.h"
#include "syscall.h"
#include "stdlib.h"


int main(void) {
    while (1) {
        char input[16];
        cons_write("\n> ", 4);
        int size = cons_read(input, 15);
        input[size] = 0;

        char command[16];
        char param[16];
        command[0] = 0;
        param[0] = 0;
        int is_param = 0;
        int idx_command = 0;
        int idx_param = 0;
        for (int i=0;i<size;i++) {
            if (input[i] == ' ') {
                if (is_param == 0) {
                    is_param = 1;
                    continue;
                } else {
                    break;
                }
            }
            if (is_param) {
                param[idx_param] = input[i];
                idx_param += 1;
            } else {
                command[idx_command] = input[i];
                idx_command += 1;
            }
        }
        command[idx_command] = 0;
        param[idx_param] = 0;

        printf("Executing command '%s' with param '%s'\n", command, param);


        if (strncmp(command, "exit", 5) == 0) {
            printf("Exiting, Bye!\n");
            kill(getpid());
        } else if (strncmp(command, "echo", 5) == 0) {
            if (strncmp(param, "on", 3) == 0) {
                cons_echo(1);
                printf("Echo is now on\n");
            } else if (strncmp(param, "off", 4) == 0) {
                cons_echo(0);
                printf("Echo is now off\n");
            } else {
                printf("Usage: 'echo on' or 'echo off'\n");
            }
        } else if (strncmp(command, "start", 6) == 0) {
            printf("Trying to run program '%s'\n", param);
            int pid = start(param, 0, 128, 0);
            printf("\n'%s' started with pid '%i'\n", param, pid);

        } else if (strncmp(command, "kill", 5) == 0) {
            
            int pid = strtol(param, 0, 10);
            printf("\nprocess pid '%i' killed with return code '%i'\n", pid, kill(pid));
        
        } else if (strncmp(command, "waitpid", 8) == 0) {
            
            int pid = strtol(param, 0, 10);
            printf("Waiting for '%i'\n", pid);
            printf("\nwaitpid(%i) returned '%i'\n", pid, waitpid(pid, 0));
            
        } else if (strncmp(command, "chprio", 7) == 0) {
            
            int prio = strtol(param, 0, 10);
            printf("\nChanging priority to %i, return code '%i'\n", prio, chprio(getpid(), prio));

        } else if (strncmp(command, "autotest", 9) == 0) {

            printf("\nStarting autotest\n\n");
            int pid = start("autotest", 0, 128, 0);
            printf("\nAutotest ended with status %i\n\n", waitpid(pid, 0));
            
        
        } else if (strncmp(command, "ps", 3) == 0) {
            char ps[80*40];
            printf("\nShowing ps\n");
            sys_info(ps);
            printf("%s", ps);
            
        } else {
            printf("Unknown command, command list:\n");
            printf("\t-exit\n");
            printf("\t-echo on/off\n");
            printf("\t-start <process_name>\n");
            printf("\t-kill <pid>\n");
            printf("\t-waitpid <pid>\n");
            printf("\t-chprio <prio>\n");
            printf("\t-autotest\n");
            printf("\t-ps\n");
        }



    }
    
    return 0;
}
