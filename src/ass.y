%debug
%locations
%define api.token.raw
%define parse.error custom

%code requires {
    #include <unistd.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdbool.h>
    #include <stdint.h>
    #include <string.h>

    #include "../linked_list.h"
    #include "../ast_node.h"
    #include "../bitpattern.h"

    int parse_file(int, char**);
    const char *getTypeName(int type);
}

%code top{
    #include <unistd.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdbool.h>
    #include <stdint.h>
    #include <string.h>

    #include "../bitpattern.h"
    #include "../failure.h"
    #include "../parameters.h"
    #include "../commands.h"

    extern int yylex();
    extern int yyparse();
    extern FILE* yyin;

    void yyerror(const char* s);
    linked_list_t* subst(data_t* id, data_t* val);
}

%union {
    data_t*         dVal;
    node_t*         nVal;
    linked_list_t*  lVal;
}

%code{
    //NULL until tree is completely built
    node_t* topNode = NULL;
    linked_list_t* top_list = NULL;
}

/********************************************************************************************************/
/*                                              BEGIN TOKENS                                            */
/********************************************************************************************************/

/* Others */
%token T_LINE
%token T_WHITESPACE
%token<dVal> T_C_BLOCK

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
%token T_CODE
%token T_OUTPUT
%token T_OVERRIDE
%token T_UNKNOWN_CMD

/* Constant */
%token<dVal> T_INTEGER
%token<dVal> T_BIT_CONSTANT
%token<dVal> T_BIT_LIT
%token<dVal> T_STRING

/* Identifier */
%token<dVal> T_IDENTIFIER

/* Substitution */
%token<dVal> T_SUBST

/* Punctuation */
%token T_LEFTPAR             
%token T_RIGHPAR   
%token T_LEFTSQBRACK         
%token T_RIGHSQBRACK         
%token T_ELIPSIS             
%token T_COMMA  
%token T_PLUS    
%token T_MINUS   
%token T_MULTIPLY
%token T_DIVIDE  
%token T_MODULO  

%start page


/* Types */
%type<lVal> param_args
%type<lVal> bit_elem
%type<lVal> bit_pattern_args
%type<lVal> bit_pattern
%type<lVal> subst
%type<lVal> order_args

%type page
%type param
%type constant
%type enum
%type pattern
%type order
%type opcode
%type format
%type command

%%

endline:              T_M_COMMENT                               
                    | T_S_COMMENT T_LINE                        
                    | T_S_COMMENT YYEOF                         
                    | T_DM_COMMENT                              
                    | T_DS_COMMENT T_LINE                       
                    | T_DS_COMMENT YYEOF                        
                    | T_LINE                                    
                    | YYEOF                                     
;

param_args:           T_IDENTIFIER                              { $$ = list_init(YYSYMBOL_T_IDENTIFIER, $1, eDATA); }
                    | param_args T_IDENTIFIER                   { $$ = $1; list_append($1, list_init(YYSYMBOL_T_IDENTIFIER, $2, eDATA)); }
                    | param_args T_INTEGER                      { $$ = $1; list_append($1, list_init(YYSYMBOL_T_INTEGER, $2, eDATA)); }
                    | param_args T_STRING                       { $$ = $1; list_append($1, list_init(YYSYMBOL_T_STRING, $2, eDATA)); }
                    | param_args T_BIT_LIT                      { $$ = $1; list_append($1, list_init(YYSYMBOL_T_BIT_LIT, $2, eDATA)); }
                    | param_args T_BIT_CONSTANT                 { $$ = $1; list_append($1, list_init(YYSYMBOL_T_BIT_CONSTANT, $2, eDATA)); }
;

param:                T_PARAM param_args endline                { fail_set_loc(@$); fail_show_loc(true); command_param($2); }
;

constant:             T_CONSTANT T_IDENTIFIER T_BIT_LIT endline { fail_set_loc(@$); fail_show_loc(true); command_bit_const($2, $3); }
                    | T_CONSTANT T_IDENTIFIER T_INTEGER endline { fail_set_loc(@$); fail_show_loc(true); command_int_const($2, $3); }
                    | T_CONSTANT T_IDENTIFIER T_STRING endline { fail_set_loc(@$); fail_show_loc(true); command_str_const($2, $3); }
;

enum:                 T_ENUM T_IDENTIFIER T_INTEGER endline     { fail_set_loc(@$); fail_show_loc(true); command_enum($2, $3); }
;

pattern:              T_PATTERN T_IDENTIFIER T_STRING T_BIT_CONSTANT endline { fail_set_loc(@$); fail_show_loc(true); command_pattern($2, $3, $4); }
                    | T_PATTERN T_IDENTIFIER T_STRING T_BIT_LIT endline { fail_set_loc(@$); fail_show_loc(true); command_pattern($2, $3, $4); }
;

order_args:           T_INTEGER                 { $$ = list_init(YYSYMBOL_T_INTEGER, $1, eDATA); }
                    | order_args T_INTEGER      { $$ = $1; list_append($1, list_init(YYSYMBOL_T_INTEGER, $2, eDATA)); }
;

order:                T_ORDER T_IDENTIFIER order_args endline           { fail_set_loc(@$); fail_show_loc(true); command_order($2, $3); }
;

opcode:               T_OPCODE T_IDENTIFIER T_STRING T_BIT_LIT endline          { fail_set_loc(@$); fail_show_loc(true); command_opcode($2, $3, $4, false); }
                    | T_OPCODE T_IDENTIFIER T_STRING T_BIT_CONSTANT endline     { fail_set_loc(@$); fail_show_loc(true); command_opcode($2, $3, $4, true); }
                    | T_OPCODE T_IDENTIFIER T_STRING endline                    { fail_set_loc(@$); fail_show_loc(true); command_opcode($2, $3, NULL, false); }
