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

    const char* s_punt = "VAR x , squ, a, y; "
            " PROCEDURE square z ; "
            " BEGIN "
            "  squ := z * z "
            " END; "

            " BEGIN "
            " y := 1;"
            " x := 1; "
            " a := y; "
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

