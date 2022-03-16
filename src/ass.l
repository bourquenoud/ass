%option yylineno

%{
    #include "ass.tab.h"
    #include "../macro.h"
    #include "../xmalloc.h"
    #include "../ast_node.h"

    #include <unistd.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdbool.h>
    #include <stdint.h>
    #include <string.h>

    #define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;

    extern int yylex();
    extern int yyparse();
    extern FILE* yyin;

    void yyerror(const char* s);

    static data_t* get_string(char*);
    static data_t* get_id(char*);
    static data_t* get_code(char* in);
    static data_t* get_const(char*);
    static data_t* get_bits(char*);
    static data_t* get_int(char*);

    static void xmalloc_callback(int err);
%}

/* Others */
L_LINE                  [\r\n]
L_WHITESPACE            [" "\t]
L_C_BLOCK               "%{"([ -$&-~\t\r\n]|%[^}])*"%}"

/* Comments */
L_S_COMMENT             "//".*
L_M_COMMENT             "/*"([^*]*|(\*+[^/]))*"*/"
L_DS_COMMENT             "///".*
L_DM_COMMENT             "/**"([^*]*|(\*+[^/]))*"*/"

/* Commands */
L_PARAM                 "%param"
L_CONSTANT              "%constant"
L_ENUM                  "%enum"
L_PATTERN               "%pattern"
L_FORMAT                "%format"
L_ORDER                 "%order"
L_OPCODE                "%opcode"
L_CODE                  "%code"
L_OUTPUT                "%output"
L_OVERRIDE              "%override"
L_UNKNOWN_CMD           %[a-zA-Z0-9_]*

/* Constant */
L_INTEGER               -?[0-9]+
L_BIT_CONSTANT          "$"[a-z][a-z0-9_]*
L_BIT_LIT               "$"[0-9a-fA-F]*(h|o|b)?
L_STRING                \".*?\"

/* Identifier */
L_IDENTIFIER            [a-z][a-z0-9_]*

/* Substitution */
L_SUBST                 [A-Z_]+

/* Punctuation */
L_LEFTPAR               "("
L_RIGHPAR               ")"
L_LEFTSQBRACK           "["
L_RIGHSQBRACK           "]"
L_ELIPSIS               "..."
L_COMMA                 ","
L_PLUS                  "+"
L_MINUS                 "-"
L_MULTIPLY              "*"
L_DIVIDE                "/"
L_MODULO                "%"

%%

{L_SUBST}                                       { yylval.dVal = get_id(yytext); return T_SUBST; }

{L_PARAM}                                       { return T_PARAM; }
{L_CONSTANT}                                    { return T_CONSTANT; }
{L_ENUM}                                        { return T_ENUM; }
{L_PATTERN}                                     { return T_PATTERN; }
{L_FORMAT}                                      { return T_FORMAT; }
{L_ORDER}                                       { return T_ORDER; }
{L_OPCODE}                                      { return T_OPCODE; }
{L_CODE}                                        { return T_CODE; }
{L_OUTPUT}                                      { return T_OUTPUT; }
{L_OVERRIDE}                                    { return T_OVERRIDE; }
{L_UNKNOWN_CMD}                                 { return T_UNKNOWN_CMD; }
{L_LEFTPAR}                                     { return T_LEFTPAR; }
{L_RIGHPAR}                                     { return T_RIGHPAR; }
{L_LEFTSQBRACK}                                 { return T_LEFTSQBRACK; }
{L_RIGHSQBRACK}                                 { return T_RIGHSQBRACK; }
{L_ELIPSIS}                                     { return T_ELIPSIS; }
{L_COMMA}                                       { return T_COMMA; }
{L_PLUS}                                        { return T_PLUS; }
{L_MINUS}                                       { return T_MINUS; }
{L_MULTIPLY}                                    { return T_MULTIPLY; }
{L_DIVIDE}                                      { return T_DIVIDE; }
{L_MODULO}                                      { return T_MODULO; }

{L_INTEGER}                                     { yylval.dVal = get_int(yytext); return T_INTEGER; }
{L_BIT_LIT}                                     { yylval.dVal = get_bits(yytext); return T_BIT_LIT; }
{L_BIT_CONSTANT}                                { yylval.dVal = get_const(yytext); return T_BIT_CONSTANT; }
{L_STRING}                                      { yylval.dVal = get_string(yytext); return T_STRING; }

