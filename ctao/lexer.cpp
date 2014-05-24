#include "lexer.h"
#include <algorithm>
#include <iostream>

lexer::lexer(const char* p){
    _p = p;   
}

lexer::lexer(ifstream* _in): in(_in){
	_p = NULL;
	init();
}

char lexer::get_next(){
	if(_p){
		_p++;
		return *_p;	
	}
	else{
		if(in->good())
			return in->get();
		else
		{

			return EOF;
		}
	}
}

void lexer::init(){

	reserved_words["call"] = token::callsym;
    reserved_words["begin"] = token::beginsym;
    reserved_words["end"] = token::endsym;
    reserved_words["if"] = token::ifsym;
    reserved_words["while"] = token::whilesym;
    reserved_words["then"] = token::thensym;
    reserved_words["do"] = token::dosym;
    reserved_words["const"] = token::constsym;
    reserved_words["var"] = token::varsym;
    reserved_words["procedure"] = token::procsym;
    reserved_words["oddsym"] = token::oddsym;
    reserved_words["print"] = token::print;
    reserved_words["array"] = token::arraysym;
        
    punctuation["("] = token::lparen; 
    punctuation[")"] = token::rparen; 
    punctuation["*"] = token::times;
    punctuation["/"] = token::slash;
    punctuation["+"] = token::plus;
    punctuation["-"] = token::minus;
    punctuation["="] = token::eql;
    punctuation["!="] = token::neq;
    punctuation["<"] = token::lss;
    punctuation["<="] = token::leq;
    punctuation[">"] = token::gtr;
    punctuation[">="] = token::geq;
    punctuation[";"] = token::semicolon;
    punctuation[":="] = token::becomes;
    punctuation["."] = token::period;
    punctuation["!"] = token::print;
    punctuation[","] = token::comma;
    punctuation["["] = token::lsquare;
    punctuation["]"] = token::rsquare;
}

int lexer::next(){
    
    identifier_str.clear();
    
    while(isspace(*_p))
        _p++;
    
    if (isalpha(*_p)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
        do{
                identifier_str += *_p;
                _p++;
        }while (isalnum(*_p));
        
        std::cout << identifier_str << std::endl;
        
        transform(identifier_str.begin(), identifier_str.end(), identifier_str.begin(), ::tolower);
        
        it = reserved_words.find(identifier_str);
        
        if(it != reserved_words.end()){
           return  it->second;
        }
        else{
            return token::identifier;
        }
    }
    
    if (isdigit(*_p)) {   // Number: [0-9]+
        string NumStr;
        
        do {
          NumStr += *_p;
          _p++;
        } while (isdigit(*_p));

        num_val = strtod(NumStr.c_str(), 0);
        
        std::cout << num_val << std::endl;
        
        return token::number;
    }
    
    if (ispunct(*_p)) { // punctuation
        identifier_str = *_p;
        _p++;
        while (ispunct(*_p)){
                identifier_str += *_p;
                _p++;
        }
        
        std::cout << identifier_str << std::endl;
        
        it = punctuation.find(identifier_str);

		while(it == punctuation.end() && identifier_str.size() > 1){
			
			identifier_str = identifier_str.substr(0, identifier_str.size() - 1);

			_p--;

			std::cout << identifier_str << std::endl;

			it = punctuation.find(identifier_str);
		}

        
        if(it != punctuation.end()){
           return  it->second;
        }
    }
    
    
    if (*_p == EOF)
        return token::eof;
    
    return token::error;
    
        
}
