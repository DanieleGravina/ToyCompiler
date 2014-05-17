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
#include<vector>
#include "lexer.h"
#include "ir.h"

using namespace std;

class frontend{
    
public:
    
    frontend(lexer lex):lex(lex){
        getNextToken();
    }
    
    
    
    void program(){
        Block root = block(global_symtab);
        expect(token::period);
    }
    
    
private:
    
    lexer lex;
    
    int CurTok;
    
    string name;
    
    int value;
    
    SymbolTable global_symtab;
    
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
    
    Block block(SymbolTable &symtab){
     
        SymbolTable local_vars;
        
        DefinitionList defs;
        
        if(accept(token::constsym)){
            
            expect(token::identifier);
            
            name = lex.Identifier(); 
            expect(token::eql);
            expect(token::number);
	    local_vars.append(Symbol(name, standard_types[BaseType::INT], lex.Value()));
            
	    while ( accept(token::comma) ) {
			expect(token::identifier);
			name = lex.Identifier(); 
                        expect(token::eql);
                        expect(token::number);
			local_vars.append(Symbol(name, standard_types[BaseType::INT], lex.Value()));
            }
            
	    expect(token::semicolon);
            
        }
        if(accept(token::varsym)){
            
            expect(token::identifier);
            
            local_vars.append(Symbol(lex.Identifier(), standard_types[BaseType::INT]));
            while (accept(token::comma)){
                    expect(token::identifier);
                    local_vars.append(Symbol(lex.Identifier(), standard_types[BaseType::INT]));
            }
            expect(token::semicolon); 
           
        }
        
        while(accept(token::procsym)){
            
            expect(token::identifier);
            
            string fname = lex.Identifier();
            vector<string> fvars;
            vector<Symbol&> parameters;
            
            
            if(accept(token::identifier)){
                
                fvars.push_back(lex.Identifier());
                
                while(accept(token::comma)){
                    expect(token::identifier);
                    fvars.push_back(lex.Identifier());
                }
            }
                
            expect(token::semicolon);
            local_vars.append(Symbol(fname, standard_types[BaseType::FUNCTION]));
            
            for(vector<const string>::size_type i = 0; i < fvars.size(); ++i){
                local_vars.append(Symbol(fvars[i], standard_types[BaseType::INT]));
                parameters[i] = local_vars.find(fvars[i]);
            }
            
            Block fbody = block(local_vars); //bisgna passargli i parametri come var locali
            expect(token::semicolon);
            defs.append(FunctionDef(local_vars.find(fname), parameters, fbody));
        }
        
        IRNode stat = statement(local_vars); //TODO concatena local_var con symtab??
	return Block(symtab, local_vars, stat, defs);
    }
    
    IRNode statement(SymbolTable& symtab){
        
        switch(CurTok){
            
            case token::identifier : 
            {
                cout << "accepting " << token::identifier << " == " << CurTok << endl;
                getNextToken();
                Symbol target = symtab.find(lex.Identifier());
		expect(token::becomes);
                IRNode expr = expression(symtab);
		return AssignStat(target, expr, symtab);
                break;
            }
                
            case token::callsym :
            {
                int tok = token::callsym;
                cout << "accepting " << token::callsym << " == " << CurTok << endl;
                getNextToken();
                expect(token::identifier);
                
                if(accept(token::identifier)){
                  while(accept(token::comma))
                      expect(token::identifier);
                }
                
                return CallStat(CallExpr(symtab.find(lex.Identifier()), tok, symtab), symtab);
                break;
            }
                
            case token::beginsym :
            {
                cout << "accepting " << token::beginsym << " == " << CurTok << endl;
                getNextToken();
                StatList statement_list(symtab);
		statement_list.append(statement(symtab));
                do{
                 statement_list.append(statement(symtab));   
                }while (accept(token::semicolon));
		expect(token::endsym);
		//statement_list.print_content()
		return statement_list;
                break;
            }    
            case token::ifsym :
            {
                cout << "accepting " << token::ifsym << " == " << CurTok << endl;
                getNextToken();
                IRNode cond = condition(symtab);
		expect(token::thensym);
		IRNode then = statement(symtab);
		return IfStat(cond, then, symtab);
                break;
            }
                
            case token::whilesym :
            {
                cout << "accepting " << token::whilesym << " == " << CurTok << endl;
                getNextToken();
                IRNode cond = condition(symtab);
		expect(token::dosym);
                IRNode body = statement(symtab);
		return WhileStat(cond, body, symtab);
                break;
            }    
            case token::print :
            {
                cout << "accepting " << token::print << " == " << CurTok << endl;
                getNextToken();
                expect(token::identifier);
                return PrintStat(symtab.find(lex.Identifier()), symtab);
                break;
            }    
            default :
            {
                error("statement : syntax error");
                getNextToken();
                break;
            }
                
        }
        
        
        
    }
    
    IRNode expression(SymbolTable& symtab){
        
        int op = 0;
        
        if (CurTok == token::plus || CurTok == token::minus){
            op = CurTok;
            getNextToken();
        }
        IRNode expr = term(symtab);
        
        if(op)
            expr = UnExpr(op, expr, symtab);
        
        while (CurTok == token::plus || CurTok == token::minus) {
            getNextToken();
            IRNode expr2 = term(symtab);
            vector<IRNode> children;
            children.push_back(Tok(op, symtab));
            children.push_back(expr);
            children.push_back(expr2);
            expr = BinExpr(op, children, symtab);
        }
        
        return expr;
    }
    
    IRNode term(SymbolTable& symtab) {
        IRNode expr = factor(symtab);
        int op;
        while (CurTok == token::times || CurTok == token::slash) {
            op = CurTok;
            getNextToken();
            IRNode expr2 = factor(symtab);
            vector<IRNode> children;
            children.push_back(Tok(op, symtab));
            children.push_back(expr);
            children.push_back(expr2);
            expr = BinExpr(op, children, symtab);
        }
        return expr;
    }
    
    IRNode factor(SymbolTable &symtab){
        if (accept(token::identifier)) {
            return Var(symtab.find(lex.Identifier()), symtab);
        
        } else if (accept(token::number)) {
            return Const(lex.Value(), symtab);
            
        } else if (accept(token::lparen)) {
            IRNode expr = expression(symtab);
            expect(token::rparen);
            return expr;
        } else {
            error("factor: syntax error");
            getNextToken();
        }
    }
    
    IRNode condition(SymbolTable& symtab){
        
        if (accept(token::oddsym)) {
            return UnExpr(token::oddsym, expression(symtab), symtab);
        } else {
            IRNode expr = expression(symtab);
            if (CurTok == token::eql || CurTok == token::neq || 
                    CurTok == token::lss || CurTok == token::leq || 
                    CurTok == token::gtr || CurTok == token::geq) {
                getNextToken();
                IRNode expr2 = expression(symtab);
                vector<IRNode> children;
                children.push_back(Tok(CurTok, symtab));
                children.push_back(expr);
                children.push_back(expr2);
                return BinExpr(CurTok, children, symtab);
            } else {
                error("condition: invalid operator");
                getNextToken();
            }
        }

        
    }
    
    

    
};



#endif	/* FRONTEND_H */

