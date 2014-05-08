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
#include<iostream>
#include "lexer.h"

using namespace std;

class frontend{
    
public:
    
    frontend(lexer lex):lex(lex){
        getNextToken();
    }
    
    
    
    void program(){
        block();
        expect(token::period);
    }
    
    
private:
    
    lexer lex;
    
    int CurTok;
    
    string name;
    
    int value;
    
    int getNextToken() {
        return CurTok = lex.next();
    }
    
    void error(const char s[]){
        cout << s << endl;
    }
    
    bool expect(int tok){
        
        cout << "expecting " << tok << endl;
        
        if (!accept(tok)){
            error("unexpected symbol");
            return false;
        }
        
        return true;
    }
    
    bool accept(int tok){
        
        cout << "accepting " << tok << " == " << CurTok << endl;
        
        if(tok == CurTok){
            getNextToken();
            return true;
        }
        
        return false;
            
    }
    
    void block(){
        if(accept(token::constsym)){
            
            expect(token::identifier);
            
            name = lex.Identifier(); 
            expect(token::eql);
            expect(token::number);
	    //local_vars.append(Symbol(name, standard_types['int']), value);
            
	    while ( accept(token::comma) ) {
			expect(token::identifier);
			name = lex.Identifier(); 
                        expect(token::eql);
                        expect(token::number);
			//local_vars.append(Symbol(name, standard_types['int']), value)
            }
            
	    expect(token::semicolon);
            
        }
        if(accept(token::varsym)){
            
            expect(token::identifier);
            
            //local_vars.append(Symbol(value, standard_types['int']))
            while (accept(token::comma)){
                    expect(token::identifier);
                    //local_vars.append(Symbol(value, standard_types['int']))
            }
            expect(token::semicolon); 
           
        }
        
        while(accept(token::procsym)){
            
            expect(token::identifier);
            string fname = lex.Identifier();
            expect(token::semicolon);
            //local_vars.append(Symbol(fname, standard_types['function']))
            block();
            expect(token::semicolon);
            //defs.append(FunctionDef(symbol=local_vars.find(fname), body=fbody))
        }
        
        statement();
	//return Block(gl_sym=symtab, lc_sym=local_vars, defs=defs, body=stat)
    }
    
    void statement(){
        
        switch(CurTok){
            case token::identifier : 
                cout << "accepting " << token::identifier << " == " << CurTok << endl;
                getNextToken();
                //target=symtab.find(value)
		expect(token::becomes);
		expression();
		//return AssignStat(target=target, expr=expr, symtab=symtab)
                break;
            case token::callsym :
                cout << "accepting " << token::callsym << " == " << CurTok << endl;
                getNextToken();
                expect(token::identifier);
                //return CallStat(call_expr=CallExpr(function=symtab.find(value), symtab=symtab), symtab=symtab)
                break;
                
            case token::beginsym :
                cout << "accepting " << token::beginsym << " == " << CurTok << endl;
                getNextToken();
                //statement_list = StatList(symtab=symtab)
		//statement_list.append(statement(symtab))
                do{
                 statement();   
                }while (accept(token::semicolon));
			//statement_list.append(statement(symtab))
		expect(token::endsym);
		//statement_list.print_content()
		//return statement_list
                break;
                
            case token::ifsym :
                cout << "accepting " << token::ifsym << " == " << CurTok << endl;
                getNextToken();
                //cond=condition()
                condition();
		expect(token::thensym);
                statement();
		//then=statement(symtab)
		//return IfStat(cond=cond,thenpart=then, symtab=symtab)
                break;
                
            case token::whilesym :
                cout << "accepting " << token::whilesym << " == " << CurTok << endl;
                getNextToken();
                //cond=condition(symtab)
                condition();
		expect(token::dosym);
                statement();
		//return WhileStat(cond=cond, body=body, symtab=symtab)
                break;
                
            case token::print :
                cout << "accepting " << token::print << " == " << CurTok << endl;
                getNextToken();
                expect(token::identifier);
		//return PrintStat(symbol=symtab.find(value),symtab=symtab)
                break;
                
            default :
                error("statement : syntax error");
                getNextToken();
                break;
                
        }
        
    }
    
    void expression(){
        if (CurTok == token::plus || CurTok == token::minus)
            getNextToken();
        term();
        while (CurTok == token::plus || CurTok == token::minus) {
            getNextToken();
            term();
        }
    }
    
    void condition(){
        
        if (accept(token::oddsym)) {
            expression();
        } else {
            expression();
            if (CurTok == token::eql || CurTok == token::neq || 
                    CurTok == token::lss || CurTok == token::leq || 
                    CurTok == token::gtr || CurTok == token::geq) {
                getNextToken();
                expression();
            } else {
                error("condition: invalid operator");
                getNextToken();
            }
        }

        
    }
    
    void term() {
        factor();
        while (CurTok == token::times || CurTok == token::slash) {
            getNextToken();
            factor();
        }
    }
    
    void factor(){
        if (accept(token::identifier)) {
        ;
        } else if (accept(token::number)) {
            ;
        } else if (accept(token::lparen)) {
            expression();
            expect(token::rparen);
        } else {
            error("factor: syntax error");
            getNextToken();
        }
    }
    
    

    
};



#endif	/* FRONTEND_H */

