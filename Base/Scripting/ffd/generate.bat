bison -d -o ffd.skeleton.cc parser.y
flex  --wincompat --header-file=ffd.lexer.hh -o ffd.lexer.cc scanner.l