;

subst:                T_SUBST T_LEFTPAR T_INTEGER T_RIGHPAR             { fail_set_loc(@$); fail_show_loc(true); $$ = subst($1, $3); }
;

format:               T_FORMAT T_IDENTIFIER bit_pattern endline         { fail_set_loc(@$); fail_show_loc(true); command_format($2, $3); }
;

bit_pattern:          T_LEFTSQBRACK bit_pattern_args T_RIGHSQBRACK      { $$ = $2; }
;

bit_pattern_args:     bit_elem                              {$$ = $1;}
                    | bit_pattern_args T_COMMA bit_elem     {$$ = $1; list_append($1, $3);}
;

bit_elem:             T_BIT_LIT         {$$ = list_init(YYSYMBOL_T_BIT_LIT, bit_elem_init(eBP_BIT_LIT, 0, &($1->bVal)), eBIT_ELEM);}
                    | T_BIT_CONSTANT    {$$ = list_init(YYSYMBOL_T_BIT_CONSTANT, bit_elem_init(eBP_BIT_CONST, 0, $1->strVal), eBIT_ELEM);}
                    | subst             {$$ = $1;}
                    | T_IDENTIFIER      {$$ = list_init(YYSYMBOL_T_IDENTIFIER, bit_elem_init(eBP_ENUM, 0, $1->strVal), eBIT_ELEM);}
                    | T_ELIPSIS         {$$ = list_init(YYSYMBOL_T_IDENTIFIER, bit_elem_init(eBP_ELLIPSIS, 0, NULL), eBIT_ELEM);}
;

command:              param
                    | constant
                    | enum
                    | pattern
                    | order
                    | opcode
                    | format
                    | code
                    | override
                    | output
;

code:                 T_CODE T_C_BLOCK endline    { fail_set_loc(@$); fail_show_loc(true); command_code($2); }
;

output:               T_OUTPUT T_IDENTIFIER T_STRING T_C_BLOCK endline  { fail_set_loc(@$); fail_show_loc(true); command_output($2,$3,$4); }
;

override:             T_OVERRIDE T_IDENTIFIER T_C_BLOCK endline { fail_set_loc(@$); fail_show_loc(true); command_override($2,$3); }
;


page:                 %empty 
                    | page command 
                    | page endline 
                    | page error endline
;

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
        fail_show_loc(false);
        fail_detail("Reading file '%s'", files[currentFileIndex]);
        yyin = fopen(files[currentFileIndex], "r" );
        currentFileIndex++;
        if(yyin == NULL)
        {
            fail_error("Unable to open '%s'");
            exit(EXIT_FAILURE);
        }
        return 0;
    }
    return 1;
}

int parse_file(int _totalFiles, char** _files)
{   
    totalFiles = _totalFiles;
    files = _files;
    
    currentFileIndex = 0;                       //Start at the first file
    
    /*Detect if we should read from stdin/write to stdout*/
    readFromStdin = totalFiles <= 0 || !isatty(fileno(stdin));
    writeToStdout = !isatty(fileno(stdout));

    /*Open the first file, or read from stdin*/
    if(readFromStdin)
    {
        yyin = stdin;
        printf(">");
    }
    else if (yywrap() == 1)
    {
        printf("ERROR : no input file.\n");
        exit(EXIT_FAILURE);
    }
    
    //Parse all files
	do
    {
		yyparse();
	}
    while(!feof(yyin));

    return 0;
}

//Simple error print
void yyerror(const char* s) {
	fail_error("%s", s);
}

const char *getTypeName(int type)
{
    return yysymbol_name(type);
}

static int yyreport_syntax_error (const yypcontext_t *ctx)
{
    int res = 0;
    char error_message[1024] = "syntax error, "; 
    fail_set_loc(*yypcontext_location (ctx));

    // Report the tokens expected at this point.
    enum { TOKENMAX = 5 };
    yysymbol_kind_t expected[TOKENMAX];
    int n = yypcontext_expected_tokens (ctx, expected, TOKENMAX);
    if (n < 0)
    {
        // Forward errors to yyparse.
        res = n;
    }
    else
    {
        for (int i = 0; i < n; ++i)
        {
            strcat(error_message, i == 0 ? "expected " : " or ");
            strcat(error_message, yysymbol_name (expected[i]));
        }
    }

    // Report the unexpected token.

    yysymbol_kind_t lookahead = yypcontext_token (ctx);
    if (lookahead != YYSYMBOL_YYEMPTY)
    {
        strcat(error_message, " before ");
        strcat(error_message, yysymbol_name (lookahead));
    }
    fail_error(error_message);
}

linked_list_t* subst(data_t* id, data_t* val)
{
    bit_elem_t* new_bit_elem;
    if(strcmp(id->strVal, "ID") == 0)
    {
        new_bit_elem = bit_elem_init(eBP_ID, val->iVal, NULL);
    }
    else if(strcmp(id->strVal, "IMMEDIATE") == 0)
    {
        new_bit_elem = bit_elem_init(eBP_IMMEDIATE, val->iVal, NULL);
    }
    else if(strcmp(id->strVal, "LABEL_ABS") == 0)
    {
        new_bit_elem = bit_elem_init(eBP_LABEL_ABS, val->iVal, NULL);
    }
    else if(strcmp(id->strVal, "LABEL_REL") == 0)
    {
        new_bit_elem = bit_elem_init(eBP_LABEL_REL, val->iVal, NULL);
    }
    else
    {
        new_bit_elem = bit_elem_init(eBP_UNDEF, val->iVal, NULL);
        fail_error("Unkown substitution '%s'", id->strVal);
    }

    return list_init(YYSYMBOL_T_SUBST, (void*)new_bit_elem, eBIT_ELEM);
}