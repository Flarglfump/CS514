/*
Author: Gavin Witsken
Program: ish - interactive shell
File: parse.c
*/

#include "parse.h"

char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>&;()";

ish_cmd_t * ish_parsecmd(char * s) {
    char * es;
    ish_cmd_t * cmd;

    es = s + strlen(s);
    cmd = ish_parseline(&s, es);
    ish_peek(&s, es, "");

    if(s != es){
        dprintf(STDERR_FILENO, "leftovers: %s\n", s);
        ish_panic("syntax");
    }

    ish_nulterminate(cmd);

    return cmd;
}

ish_cmd_t * ish_parseline(char ** ps, char * es) {
    ish_cmd_t * cmd;

    cmd = ish_parsepipe(ps, es);

    while(ish_peek(ps, es, "&")){
        ish_gettoken(ps, es, NULL, NULL);
        cmd = ish_backcmd_create(cmd);
    }

    if(ish_peek(ps, es, ";")){
        ish_gettoken(ps, es, NULL, NULL);
        cmd = ish_listcmd_create(cmd, ish_parseline(ps, es));
    }

    return cmd;
}

ish_cmd_t * ish_parsepipe(char ** ps, char * es) {
    ish_cmd_t *cmd;

    cmd = ish_parseexec(ps, es);

    if(ish_peek(ps, es, "|")){
        ish_gettoken(ps, es, 0, 0);
        cmd = ish_pipecmd_create(cmd, ish_parsepipe(ps, es));
    }

    return cmd;
}

ish_cmd_t * ish_parseexec(char ** ps, char * es) {
    char * q, * eq;
    int tok, argc;
    ish_execcmd_t * cmd;
    ish_cmd_t * ret;

    if(ish_peek(ps, es, "(")) {
        return ish_parseblock(ps, es);
    }

    ret = ish_execcmd_create();
    cmd = (ish_execcmd_t *) ret;

    argc = 0;
    ret = ish_parseredirs(ret, ps, es);
    while(!ish_peek(ps, es, "|)&;")){
        if((tok = ish_gettoken(ps, es, &q, &eq)) == 0) {
            break;
        }
        if(tok != 'a') {
            ish_panic("syntax");
        }

        cmd->argv[argc] = q;
        cmd->eargv[argc] = eq;

        argc++;

        if(argc >= ISH_MAX_ARG_COUNT) {
            ish_panic("too many args");
        }
            
        ret = ish_parseredirs(ret, ps, es);
    }

    cmd->argv[argc] = 0;
    cmd->eargv[argc] = 0;

    return ret;
}

ish_cmd_t * ish_parseredirs(ish_cmd_t * cmd, char ** ps, char * es) {
    int tok;
    char * q, * eq;

    while(ish_peek(ps, es, "<>")){
        tok = ish_gettoken(ps, es, NULL, NULL);
        if(ish_gettoken(ps, es, &q, &eq) != 'a') {
            ish_panic("missing file for redirection");
        }
        switch(tok) {
        case '<':
            cmd = ish_redircmd_create(cmd, q, eq, O_RDONLY, STDIN_FILENO, -1);
        break;
        case '>':
            cmd = ish_redircmd_create(cmd, q, eq, O_WRONLY | O_CREAT | O_TRUNC, STDOUT_FILENO, -1);
        break;
        case '&':  // >&
            cmd = ish_redircmd_create(cmd, q, eq, O_WRONLY | O_CREAT | O_TRUNC, STDOUT_FILENO, STDERR_FILENO);
        case '+':  // >>
            cmd = ish_redircmd_create(cmd, q, eq, O_WRONLY | O_CREAT, STDOUT_FILENO, -1);
        break;
        case '-':  // >>&
            cmd = ish_redircmd_create(cmd, q, eq, O_WRONLY | O_CREAT, STDOUT_FILENO, STDERR_FILENO);
        break;
        }
    }
    
    return cmd;
}

ish_cmd_t * ish_parseblock(char ** ps, char * es) {
    ish_cmd_t * cmd;

    if(!ish_peek(ps, es, "(")) {
        ish_panic("parseblock");
    }

    ish_gettoken(ps, es, NULL, NULL);

    cmd = ish_parseline(ps, es);

    if(!ish_peek(ps, es, ")")) {
        ish_panic("syntax - missing )");
    }
        
    ish_gettoken(ps, es, NULL, NULL);

    cmd = ish_parseredirs(cmd, ps, es);
    
    return cmd;
}

int ish_gettoken(char ** ps, char * es, char ** q, char ** eq) {
    char *s;
    int ret;

    s = *ps;
    while(s < es && strchr(whitespace, *s)) { // Skip starting whitespace
        s++;
    }

    if(q) {
        *q = s;
    }

    ret = *s;

    switch(*s) {
    case '\0':
    break;

    case '\n': // If newline, replace with semicolon
        *s = ';';
    case '(':
    case ')':
    case ';':
    case '&':
    case '|':
    case '<':
        s++;
    break;
    
    case '>':
        s++;
        if(*s == '>') {
            ret = '+'; // >>
            s++;
            if (*s == '&') {
                ret = '-'; // >>&
                s++;
            }
        } else if (*s == '&') {
            ret = '&'; // >&
            s++;
        }
    break;
    default:
        ret = 'a';
        while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s)) { // skip through whitespace
            s++;
        }
    break;
    } // end switch

    if(eq) {
        *eq = s;
    }

    while(s < es && strchr(whitespace, *s)) { // skip through whitespace
        s++;
    }
    *ps = s;
    return ret;
}

int ish_peek(char ** ps, char * es, char * toks) {
    char * s;

    s = *ps;
    while(s < es && strchr(whitespace, *s)) {
        s++;
    }
        
    *ps = s;

    return *s && strchr(toks, *s);
}

ish_cmd_t * ish_nulterminate(ish_cmd_t * cmd) {
    int i;
    ish_backcmd_t * bcmd;
    ish_execcmd_t * ecmd;
    ish_listcmd_t * lcmd;
    ish_pipecmd_t * pcmd;
    ish_redircmd_t * rcmd;

    if(cmd == NULL) {
        return 0;
    }

    switch(cmd->type){
    default:
    break;

    case ISH_TOKEN_TYPE_EXEC:
        ecmd = (ish_execcmd_t *) cmd;
        for(i = 0; ecmd->argv[i]; i++) {
            *(ecmd->eargv[i]) = '\0';
        }
    break;

    case ISH_TOKEN_TYPE_REDIR:
        rcmd = (ish_redircmd_t *) cmd;
        ish_nulterminate(rcmd->cmd);
        *(rcmd->efile) = '\0';
    break;

    case ISH_TOKEN_TYPE_PIPE:
        pcmd = (ish_pipecmd_t *) cmd;
        ish_nulterminate(pcmd->left);
        ish_nulterminate(pcmd->right);
    break;

    case ISH_TOKEN_TYPE_LIST:
        lcmd = (ish_listcmd_t *) cmd;
        ish_nulterminate(lcmd->left);
        ish_nulterminate(lcmd->right);
    break;

    case ISH_TOKEN_TYPE_BACK:
        bcmd = (ish_backcmd_t *) cmd;
        ish_nulterminate(bcmd->cmd);
    break;
    }

    return cmd;
}