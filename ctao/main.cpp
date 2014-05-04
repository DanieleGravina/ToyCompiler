/* 
 * File:   main.cpp
 * Author: daniele
 *
 * Created on 28 aprile 2014, 11.11
 */

#include <string>
#include <token.h>
#include <cstdio>
#include <lexer.h>

int main(){
    
    string s = "VAR x, squ;  PROCEDURE square; BEGIN squ := x * x END; BEGIN x := 1; WHILE x <= 10 DO BEGIN CALL square;x := x + 1 ; !squ    END END."
    
    char* s_punt = s;
    
    lexer l(s_punt);
    
    while(l.next() != token::TokenType::eof){
        std::cout << (token::TokenType)l.next() << std::endl; 
    }
    
    
            
    
}

