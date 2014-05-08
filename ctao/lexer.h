/* 
 * File:   lexer.h
 * Author: daniele
 *
 * Created on 28 aprile 2014, 11.44
 */

#ifndef LEXER_H
#define	LEXER_H

#include<map>
#include<string>
#include<cstdio>

using namespace std;

class token{
    
public:
    
    enum TokenType{
        error,
        eof,
        number,
        identifier,
        lparen,
        rparen,
        times,
        slash,
        plus,
        minus,
        eql,
        neq,
        lss,
        leq,
        gtr,
        geq,
        callsym,
        beginsym,
        semicolon,
        endsym,
        ifsym,
        whilesym,
        becomes,
        thensym,
        dosym,
        constsym,
        comma,
        varsym,
        procsym,
        period,
        oddsym,
        print
    };
    
private:
    //just a wrapper class
    token();
};


class lexer{
    
public:
    
    lexer(const char *p);
    
    ~lexer(){
        
    }
    
    int next();
    
    string Identifier(){
        return identifier_str;
    }
    
    int Value(){
        return num_val;
    }
    
private:
    
    const char *_p;
    
    string identifier_str;  
    int num_val;    
    
    map<string, token::TokenType> reserved_words;
    map<string, token::TokenType> punctuation;
    
    map<string, token::TokenType>::iterator it;
    
    lexer&  operator=(lexer& rhs);  
    
    
};
        


#endif	/* LEXER_H */

