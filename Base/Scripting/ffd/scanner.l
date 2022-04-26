%option yylineno
%option noinput nounput noyywrap 8bit

%{
#include "ffd.skeleton.hh"
%}

%%

(\/\*([^*]|[\r\n]|(\*+([^*\/]|[\r\n])))*\*+\/)|(\/\/.*) ; { /* Ignore Comments */ }
[ \t\r\n]+                                              ; { /* Ignore whitespaces */ }

"true"                               { return VTRUE;  }
"false"                              { return VFALSE; }

\"[^"]*\"                            { yylval.string = strdup(yytext); return STRING;       }
[A-Za-z_][A-Za-z_0-9]+               { yylval.string = strdup(yytext); return IDENTIFIER;   }
[+-]?([0-9]*[.])?[0-9]+              { yylval.number = atof(yytext); return NUMBER; }

\{                                   { return LCURLY; }
\}                                   { return RCURLY; }
\=                                   { return ASSIGN; }

\[                                   { return RBRACKET; }
\]                                   { return LBRACKET; }
\,                                   { return COMMA; }

%%