{L_DS_COMMENT}|{L_DM_COMMENT}                   { return T_DS_COMMENT;}
{L_S_COMMENT}|{L_M_COMMENT}                     { return T_S_COMMENT;}

{L_IDENTIFIER}                                  { yylval.dVal = get_id(yytext); return T_IDENTIFIER; }

{L_C_BLOCK}                                     { yylval.dVal = get_code(yytext); return T_C_BLOCK; }
{L_LINE}                                        { return T_LINE; }
{L_WHITESPACE}                                  {/* Ignore white spaces */}

.                                               { fprintf(stderr, "\033[31mERROR line %i : syntax error, unexpected '%s'\033[0m\n", yylineno, yytext); exit(EXIT_FAILURE);}

%%

data_t* get_string(char* in) //Return a string
{
    data_t* data;

    char* str;
    int len = strlen(in);

    xmalloc_set_handler(xmalloc_callback);
    str = xmalloc(len - 1); //FIXME: <---THIS IS A DATA LEAK, STR IS NEVER FREED
    strncpy(str, in + 1, len - 1); //Remove the leading and trailing "
    str[len - 2] = '\0'; //Null terminate the string
    data = xmalloc(sizeof(data_t));
    data->strVal = str;
    return data;
}

data_t* get_id(char* in) //Return a string
{
    data_t* data;

    char* str;
    int len = strlen(in);

    xmalloc_set_handler(xmalloc_callback);
    str = xmalloc(len + 1); //FIXME: <---THIS IS A DATA LEAK, STR IS NEVER FREED
    strncpy(str, in, len + 1); //Keep everything
    data = xmalloc(sizeof(data_t));
    data->strVal = str;
    return data;
}

data_t* get_code(char* in) //Return a string
{
    data_t* data;

    char* str;
    int len = strlen(in);

    xmalloc_set_handler(xmalloc_callback);
    str = xmalloc(len - 3); //FIXME: <---THIS IS A DATA LEAK, STR IS NEVER FREED
    strncpy(str, in + 2, len - 3); //Remove the %{ and %}
    str[len - 4] = '\0'; //Null terminate the string
    data = xmalloc(sizeof(data_t));
    data->strVal = str;
    return data;
}

data_t* get_const(char* in) //Return a string
{
    data_t* data;

    char* str;
    int len = strlen(in);

    xmalloc_set_handler(xmalloc_callback);
    str = xmalloc(len); //FIXME: <---THIS IS A DATA LEAK, STR IS NEVER FREED
    strncpy(str, in + 1, len + 1); //Remove the leading $
    data = xmalloc(sizeof(data_t));
    data->strVal = str;
    return data;
}

data_t* get_bits(char* in) //Return an unsigned int64
{
    data_t* data;

    int len = strlen(in);
    char lastChar = in[len-1];

    xmalloc_set_handler(xmalloc_callback);
    data = xmalloc(sizeof(data_t));
    data->bVal.width = 0;

    if( '0' == lastChar || '1' == lastChar)
    {
        data->bVal.val = strtoull(in + 1, NULL, 2);
        data->bVal.width = len - 1; //Example $110101, length of 7 and 6 bits
    }
    else if('b' == lastChar)
    {
        data->bVal.val = strtoull(in + 1, NULL, 2);
        data->bVal.width = len - 2; //Example $110101b, length of 8 and 6 bits
    }
    else if('o' == lastChar)
    {
        data->bVal.val = strtoull(in + 1, NULL, 8); 
        data->bVal.width = (len - 2)*3; //Example $054o, length of 5 and 9 bits
    }
    else if('h' == lastChar)
    {
        data->bVal.val = strtoull(in + 1, NULL, 16);
        data->bVal.width = (len - 2)*4; //Example $054Fh, length of 6 and 16 bits
    }
    
    return data;
}

data_t* get_int(char* in) //Return an signed int
{
    data_t* data;

    xmalloc_set_handler(xmalloc_callback);
    data = xmalloc(sizeof(data_t));

    data->iVal = strtoll(in, NULL, 10);

    return data;
}

void xmalloc_callback(int err)
{
    fputs("\033[31mError in " STR(__FILE__) " : ", stderr);
    if(0 == err)
        fputs("Cannot allocate zero length memory\033[0m\n", stderr);
    else if(1 == err)
        fputs("Malloc returned a NULL pointer\033[0m\n", stderr);
    else
        fputs("Unknown errro\033[0m\n", stderr);
}