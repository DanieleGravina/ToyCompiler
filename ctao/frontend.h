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
#include "registeralloc.h"
#include "Assembler.h"

#define NUM_REGS 4

using namespace std;

class frontend {
public:

    frontend(lexer lex) : lex(lex) {
        getNextToken();

        standard_types[BaseType::INT] = new Type("INTEGER", 4, BaseType::INT);
        standard_types[BaseType::LABEL] = new LabelType();
        standard_types[BaseType::FUNCTION] = new FunctionType();
    }

    ~frontend() {
        delete global_symtab;
        //delete bbs;
    }

    void program() {

        global_symtab = new SymbolTable();

        Block *root = block(global_symtab);
        expect(token::period);

        root->setGlobal();

        root->repr();

        root->lowering();

        cout << "after lowering" << endl;

        root->repr();

        root->flattening();

        cout << "after flattening" << endl;

        root->repr();

		std::list<CFG*> l_cfg;

		std::list<IRNode*> statlists;

		root->getStatLists(statlists);

		for (list<IRNode*>::iterator it = statlists.begin(); it != statlists.end(); ++it) {

			cout << "Statlist : " << (*it)->Id() << "to BBs" << endl;

			std::list<BasicBlock*> *tmp = IRtoBB(*it);

			l_cfg.push_back(new CFG(*tmp));

			delete tmp;
		}

		for (std::list<CFG*>::iterator it = l_cfg.begin(); it != l_cfg.end(); ++it) {
			(*it)->liveness();
			(*it)->print_liveness();
			cout << endl;
		}

		std::map<CFG*, RegisterAlloc*> procedures;

		for (std::list<CFG*>::iterator it = l_cfg.begin(); it != l_cfg.end(); ++it) {

			procedures[*it] = new RegisterAlloc(*(*it), NUM_REGS);

			bool all_reg_assigned = false;
			bool cfg_reg_assigned = false;

			while(!all_reg_assigned){

				cfg_reg_assigned = procedures[*it]->TryAlloc();

				if(!cfg_reg_assigned){
					procedures[*it]->res();

					(*it)->spill(procedures[*it]->getCandidateToSpill());
					(*it)->liveness();
					(*it)->print_liveness();

					delete procedures[*it];

					procedures[*it] = new RegisterAlloc(*(*it), NUM_REGS);
				}
				else{
					all_reg_assigned = true;
					for(CFG::iterator it2 = (*it)->begin(); it2 != (*it)->end(); ++it2){
						if (!(*it2)->registerAllocation(procedures[*it]->allRegs(), procedures[*it]->mapVarReg())){
							all_reg_assigned = false;
							(*it)->spill(procedures[*it]->getCandidateToSpill());
							(*it)->liveness();
							cout << endl;
							(*it)->print_liveness();
							delete procedures[*it];
                            procedures[*it] = new RegisterAlloc(*(*it), NUM_REGS);

							break;
						}

					}
				}
			}

			//procedures[*it]->res();

			cout << "************************************" << endl;
		}




        CodeGenerator codgen(procedures);

		cout << "end" << endl;
    }




private:

    lexer lex;

    int CurTok;

    int value;

    string name;

    SymbolTable* global_symtab;

    std::list<BasicBlock*>* bbs;

    int getNextToken() {
        return CurTok = lex.next();
    }

    void error(const char s[]) {
        cout << s << endl;
		exit(1);
    }

    bool expect(int tok) {

        cout << "expecting " << tok << endl;

        if (!accept(tok)) {
            error("syntax error: unexpected symbol");
            return false;
        }

        return true;
    }

    bool accept(int tok) {

        cout << "accepting " << tok << " == " << CurTok << endl;

        if (tok == CurTok) {
            getNextToken();
            return true;
        }

        return false;

    }

