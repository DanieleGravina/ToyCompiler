#include <lexer.h>

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
    
    if (ispunct(*_p)) { // punctuation
        identifier_str = *_p;
        _p++;
        while (ispunct(*_p)){
                identifier_str += *_p;
                _p++;
        }
        
        it = punctuation.find(identifier_str);
        
        if(it != punctuation.end()){
           return  it->second;
        }
    }
    
    
    if (*_p == EOF)
        return token::TokenType::eof;
    
    return token::TokenType::error;
    
        
}
