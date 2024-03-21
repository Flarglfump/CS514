/*
Author: Gavin Witsken
Program: ish - interactive shell
File: ish.c
*/

/* All other external includes are included from ish.h */
#include "ish.h"
#include "parse.h"

int main() {
    static char ishInputBuf[ISH_INPUT_BUF_SIZE];
    
    while(ish_getcmd(ishInputBuf, sizeof(ishInputBuf)) >= 0){
        if(ishInputBuf[0] == 'c' && ishInputBuf[1] == 'd' && ishInputBuf[2] == ' '){
            // Chdir must be called by the parent, not the child.
            ishInputBuf[strlen(ishInputBuf) - 1] = '\0';  // chop \n
            if(chdir(ishInputBuf+3) < 0) {
                dprintf(STDERR_FILENO, "cannot cd %s\n", ishInputBuf+3);
            }         
            continue;
        }
        if(ish_fork_assert() == 0) {
            ish_runcmd(ish_parsecmd(ishInputBuf));
        }
      
        wait(NULL);
    } // end while

    return 0;
}

void ish_panic(const char * msg) {
    dprintf(STDERR_FILENO, "%s\n", msg);
    exit(EXIT_FAILURE);
}

int ish_fork_assert(void) {
    pid_t pid = fork();
    if (pid == -1) {
        ish_panic("ish_fork");
    }
    return pid;
}

void ish_runcmd(ish_cmd_t * cmd) {
    int pipe_fd[2];
    ish_backcmd_t * bcmd;
    ish_execcmd_t * ecmd;
    ish_listcmd_t * lcmd;
    ish_pipecmd_t * pcmd;
    ish_redircmd_t * rcmd;

    if(cmd == ISH_TOKEN_TYPE_NONE)
        exit(EXIT_SUCCESS);

    switch(cmd->type){
    default: // Catches any other unspecified case
        ish_panic("ish_runcmd");
    break;

    case ISH_TOKEN_TYPE_EXEC:
        ecmd = (ish_execcmd_t *) cmd;

        if(ecmd->argv[0] == NULL) {
            exit(EXIT_FAILURE);
        }

        ish_exec(ecmd->argv[0], ecmd->argv);
        dprintf(STDERR_FILENO, "ish_exec %s failed!\n", ecmd->argv[0]);
    break;

    case ISH_TOKEN_TYPE_REDIR:
        rcmd = (ish_redircmd_t *) cmd;

        close(rcmd->fd1);
        if (rcmd->fd2 != -1) {
            close(rcmd->fd2);
        }

        if(open(rcmd->file, rcmd->mode) < 0) {  
            dprintf(STDERR_FILENO, "open %s failed\n", rcmd->file);
            exit(EXIT_FAILURE);
        }

        ish_runcmd(rcmd->cmd);
    break;

    case ISH_TOKEN_TYPE_LIST:
        lcmd = (ish_listcmd_t *) cmd;

        if(ish_fork_assert() == 0) {
            ish_runcmd(lcmd->left);
        }

        wait(NULL);
        ish_runcmd(lcmd->right);
    break;

    case ISH_TOKEN_TYPE_PIPE:
        pcmd = (ish_pipecmd_t *) cmd;

        if(pipe(pipe_fd) < 0) {
            ish_panic("pipe");
        }
        
        if(ish_fork_assert() == 0) {
            close(STDOUT_FILENO);
            dup(pipe_fd[1]);
            close(pipe_fd[0]);
            close(pipe_fd[1]);

            ish_runcmd(pcmd->left);
        }

        if(ish_fork_assert() == 0) {
            close(STDIN_FILENO);
            dup(pipe_fd[0]);
            close(pipe_fd[0]);
            close(pipe_fd[1]);

            ish_runcmd(pcmd->right);
        }

        close(pipe_fd[0]);
        close(pipe_fd[1]);

        wait(NULL);
        wait(NULL);
    break;

    case ISH_TOKEN_TYPE_BACK:
        bcmd = (ish_backcmd_t *) cmd;

        if(ish_fork_assert() == 0) {
            ish_runcmd(bcmd->cmd);
        }
    break;
  } // end switch

  exit(EXIT_SUCCESS); // Terminate process upon reaching this point
}

int ish_exec(const char * cmd, char * const * argv) {
    extern char ** environ;

    return (execve(cmd, argv, environ));
}

ish_cmd_t * ish_execcmd_create(void) {
    ish_execcmd_t * cmd;

    cmd = malloc(sizeof(*cmd));
    memset(cmd, 0, sizeof(*cmd));

    cmd->type = ISH_TOKEN_TYPE_EXEC;

    return (ish_cmd_t *) cmd;
}

ish_cmd_t * ish_redircmd_create(ish_cmd_t * subcmd, char * file, char * efile, int mode, int fd1, int fd2) {
    ish_redircmd_t * cmd;

    cmd = malloc(sizeof(*cmd));
    memset(cmd, 0, sizeof(*cmd));

    cmd->type = ISH_TOKEN_TYPE_REDIR;
    cmd->cmd = subcmd;
    cmd->file = file;
    cmd->efile = efile;
    cmd->mode = mode;
    cmd->fd1 = fd1;
    cmd->fd2 = fd2;

    return (ish_cmd_t *) cmd;
}

ish_cmd_t * ish_pipecmd_create(ish_cmd_t * left, ish_cmd_t * right) {
    ish_pipecmd_t * cmd;

    cmd = malloc(sizeof(*cmd));
    memset(cmd, 0, sizeof(*cmd));

    cmd->type = ISH_TOKEN_TYPE_PIPE;
    cmd->left = left;
    cmd->right = right;

    return (ish_cmd_t *) cmd;
}

ish_cmd_t * ish_listcmd_create(ish_cmd_t * left, ish_cmd_t * right) {
  ish_listcmd_t * cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));

  cmd->type = ISH_TOKEN_TYPE_LIST;
  cmd->left = left;
  cmd->right = right;

  return (ish_cmd_t *) cmd;
}

ish_cmd_t * ish_backcmd_create(ish_cmd_t * subcmd) {
  ish_backcmd_t * cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));

  cmd->type = ISH_TOKEN_TYPE_BACK;
  cmd->cmd = subcmd;

  return (ish_cmd_t *) cmd;
}

int ish_getcmd(char * buf, int nbuf) {
    dprintf(STDOUT_FILENO, "$ ");
    memset(buf, 0, nbuf);
    
    char * res = fgets(buf, nbuf, stdin);

    if(res == NULL || buf[0] == '\0') {
        return -1;
    }
    
    return 0;
}