    Block* block(SymbolTable* symtab, SymbolTable* parameters = NULL) {

        SymbolTable* local_vars = new SymbolTable();

        if (parameters) {
            for (SymbolTable::iterator it = parameters->begin(); it != parameters->end(); ++it)
                local_vars->append(it->second);
        }

        DefinitionList* defs = new DefinitionList();

        if (accept(token::constsym)) {

            name = lex.Identifier();
            expect(token::identifier);

            expect(token::eql);
            expect(token::number);
            local_vars->append(new Symbol(name, standard_types[BaseType::INT], lex.Value()));

            while (accept(token::comma)) {
                expect(token::identifier);
                name = lex.Identifier();
                expect(token::eql);
                expect(token::number);
                local_vars->append(new Symbol(name, standard_types[BaseType::INT], lex.Value()));
            }

            expect(token::semicolon);

        }
        if (accept(token::varsym)) {

            string name = lex.Identifier();
            expect(token::identifier);

            local_vars->append(new Symbol(name, standard_types[BaseType::INT]));

            while (accept(token::comma)) {
                name = lex.Identifier();
                expect(token::identifier);
                local_vars->append(new Symbol(name, standard_types[BaseType::INT]));
            }
            expect(token::semicolon);

        }

        if (accept(token::arraysym)) {

            name = lex.Identifier();

            expect(token::identifier);

            expect(token::lsquare);

            value = lex.Value();

            expect(token::number);

            expect(token::rsquare);

            local_vars->append(new Symbol(name, new ArrayType("array", value, BaseType::INT)));
            while (accept(token::comma)) {
                name = lex.Identifier();
                expect(token::identifier);
                expect(token::lsquare);
                value = lex.Value();
                expect(token::number);
                expect(token::rsquare);
                local_vars->append(new Symbol(name, new ArrayType("array", value, BaseType::INT)));
            }
            expect(token::semicolon);

        }

        while (accept(token::procsym)) {

            string fname = lex.Identifier();
            expect(token::identifier);

            SymbolTable* parameters = new SymbolTable();

            name = lex.Identifier();
            if (accept(token::identifier)) {

                //TODO se i parametri sono array??
                Symbol *temp = new Symbol(name, standard_types[BaseType::INT]);
                parameters->append(temp);

                while (accept(token::comma)) {
                    name = lex.Identifier();
                    expect(token::identifier);
                    temp = new Symbol(name, standard_types[BaseType::INT]);
                    parameters->append(temp);
                }
            }

            expect(token::semicolon);
            local_vars->append(new Symbol(fname, standard_types[BaseType::FUNCTION]));


            Block *fbody = block(local_vars, parameters);
            expect(token::semicolon);
            defs->append(new FunctionDef(local_vars->find(fname), parameters, fbody));
        }

        IRNode *stat = statement(new SymbolTable(symtab, local_vars));
        return new Block(symtab, local_vars, stat, defs);
    }

    IRNode* statement(SymbolTable* symtab) {

        switch (CurTok) {

            case token::identifier:
            {
                cout << "accepting identifier" << token::identifier << " == " << CurTok << endl;
                name = lex.Identifier();
                getNextToken();

                if (accept(token::lsquare)) {
                    //TODO review expr_left, could be dangerous
                    IRNode* expr_left = expression(symtab);
                    Symbol* target = symtab->find(name);
                    expect(token::rsquare);
                    expect(token::becomes);
                    IRNode* expr_right = expression(symtab);
                    return new AssignArrayStat(target, expr_left, expr_right, symtab);
                }

                Symbol* target = symtab->find(name);
                expect(token::becomes);
                IRNode* expr = expression(symtab);
                return new AssignStat(target, expr, symtab);
                break;
            }

            case token::callsym:
            {
                int tok = token::callsym;
                cout << "accepting call" << token::callsym << " == " << CurTok << endl;
                getNextToken();
                string fname = lex.Identifier();
                expect(token::identifier);

                vector<IRNode*>* parameters = new vector<IRNode*>();

                do {
                    name = lex.Identifier();
                    value = lex.Value();
                    //TODO if argument is an expression?
                    if (accept(token::identifier)) {
                        parameters->push_back(new Var(symtab->find(name), symtab));

                    } else if (accept(token::number)) {

                        parameters->push_back(new Const(value, symtab));
                    }

                } while (accept(token::comma));


                return new CallStat(new CallExpr(symtab->find(fname), tok, parameters, symtab), symtab);
                break;
            }

            case token::beginsym:
            {
                cout << "accepting begin" << token::beginsym << " == " << CurTok << endl;
                getNextToken();
                StatList* statement_list = new StatList(symtab);
                do {
                    statement_list->append(statement(symtab));
                } while (accept(token::semicolon));
                expect(token::endsym);
                statement_list->printContent();
                return statement_list;
                break;
            }
            case token::ifsym:
            {
                cout << "accepting " << token::ifsym << " == " << CurTok << endl;
                getNextToken();
                IRNode* cond = condition(symtab);
                expect(token::thensym);
                IRNode* then = statement(symtab);
                return new IfStat(cond, then, symtab);
                break;
            }

            case token::whilesym:
            {
                cout << "accepting while " << token::whilesym << " == " << CurTok << endl;
                getNextToken();
                IRNode *cond = condition(symtab);
                expect(token::dosym);
                IRNode* body = statement(symtab);
                return new WhileStat(cond, body, symtab);
                break;
            }
            case token::print:
            {
                cout << "accepting print" << token::print << " == " << CurTok << endl;
                getNextToken();
                name = lex.Identifier();
                expect(token::identifier);
                return new PrintStat(symtab->find(name), symtab);
                break;
            }
            default:
            {
                error("statement : syntax error");
                getNextToken();
                break;
            }

        }



    }

