%require "3.7"
%define parse.error verbose

%union
{
    char *string;
    double number;
}

%token LCURLY RCURLY RBRACKET LBRACKET COMMA ASSIGN
%token VTRUE VFALSE

%token <string> IDENTIFIER STRING
%token <number> NUMBER

%left LCURLY LBRACKET
%right RCURLY RBRACKET

%{
#include "ffd.lexer.hh"
void yyerror(const char *s);

#ifdef YYDEBUG
int yydebug = 1;
#endif

extern int yylineno;

extern void ffd_push_array(char *var);
extern void ffd_pop_array();

extern void ffd_array_str(char *val);
extern void ffd_array_num(double val);

extern void ffd_push_category(char *var);
extern void ffd_pop_category();

extern void ffd_var(char *var);
extern void ffd_num(double num);
extern void ffd_str(char *str);
extern void ffd_bool(bool b);
%}

%locations

%%

FFD
:
| categories 	// Allow global categories
| members  	    // Allow global members
;

object
: LCURLY         RCURLY { ffd_pop_category(); } //  { }
| LCURLY members RCURLY { ffd_pop_category(); } //  {Members..}
;

array
: RBRACKET  		   LBRACKET { ffd_pop_array(); } // []
| RBRACKET arrayValues LBRACKET { ffd_pop_array(); } // [values...]
;

value
:
| STRING { ffd_str($1);     } // "String"
| NUMBER { ffd_num($1);     } // 12345
| VTRUE  { ffd_bool(true);  } // true
| VFALSE { ffd_bool(false); } // false
;

arrayValue
:
| STRING { ffd_array_str($1); }
| NUMBER { ffd_array_num($1); }
;

arrayValues
: arrayValue
| arrayValues COMMA arrayValue
;

members
: member
| members member
| members category
| members list
| list
;

categories
: category
| categories category
;

category: 	IDENTIFIER 			{ ffd_push_category($1); } object;
member: 	IDENTIFIER ASSIGN 	{ ffd_var($1); } value; // IDENTIFIER = value
list: 		IDENTIFIER ASSIGN 	{ ffd_push_array($1); } array;

%%

void yyerror(const char *s)
{
    printf("** Line %d: %s\n", yylineno, s);
}