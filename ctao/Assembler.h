#ifndef ASSEMBLER_H
#define	ASSEMBLER_H

#include "registeralloc.h"

#define MAX_NUM_ARGUMENTS 4


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

				Symbol* label = static_cast<Stat*> (*it2)->getLabel();

				if((*it2)->NodeType() == "AssignStat"){
					genAssign(*it2, current, label);
				}

				if((*it2)->NodeType() == "Branch"){
					genBranch(*it2, current, label);
				}

				if((*it2)->NodeType() == "CallStat"){
					genCall(*it2, current, label);
				}

				if((*it2)->NodeType() == "Load"){
					genLoad(*it2, current, label);
				}

				if((*it2)->NodeType() == "StoreStat"){
					genStore(*it2, current, label);
				}

				if((*it2)->NodeType() == "Stat"){
					genEmpty(*it2, current, label);
				}
			}

		}
	}

	~CodeGenerator(){
		for(std::list<FunctionCode*>::iterator it = functions.begin(); it != functions.end(); ++it){
			delete *it;
		}
	}

private:

	void genEmpty(IRNode* stat, FunctionCode* current, Symbol* label){
		string label_name;

		if(label){
			label_name = label->getName() + ": ";
		}

		cout << label_name + "NOP" << endl;

		current->insertCode(label_name + "NOP" );
	}

	void genStore(IRNode* stat, FunctionCode* current, Symbol* label){
		
		string r1;
		string r2;
		string r3;

		string label_name;

		if(label){
			label_name = label->getName() + ": ";
		}

		StoreStat* store = static_cast<StoreStat*>(stat);

		Symbol* reg = store->getSymbol();

		if(!reg->isSpilled()){
			reg = regalloc.mapVarReg()[store->getSymbol()];
		}

		r1 = reg->getName();

		if(store->getChildren()->at(0)->NodeType() == "BinExpr"){
		}
		else{
			r2 = TermCode(store->getChildren()->at(0));
			r3 = "#0";
		}
		
		cout <<label_name + "STORE " + r1 + " " + r2 + " " + r3 << endl; 

		current->insertCode(label_name + "STORE " + r1 + " " + r2 + " " + r3);
	}

	void genLoad(IRNode* stat, FunctionCode* current, Symbol* label){

		string r1;
		string r2;
		string r3;

		string label_name;

		if(label){
			label_name = label->getName() + ": ";
		}

		LoadStat* load = static_cast<LoadStat*>(stat);

		Symbol* reg = regalloc.mapVarReg()[load->getSymbol()];
		r1 = reg->getName();

		if(load->getChildren()->at(0)->NodeType() == "BinExpr"){
		}
		else{
			r2 = TermCode(load->getChildren()->at(0));
			r3 = "#0";
		}
		
		cout << "LOAD " + r1 + " " + r2 + " " + r3 << endl; 

		current->insertCode(label_name + "LOAD " + r1 + " " + r2 + " " + r3);
	}

	void genPush(IRNode* stat, FunctionCode* current){
	}

	void genPop(IRNode* stat, FunctionCode* current){
	}

	void genCall(IRNode* stat, FunctionCode* current, Symbol* label){

		string label_name;

		if(label){
			label_name = label->getName() + ": ";
		}

		string op = "BL";
		string call_name;

		CallStat* call = static_cast<CallStat*>(stat);

		CallExpr* call_expr = static_cast<CallExpr*>(call->getChildren()->at(0));

		Symbol* call_sym = call_expr->getSymbol();

		call_name = call_sym->getName();

		string push;
		string pop;
		vector<string> movs;

		for(std::list<IRNode*>::size_type i = 0; i < call_expr->getChildren()->size(); ++i){
			if(i < MAX_NUM_ARGUMENTS){
				IRNode* argument = call_expr->getChildren()->at(i);
				ostringstream convert;
				string mov;

				convert << i;

				if(!i){
					push = label_name +  "PUSH { R" + convert.str();
				}
				else{
					push += ", R" + convert.str();
				}

				mov = "MOV R"+ convert.str();
				mov += " ";
				mov += TermCode(argument);

				movs.push_back(mov);

				mov.clear();
			}

			else 
				cout << "more than 4 arguments" << endl;
		}

		if(push.size()){
			push += " }";

			cout << push << endl;

			current->insertCode(push);
		}
		else{
			call_name = label_name + call_name;
		}

		for(vector<string>::iterator it = movs.begin(); it != movs.end(); ++it){
			cout << *it << endl;
			current->insertCode(*it);
		}

		cout << op + " " + call_name << endl;

		current->insertCode(op + " " + call_name);

		for(std::list<IRNode*>::size_type i = 0; i < call_expr->getChildren()->size(); ++i){
			if(i < MAX_NUM_ARGUMENTS){
				IRNode* argument = call_expr->getChildren()->at(i);
				ostringstream convert;
				string mov;

				convert << i;

				if(!i){
					pop = "POP { R" + convert.str();
				}
				else{
					pop += ", R" + convert.str();
				}
			}
		}

		if(pop.size()){
			pop += " }";

			cout << pop << endl;

			current->insertCode(pop);
		}
	}

	void genCmp(IRNode* stat, FunctionCode* current, Symbol* label){

		string label_name;

		if(label){
			label_name = label->getName() + ": ";
		}

		BranchStat* branch = static_cast<BranchStat*>(stat);

		string op = "CMP";

		string r1 = TermCode(branch->getChildren()->at(0)->getChildren()->at(1));

		string r2 = TermCode(branch->getChildren()->at(0)->getChildren()->at(2));

		cout << label_name + op + " " + r1 + " " + r2 << endl;

		current->insertCode(label_name + op + " " + r1 + " " + r2 );
	}

	void genBranch(IRNode* stat, FunctionCode* current, Symbol* label_stat){

		string op;

		BranchStat* branch = static_cast<BranchStat*>(stat);

		Symbol* label_sym = branch->getSymbol();

		string label = label_sym->getName();

		if(branch->isUnconditional()){
			cout << "B " + label << endl;
			current->insertCode("B " + label);
		}
		else{
			genCmp(branch, current, label_stat);

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

	void genAssign(IRNode* stat, FunctionCode* current, Symbol* label){

		string op;
		string r1;
		string r2;
		string r3;

		string label_name;

		if(label){
			label_name = label->getName() + ": ";
		}

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

				cout << label_name + op + " " + r1 + " " + r2 + " " + r3 << endl;

				current->insertCode(label_name + op + " " + r1 + " " + r2 + " " + r3); 
			}else{
				cout << label_name + op + " " + r1 + " " + r2 << endl;

				current->insertCode(label_name + op + " " + r1 + " " + r2); 
			}



		}
		else{

			op = "MOV";

			r2 = TermCode(expr);

			cout << label_name + op + " " + r1 + " " + r2 << endl;

			current->insertCode(label_name + op + " " + r1 + " " + r2);
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

			if(sym->isSpilled())
				return sym->getName();

			Symbol* reg = regalloc.mapVarReg()[sym];
			return reg->getName();
		}
	}

	std::list<FunctionCode*> functions;

	CFG& cfg;
	RegisterAlloc& regalloc;
};



#endif