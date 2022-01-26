%debug
%define api.token.raw
%define parse.error detailed

%code requires {
    #include <unistd.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdbool.h>
    #include <stdint.h>

    #include "../ast_node.h"

    int build_ast(int argc, char** argv);
    const char *getTypeName(int type);
}

%code top{
    #include <unistd.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdbool.h>
    #include <stdint.h>

    extern int yylex();
    extern int yyparse();
    extern FILE* yyin;

    void yyerror(const char* s);
}

%union {
    char*       strVal;
	int64_t     iVal;
	uint64_t    uVal;
	double      dVal;
    node_t*     nVal;
}

%code{
    //NULL until tree is completely built
    node_t* topNode = NULL;
}

/********************************************************************************************************/
/*                                              BEGIN TOKENS                                            */
/********************************************************************************************************/

/* Others */
%token T_LINE
%token T_WHITESPACE

/* Comments */
%token T_S_COMMENT
%token T_M_COMMENT
%token T_DS_COMMENT
%token T_DM_COMMENT

/* Commands */
%token T_PARAM
%token T_CONSTANT
%token T_ENUM
%token T_PATTERN
%token T_ORDER
%token T_OPCODE
%token T_FORMAT
%token T_UNKNOWN_CMD

/* Constant */
%token<iVal> T_INTEGER
%token<strVal> T_BIT_CONSTANT
%token<uVal> T_BIT_LIT
%token<strVal> T_STRING

/* Identifier */
%token<strVal> T_IDENTIFIER

/* Substitution */
%token<strVal> T_SUBST

/* Punctuation */
%token T_LEFTPAR             
%token T_RIGHPAR             
%token T_LEFTSQBRACK         
%token T_RIGHSQBRACK         
%token T_ELIPSIS             
%token T_COMMA  

%start page


/* Types */
%type<nVal> param_args
%type<nVal> param
%type<nVal> constant
%type<nVal> enum
%type<nVal> pattern
%type<nVal> order_args
%type<nVal> order
%type<nVal> opcode
%type<nVal> expr
%type<nVal> subst
%type<nVal> format
%type<nVal> bit_pattern
%type<nVal> bit_pattern_args
%type<nVal> bit_elem
%type<nVal> command
%type<nVal> page

%%

endline:              T_M_COMMENT
                    | T_S_COMMENT T_LINE
                    | T_S_COMMENT YYEOF
                    | T_DM_COMMENT
                    | T_DS_COMMENT T_LINE
                    | T_DS_COMMENT YYEOF
                    | T_LINE
                    | YYEOF

param_args:           T_IDENTIFIER                              { $$ = node_init(T_IDENTIFIER,   (data_t){.strVal=$1}, NULL, NULL, NULL); }
                    | param_args T_IDENTIFIER                   { $$ = node_init(T_IDENTIFIER,   (data_t){.strVal=$2}, $1,   NULL, NULL); }
                    | param_args T_INTEGER                      { $$ = node_init(T_INTEGER,      (data_t){.iVal=$2},   $1,   NULL, NULL); }
                    | param_args T_STRING                       { $$ = node_init(T_STRING,       (data_t){.strVal=$2}, $1,   NULL, NULL); }
                    | param_args T_BIT_LIT                      { $$ = node_init(T_BIT_LIT,      (data_t){.iVal=$2},   $1,   NULL, NULL); }
                    | param_args T_BIT_CONSTANT                 { $$ = node_init(T_BIT_CONSTANT, (data_t){.strVal=$2}, $1,   NULL, NULL); }
;

param:                T_PARAM param_args endline                { $$ = node_init(T_PARAM, (data_t){.iVal=0}, $2, NULL, NULL); }
;

constant:             T_CONSTANT T_IDENTIFIER T_BIT_LIT endline { $$ = node_init(T_PARAM, (data_t){.iVal=0}, $2, NULL, NULL); }
;

enum:                 T_ENUM T_IDENTIFIER T_INTEGER endline     { $$ = node_init(T_PARAM, (data_t){.iVal=0}, $2, NULL, NULL); }
;

pattern:              T_PATTERN T_IDENTIFIER T_STRING T_BIT_CONSTANT endline
                    | T_PATTERN T_IDENTIFIER T_STRING T_BIT_LIT endline
;

order_args:           T_INTEGER
                    | order_args T_INTEGER
;

order:                T_ORDER T_IDENTIFIER order_args endline
;

opcode:               T_OPCODE T_IDENTIFIER T_STRING T_BIT_LIT endline
                    | T_OPCODE T_IDENTIFIER T_STRING T_CONSTANT endline
                    | T_OPCODE T_IDENTIFIER T_STRING endline
;

expr:                 T_LEFTPAR T_INTEGER T_RIGHPAR
                    | T_LEFTPAR T_RIGHPAR
;

subst: T_SUBST expr
;

format:               T_FORMAT T_IDENTIFIER bit_pattern endline

bit_pattern:          T_LEFTSQBRACK bit_pattern_args T_RIGHSQBRACK
;

bit_pattern_args:     bit_elem
                    | bit_pattern_args T_COMMA bit_elem
;

bit_elem:             T_BIT_LIT
                    | T_BIT_CONSTANT
                    | subst
                    | T_IDENTIFIER
                    | T_ELIPSIS
;

command:              param
                    | constant
                    | enum
                    | pattern
                    | order
                    | opcode
                    | format

page:                 %empty
                    | page command
                    | page endline

%%


char** files;
int totalFiles;
int currentFileIndex;
bool readFromStdin = false;
bool writeToStdout = false;

//Support multiple files
int yywrap()
{
    /*Open next file only if one or more remain and we are not reading from stdin*/
    if(currentFileIndex < totalFiles && !readFromStdin)
    {
        printf("Reading file %s\n", files[currentFileIndex]);
        yyin = fopen( files[currentFileIndex], "r" );
        currentFileIndex++;
        return 0;
    }
    return 1;
}

int build_ast(int argc, char** argv)
{
    /*Allocate memory for the files paths*/
    totalFiles = argc - 1;                      //Don't count argv[0] (programm name)
    currentFileIndex = 0;                       //Start at the first file
    files = malloc(sizeof(char*) * totalFiles);              // and allocate the memory for the paths
    for(int i = 0; i < totalFiles; i++)         
    {
        files[i] = argv[i + 1];                     //Register all file paths. Note that it is a pointer to argv
    }
    
    /*Detect if we should read from stdin/write to stdout*/
    readFromStdin = totalFiles <= 0 || !isatty(fileno(stdin));
    writeToStdout = !isatty(fileno(stdout));

    /*Open the first file, or read from stdin*/
    if(readFromStdin)
        yyin = stdin;
    else
        if (yywrap() == 1) exit(-1);
	do
    {
		yyparse();
	}
    while(!feof(yyin));

    printf("DONE\n");

    free(files);
    return 0;
}

//Simple error print
void yyerror(const char* s) {
	fprintf(stderr, "Parse error: %s\n", s);
	exit(1);
}

const char *getTypeName(int type)
{
    return "";
}