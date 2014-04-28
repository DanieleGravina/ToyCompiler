/* 
 * File:   lexer.h
 * Author: daniele
 *
 * Created on 28 aprile 2014, 11.44
 */

#ifndef LEXER_H
#define	LEXER_H

#include<map>
#include<token.h>
#include<string>
#include<cctype>
#include<algorithm>

using namespace std;



class lexer{
    
public:
    
    lexer(char *p){
        _p = p;
        
    }
    
    int next();
    
private:
    
    char *_p;
    
    string identifier_str;  
    double num_val;    
    
    map<string, token::TokenType> reserved_words;
    map<string, token::TokenType> punctuation;
    
    map<string, token::TokenType>::iterator it;
    
    
};

int lexer::next(){
    
    while(isspace(*_p))
        _p++;
    
    if (isalpha(*_p)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
        identifier_str = *_p;
        while (isalnum(*_p)){
                identifier_str += *_p;
                _p++;
        }
        
        transform(identifier_str.begin(), identifier_str.end(), identifier_str.begin(), ::tolower);
        
        it = reserved_words.find(identifier_str);
        
        if(it != reserved_words.end()){
           return  it->second;
        }
        else{
            return token::TokenType::identifier;
        }
    }
    
    if (isdigit(*_p)) {   // Number: [0-9]+
        string NumStr;
        
        do {
          NumStr += *_p;
          _p++;
        } while (isdigit(*_p));

        num_val = strtod(NumStr.c_str(), 0);
        
        return token::TokenType::number;
    }
    
    it = punctuation.find(*_p);
        
    if(it != punctuation.end()){
       return  it->second;
    }
    
    
    if (*_p == EOF)
        return token::TokenType::eof;
    
    return token::TokenType::error;
    
        
}



static const map<string, token::TokenType> reserved_words(){
    map<string, token::TokenType> result;
    
    result["call"] = token::TokenType::callsym;
    result["begin"] = token::TokenType::beginsym;
    result["end"] = token::TokenType::endsym;
    result["if"] = token::TokenType::ifsym;
    result["while"] = token::TokenType::whilesym;
    result["then"] = token::TokenType::thensym;
    result["do"] = token::TokenType::dosym;
    result["const"] = token::TokenType::constsym;
    result["var"] = token::TokenType::varsym;
    result["procedure"] = token::TokenType::procsym;
    result["oddsym"] = token::TokenType::oddsym;
    result["print"] = token::TokenType::print;
    
    return result;
}



#endif	/* LEXER_H */

