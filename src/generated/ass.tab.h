/* A Bison parser, made by GNU Bison 3.7.6.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_SRC_GENERATED_ASS_TAB_H_INCLUDED
# define YY_YY_SRC_GENERATED_ASS_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 5 "src/ass.y"

    #include <unistd.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdbool.h>
    #include <stdint.h>

    #include "../linked_list.h"
    #include "../ast_node.h"

    int build_ast(int argc, char** argv);
    const char *getTypeName(int type);

#line 63 "src/generated/ass.tab.h"

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 1,                   /* error  */
    YYUNDEF = 2,                   /* "invalid token"  */
    T_LINE = 3,                    /* T_LINE  */
    T_WHITESPACE = 4,              /* T_WHITESPACE  */
    T_S_COMMENT = 5,               /* T_S_COMMENT  */
    T_M_COMMENT = 6,               /* T_M_COMMENT  */
    T_DS_COMMENT = 7,              /* T_DS_COMMENT  */
    T_DM_COMMENT = 8,              /* T_DM_COMMENT  */
    T_PARAM = 9,                   /* T_PARAM  */
    T_CONSTANT = 10,               /* T_CONSTANT  */
    T_ENUM = 11,                   /* T_ENUM  */
    T_PATTERN = 12,                /* T_PATTERN  */
    T_ORDER = 13,                  /* T_ORDER  */
    T_OPCODE = 14,                 /* T_OPCODE  */
    T_FORMAT = 15,                 /* T_FORMAT  */
    T_UNKNOWN_CMD = 16,            /* T_UNKNOWN_CMD  */
    T_INTEGER = 17,                /* T_INTEGER  */
    T_BIT_CONSTANT = 18,           /* T_BIT_CONSTANT  */
    T_BIT_LIT = 19,                /* T_BIT_LIT  */
    T_STRING = 20,                 /* T_STRING  */
    T_IDENTIFIER = 21,             /* T_IDENTIFIER  */
    T_SUBST = 22,                  /* T_SUBST  */
    T_LEFTPAR = 23,                /* T_LEFTPAR  */
    T_RIGHPAR = 24,                /* T_RIGHPAR  */
    T_LEFTSQBRACK = 25,            /* T_LEFTSQBRACK  */
    T_RIGHSQBRACK = 26,            /* T_RIGHSQBRACK  */
    T_ELIPSIS = 27,                /* T_ELIPSIS  */
    T_COMMA = 28                   /* T_COMMA  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 33 "src/ass.y"

    data_t*         dVal;
    node_t*         nVal;
    linked_list_t*  lVal;

#line 114 "src/generated/ass.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SRC_GENERATED_ASS_TAB_H_INCLUDED  */
