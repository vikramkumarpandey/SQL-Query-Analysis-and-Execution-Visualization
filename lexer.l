%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "parser.tab.h"
%}
%option noyywrap
%option case-insensitive

%%

"SELECT"        { return SELECT; }
"INSERT"        { return INSERT; }
"UPDATE"        { return UPDATE; }
"DELETE"        { return DELETE; }
"FROM"          { return FROM;   }
"WHERE"         { return WHERE;  }
"INTO"          { return INTO;   }
"VALUES"        { return VALUES; }
"SET"           { return SET;    }
"AND"           { return AND;    }
"OR"            { return OR;     }
"="             { return EQ;     }
"<"             { return LT;     }
">"             { return GT;     }
"*"             { return STAR;   }
","             { return COMMA;  }
";"             { return SEMI;   }
"("             { return LPAREN; }
")"             { return RPAREN; }

\'([^\'\\]|\\.)*\' {
    int len = strlen(yytext);
    char *buf = (char *)malloc(len - 1);
    strncpy(buf, yytext + 1, len - 2);
    buf[len - 2] = '\0';
    yylval.strval = buf;
    return STRING;
}

[0-9]+          {
    yylval.strval = strdup(yytext);
    return NUMBER;
}

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.strval = strdup(yytext);
    return IDENTIFIER;
}

[ \t\r\n]+      {  }

.               {
    fprintf(stderr, "Lexer warning: unexpected character '%s'\n", yytext);
}

%%
