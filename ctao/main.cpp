/* 
 * File:   main.cpp
 * Author: daniele
 *
 * Created on 28 aprile 2014, 11.11
 */

#include <string>
#include <cstdio>
#include "lexer.h"
#include "frontend.h"

int main(int argc, char **argv){
    
    using namespace std;
    
    const char* s_punt = "VAR x, squ; "
    " PROCEDURE square; "          
    " BEGIN "
    " squ := x * x "
    "        END; "

    " BEGIN "
    " x := 1; "
    " WHILE x <= 10 DO "
    " BEGIN "
    " CALL square; "
    " !squ; "
    " x := x + 1 "
    " END "
    " END. ";
    
    
    
    //s_punt = &argv[1][0];
    
    lexer l(s_punt);
    
    frontend f(l);
    
    f.program();         
    
}

