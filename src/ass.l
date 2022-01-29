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
    static data_t* get_const(char*);
    static data_t* get_bits(char*);
    static data_t* get_int(char*);

    static void xmalloc_callback(int err);
%}

/* Others */
L_LINE                  [\r\n]
L_WHITESPACE            [" "\t]

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

%%

{L_SUBST}                                       { printf("T_SUBST : %s\n", yytext); return T_SUBST; }

{L_PARAM}                                       { printf("T_PARAM : %s\n", yytext); return T_PARAM; }
{L_CONSTANT}                                    { printf("T_CONSTANT : %s\n", yytext); return T_CONSTANT; }
{L_ENUM}                                        { printf("T_ENUM : %s\n", yytext); return T_ENUM; }
{L_PATTERN}                                     { printf("T_PATTERN : %s\n", yytext); return T_PATTERN; }
{L_FORMAT}                                      { printf("T_FORMAT : %s\n", yytext); return T_FORMAT; }
{L_ORDER}                                       { printf("T_ORDER : %s\n", yytext); return T_ORDER; }
{L_OPCODE}                                      { printf("T_OPCODE : %s\n", yytext); return T_OPCODE; }
{L_UNKNOWN_CMD}                                 { printf("T_UNKNOWN_CMD : %s\n", yytext); return T_UNKNOWN_CMD; }

{L_LEFTPAR}                                     { printf("T_LEFTPAR : %s\n", yytext); return T_LEFTPAR; }
{L_RIGHPAR}                                     { printf("T_RIGHPAR : %s\n", yytext); return T_RIGHPAR; }
{L_LEFTSQBRACK}                                 { printf("T_LEFTSQBRACK : %s\n", yytext); return T_LEFTSQBRACK; }
{L_RIGHSQBRACK}                                 { printf("T_RIGHSQBRACK : %s\n", yytext); return T_RIGHSQBRACK; }
{L_ELIPSIS}                                     { printf("T_ELIPSIS : %s\n", yytext); return T_ELIPSIS; }
{L_COMMA}                                       { printf("T_COMMA : %s\n", yytext); return T_COMMA; }

{L_INTEGER}                                     { printf("T_INTEGER : %s\n", yytext); yylval.dVal = get_int(yytext); return T_INTEGER; }
{L_BIT_LIT}                                     { printf("T_BIT_LIT : %s\n", yytext); yylval.dVal = get_bits(yytext); return T_BIT_LIT; }
{L_BIT_CONSTANT}                                { printf("T_BIT_CONSTANT : %s\n", yytext); yylval.dVal = get_const(yytext); return T_BIT_CONSTANT; }
{L_STRING}                                      { printf("T_STRING : %s\n", yytext); yylval.dVal = get_string(yytext); return T_STRING; }

{L_DS_COMMENT}|{L_DM_COMMENT}                   { printf("T_DS_COMMENT : %s\n", yytext); return T_DS_COMMENT;}
{L_S_COMMENT}|{L_M_COMMENT}                     { printf("T_S_COMMENT : %s\n", yytext); return T_S_COMMENT;}

{L_IDENTIFIER}                                  { printf("T_IDENTIFIER : %s\n", yytext); yylval.dVal = get_id(yytext); return T_IDENTIFIER; }

{L_LINE}                                        { return T_LINE; }
{L_WHITESPACE}                                  {/* Ignore blank spaces */}

.                                               {printf("WARNING : uknown character %s\n", yytext);}

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
    fputs("Error in " STR_EXPAND(__FILE__) " : ", stderr);
    if(0 == err)
        fputs("Cannot allocate zero length memory\n", stderr);
    else if(1 == err)
        fputs("Malloc returned a NULL pointer\n", stderr);
    else
        fputs("Unknown errro\n", stderr);
}