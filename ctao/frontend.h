/* 
 * File:   frontend.h
 * Author: daniele
 *
 * Created on 28 aprile 2014, 11.14
 */

#ifndef FRONTEND_H
#define	FRONTEND_H

#include<map>
#include<list>
#include<string>
#include<stack>
#include<cstdio>

#include "token.h"
#include "lexer.h"

using namespace std;

class frontend{
    
    frontend(lexer lex):lex(lex){
        getNextToken();
    }
    
    
    
    void program(){
        block();
        expect(token::TokenType::period);
    }
    
    
private:
    
    lexer lex;
    
    int CurTok;
    
    string name;
    
    int value;
    
    int getNextToken() {
        return CurTok = lex.next();
    }
    
    void error(string s){
        std::cout << s << endl;
    }
    
    void expect(int tok){
        std::cout << "expecting " << tok << std::endl;
        if (!accept(tok)){
            error("unexpected symbol");
        }
    }
    
    bool accept(int tok){
        
        bool result = tok == CurTok;
        
        cout << "accepting " << tok << " == " << CurTok << endl;
        
        getNextToken();
        
        return result;
            
    }
    
    void block(){
        if(accept(token::TokenType::constsym)){
            
            expect(token::TokenType::identifier);
            
            name = lex.Identifier(); 
            expect(token::TokenType::eql);
            expect(token::TokenType::number);
	    //local_vars.append(Symbol(name, standard_types['int']), value);
            
	    while ( accept(token::TokenType::comma) ) {
			expect(token::TokenType::identifier);
			name = lex.Identifier(); 
                        expect(token::TokenType::eql);
                        expect(token::TokenType::number);
			//local_vars.append(Symbol(name, standard_types['int']), value)
            }
            
	    expect(token::TokenType::semicolon);
            
        }
        if(accept(token::TokenType::varsym)){
            
        }
        while(accept(token::TokenType::procsym)){
            
        }
    }
    
    

    
};



#endif	/* FRONTEND_H */

