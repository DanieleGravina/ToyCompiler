#ifndef ASSEMBLER_H
#define	ASSEMBLER_H

#include "registeralloc.h"


class FunctionCode{
public:
	FunctionCode(Symbol* _function_sym = NULL):function_sym(_function_sym){}

	void insertCode(string generated_code){
		code.push_back(generated_code);
	}

private:
	Symbol* function_sym;
	list<string> code;
};


/**
*	Code Generator target ARM
*/
class CodeGenerator{
public:
	CodeGenerator(CFG& _cfg, RegisterAlloc& _regalloc): cfg(_cfg), regalloc(_regalloc){

		Symbol* cursym = new Symbol("global");
		FunctionCode* current = new FunctionCode(cursym);

		functions.push_front(current);

		for (list<BasicBlock*>::iterator it = cfg.begin(); it != cfg.end(); ++it) {

			if((*it)->getSymOfFunction() && (*it)->getSymOfFunction() != cursym){
				cursym = (*it)->getSymOfFunction();
				current = new FunctionCode((*it)->getSymOfFunction());
				functions.push_front(current);
			}

			list<IRNode*> stats = (*it)->getStats();

			for(list<IRNode*>::iterator it2 = stats.begin(); it2 != stats.end(); ++it2){

				if((*it2)->NodeType() == "AssignStat"){
					genAssign(*it2, current);
				}

				if((*it2)->NodeType() == "Branch"){
					genBranch(*it2, current);
				}

				if((*it2)->NodeType() == "CallStat"){
					genCall(*it2, current);
				}

				if((*it2)->NodeType() == "Load"){
					genLoad(*it2, current);
				}

				if((*it2)->NodeType() == "StoreStat"){
					genStore(*it2, current);
				}
			}

		}
	}

private:

	void genStore(IRNode* stat, FunctionCode* current){
		cout << "store" << endl;
	}

	void genLoad(IRNode* stat, FunctionCode* current){
		cout << "load" << endl;
	}

	void genCall(IRNode* stat, FunctionCode* current){
		cout << "BL square" << endl;
	}

	void genCmp(IRNode* stat, FunctionCode* current){

		BranchStat* branch = static_cast<BranchStat*>(stat);

		Symbol* label_sym = branch->getLabel();

		string label = label_sym->getName();

		string op = "CMP";

		string r1 = TermCode(branch->getChildren()->at(0)->getChildren()->at(1));

		string r2 = TermCode(branch->getChildren()->at(0)->getChildren()->at(2));

		cout << label + " : " + op + " " + r1 + " " + r2 << endl;

		current->insertCode(label + " : " + op + " " + r1 + " " + r2 );
	}

	void genBranch(IRNode* stat, FunctionCode* current){

		string op;

		BranchStat* branch = static_cast<BranchStat*>(stat);

		Symbol* label_sym = branch->getSymbol();

		string label = label_sym->getName();

		if(branch->isUnconditional()){
			cout << "B " + label << endl;
			current->insertCode("B " + label);
		}
		else{
			genCmp(branch, current);

			IRNode* oper = static_cast<Expr*>(branch->getChildren()->at(0))->getOperator();

			switch(static_cast<Tok*>(oper)->getOP()){
           
			case token::leq :
				op = "BLE";
				break;
			case token::geq :
				op = "BGE";
				break;
			case token::gtr :
				op = "BGT";
				break;
			case token::lss :
				op = "BLT";
				break;
			case token::eql :
				op = "BEQ";
				break;
			case token::neq :
				op = "BNQ";
				break;
			}

			cout << op + " " + label << endl;
			current->insertCode(op + " " + label );

		}

	}

	void genAssign(IRNode* stat, FunctionCode* current){

		string op;
		string r1;
		string r2;
		string r3;

		IRNode* expr = expr = stat->getChildren()->at(0);

		Symbol* sym = static_cast<AssignStat*>(stat)->getSymbol();

		Symbol* reg = regalloc.mapVarReg()[sym];

		r1 = reg->getName();

		if(expr->NodeType() == "BinExpr" || expr->NodeType() == "UnExpr"){

			IRNode* oper = static_cast<Expr*>(expr)->getOperator();

			switch(static_cast<Tok*>(oper)->getOP()){
			case token::plus :
				op = "ADD";
				break;
			case token::minus :
				op = "SUB";
				break;
			case token::slash :
				op = "DIV";
				break;
			case token::times :
				op = "MUL";
				break;
			}

			r2 = TermCode(expr->getChildren()->at(1));

			if(expr->NodeType() == "BinExpr"){
				r3 = TermCode(expr->getChildren()->at(2));

				cout << op + " " + r1 + " " + r2 + " " + r3 << endl;

				current->insertCode(op + " " + r1 + " " + r2 + " " + r3); 
			}else{
				cout << op + " " + r1 + " " + r2 << endl;

				current->insertCode(op + " " + r1 + " " + r2); 
			}



		}
		else{

			op = "MOV";

			r2 = TermCode(expr);

			cout << op + " " + r1 + " " + r2 << endl;

			current->insertCode(op + " " + r1 + " " + r2);
		}

	}

	string TermCode(IRNode* expr){

		ostringstream convert;

		if(expr->NodeType() == "Const"){
			Value val = static_cast<Const*>(expr)->getValue();
			convert << "#" << val.getValue();
			return convert.str();
		}
		else{
			Symbol* sym = static_cast<Var*>(expr)->getSymbol();
			Symbol* reg = regalloc.mapVarReg()[sym];
			return reg->getName();
		}
	}

	std::list<FunctionCode*> functions;

	CFG& cfg;
	RegisterAlloc& regalloc;
};



#endif