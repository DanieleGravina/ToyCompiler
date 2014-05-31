/* 
 * File:   main.cpp
 * Author: daniele
 *
 * Created on 28 aprile 2014, 11.11
 */

#include <string>
#include <cstdio>
#include <fstream>
#include "lexer.h"
#include "frontend.h"

#define ARGS 1

int main(int argc, char **argv) {

    using namespace std;

    /*if (argc != ARGS + 1) {
 fprintf(stderr, "Must use %d arguments", ARGS);
 fputs("\n", stderr);
 exit(1);
}

    ifstream in(argv[1]);

    if(!in) {
            fprintf(stderr, "Could not open file %s\n", argv[1]);
            exit(1);
    }*/

    const char* s_punt = "VAR x , squ, a; "
            " ARRAY y[10]; "
            " PROCEDURE square z ; "
            " BEGIN "
            "  squ := z * (z+z) "
            " END; "

            " BEGIN "
            " x := 1; "
            " a := y[1]; "
            " WHILE x <= 10 DO "
            "  BEGIN "
            "  CALL square x ; "
            "  !squ; "
            "  x := x + 1 "
            "  END "
            " END. ";


    lexer l(s_punt);

    frontend f(l);

    f.program();

    //in.close();
}