    IRNode* expression(SymbolTable* symtab) {

        int op = 0;

        if (CurTok == token::plus || CurTok == token::minus) {
            op = CurTok;
            getNextToken();
        }
        IRNode* expr = term(symtab);

        if (op)
            expr = new UnExpr(op, expr, symtab);

        while (CurTok == token::plus || CurTok == token::minus) {
            op = CurTok;
            getNextToken();
            IRNode* expr2 = term(symtab);
            vector<IRNode*>* children = new vector<IRNode*>();
            children->push_back(new Tok(op, symtab));
            children->push_back(expr);
            children->push_back(expr2);
            expr = new BinExpr(op, children, symtab);
        }

        return expr;
    }

    IRNode* term(SymbolTable* symtab) {
        IRNode* expr = factor(symtab);
        int op;
        while (CurTok == token::times || CurTok == token::slash) {
            op = CurTok;
            getNextToken();
            IRNode* expr2 = factor(symtab);
            vector<IRNode*>* children = new vector<IRNode*>();
            children->push_back(new Tok(op, symtab)); //TODO review the name Tok
            children->push_back(expr);
            children->push_back(expr2);
            expr = new BinExpr(op, children, symtab);
        }
        return expr;
    }

    IRNode* factor(SymbolTable *symtab) {
        string id = lex.Identifier();
        value = lex.Value();
        if (accept(token::identifier)) {

            if (accept(token::lsquare)) {
                //TODO review expr_left, could be dangerous
                IRNode* index = expression(symtab);
                expect(token::rsquare);
                return new ArrayVar(symtab->find(id), index, symtab);
            }

            return new Var(symtab->find(id), symtab);

        } else if (accept(token::number)) {
            return new Const(value, symtab);

        } else if (accept(token::lparen)) {
            IRNode *expr = expression(symtab);
            expect(token::rparen);
            return expr;
        } else {
            error("factor: syntax error");
            getNextToken();
        }
    }

    IRNode* condition(SymbolTable* symtab) {

        if (accept(token::oddsym)) {
            return new UnExpr(token::oddsym, expression(symtab), symtab);
        } else {
            IRNode* expr = expression(symtab);
            if (CurTok == token::eql || CurTok == token::neq ||
                    CurTok == token::lss || CurTok == token::leq ||
                    CurTok == token::gtr || CurTok == token::geq) {
                int op = CurTok;
                getNextToken();
                IRNode* expr2 = expression(symtab);
                vector<IRNode*>* children = new vector<IRNode*>();
                children->push_back(new Tok(op, symtab));
                children->push_back(expr);
                children->push_back(expr2);
                return new BinExpr(op, children, symtab);
            } else {
                error("condition: invalid operator");
                getNextToken();
            }
        }


    }




};



#endif	/* FRONTEND_H */

