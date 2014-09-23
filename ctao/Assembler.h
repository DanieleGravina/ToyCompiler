#ifndef ASSEMBLER_H
#define	ASSEMBLER_H

#include "registeralloc.h"
#include <fstream>
#include <string>
#include <iostream>

#define MAX_NUM_ARGUMENTS 4
#define OFFSET 4

class Register{

public:

	enum AuxiliaryRegister{
		r0 = 0,
		r1 = 1,
		r2 = 2,
		r3 = 3,
		r4 = 4,
		r5 = 5,
		r6 = 6,
		r7 = 7,
		r8 = 8,
		sp = 10,
		fp = 11,
		lr = 12
	};

private:
	Register();
};

class Instruction{

public:

	static string convert(int imm){
		ostringstream convert;

		convert << "#" << imm;

		return convert.str();
	}

	static string store(Symbol* first, Symbol* second, Symbol *third){
		return "str " + first->getName() + " " + second->getName() + " " + third->getName();
	}

	static string store(Symbol* first, Symbol* second, int imm){
		return "str " + first->getName() + " " + second->getName() + " " + convert(imm);
	}

	static string load(Symbol* first, Symbol* second, Symbol *third){
		return "ldr " + first->getName() + " " + second->getName() + " " + third->getName();
	}

	static string load(Symbol* first, Symbol* second, int imm){
		return "ldr " + first->getName() + " " + second->getName() + " " + convert(imm);
	}

	static string nop(){
		return "nop ";
	}

	static string mov(Symbol* first, Symbol* second){
		return "mov " + first->getName() + " " + second->getName();
	}

	static string sub(Symbol* first, Symbol* second, Symbol *third){

	}

	static string sub(Symbol* first, Symbol* second, int imm){

		return "sub " + first->getName() + " " + second->getName() + " " + convert(imm);
	}

	static string push(std::list<Symbol*>& to_push){

		string reg_push = "push { ";

		reg_push += to_push.front()->getName();

		to_push.pop_front();

		while(to_push.size()){
			reg_push += " ," + to_push.front()->getName();

			to_push.pop_front();
		}

		reg_push += " }";

		return reg_push;

	}

	static string pop(std::list<Symbol*>& to_pop){

		string reg_pop = "pop { ";

		reg_pop += to_pop.front()->getName();

		to_pop.pop_front();

		while(to_pop.size()){
			reg_pop += " ," + to_pop.front()->getName();

			to_pop.pop_front();
		}

		reg_pop += " }";

		return reg_pop;
	}

private:
	Instruction();
};

class FunctionCode {
public:

	FunctionCode(Symbol* _function_sym = NULL) : function_sym(_function_sym), offset(0) {
	}

	void insertRegAlloc(RegisterAlloc* regalloc){
		reg_alloc = regalloc;
	}

	RegisterAlloc& regalloc(){
		return *reg_alloc;
	}

	void insertCode(string generated_code) {

		code.push_back(generated_code);
	}

	std::map<Symbol*, int>& getSpillOffset() {
		return spill_offset;
	}

	void insertSpilled(Symbol* spilled_sym) {

		spill_offset[spilled_sym] = offset;

		offset += OFFSET;

	}

	void insertLocal(Symbol* local){
		local_offset[local] = offset;

		if(local->getType().getName() == "array")
			offset += local->getType().getSize();
		else
			offset += OFFSET;
	}

	void print() {
		for (std::list<string>::iterator it = code.begin(); it != code.end(); ++it) {
			cout << *it << endl;
		}
	}

	std::list<string> getCode(){
		return code;
	}

	void insertEnd() {
		int counter = 0;

		insertCode("       bx lr");

	}

	Symbol* getFunctionSym() {
		return function_sym;
	}

private:
	int offset;
	Symbol* function_sym;
	list<string> code;
	std::map<Symbol*, int> spill_offset;
	std::map<Symbol*, int> local_offset;
	RegisterAlloc* reg_alloc;
};

/**
*	Code Generator target ARM
*/
class CodeGenerator {
public:

	CodeGenerator(std::map<CFG*, RegisterAlloc*>& _procedures) : procedures(_procedures), aux(20), mainCode(NULL) {


		aux[Register::sp] = new Symbol("sp");
		aux[Register::fp] = new Symbol("fp");
		aux[Register::lr] = new Symbol("lr");
		aux[Register::r0] = new Symbol("r0");
		aux[Register::r1] = new Symbol("r1");
		aux[Register::r2] = new Symbol("r2");
		aux[Register::r3] = new Symbol("r3");
		aux[Register::r4] = new Symbol("r4");
		aux[Register::r5] = new Symbol("r5");
		aux[Register::r6] = new Symbol("r6");
		aux[Register::r7] = new Symbol("r7");
		aux[Register::r8] = new Symbol("r8");
		mainSym = new Symbol("main");

		Symbol* cursym = NULL;



		for(std::map<CFG*, RegisterAlloc*>::iterator it = procedures.begin(); it != procedures.end(); ++it){

			if(!(*it->first->begin())->getFunction()) //main
				cursym = mainSym;
			else
				cursym = static_cast<FunctionDef*>((*it->first->begin())->getFunction())->getSymbol();

			FunctionCode* current = new FunctionCode(cursym);

			if(cursym == mainSym)
				mainCode = current; //main has to be inserted as last one
			else
				functions.push_front(current);

			addInitialDeclaration(current, (*(it->first->begin()))->getSymTab());

			current->insertRegAlloc(it->second);

			//save lr, sp + calle save register

			std::list<Symbol*> to_push;

			to_push.push_back(aux[Register::fp]);
			to_push.push_back(aux[Register::lr]);

			std::set<Symbol*> vars = it->second->Vars();
			SymbolTable* params = it->second->getParameters();

			/*int calle_save = 4;

			while(calle_save < it->second->num_reg()){
			to_push.push_back(aux[calle_save]);
			calle_save++;
			}*/

			current->insertCode("       " + Instruction::push(to_push));

			current->insertCode("       " + Instruction::mov(aux[Register::fp], aux[Register::sp]));

			int allocSpace = 0;

			SymbolTable* symtab = (*(it->first->begin()))->getSymTab();

			std::list<Symbol*> spilled_parameters;

			for (SymbolTable::iterator it2 = symtab->begin(); it2 != symtab->end(); ++it2) {
				if(!(it2->second)->isGlobal() && (it2->second->getType().getName() != "Function" && it2->second->getType().getName() != "Label")){
					if((it2->second)->getType().getName() == "array"){
						int size = (it2->second)->getType().getSize();
						allocSpace += size;
						for(CFG::iterator it3 = it->first->begin(); it3 != it->first->end(); ++it3) 
							(*it3)->mapVarToReg()[it2->second] = aux[Register::fp]; 
					}
					else{
						if(params && params->find((it2->second)->getName()) != NULL && (it2->second)->isSpilled() ){ //there are parameter spilled?
							allocSpace += OFFSET;
							spilled_parameters.push_back(it2->second);
						}
					}

					current->insertLocal(it2->second);
				}
			}

			//alloc stack memory for local vars and local arrays
			if(allocSpace)
				current->insertCode("       " + Instruction::sub(aux[Register::sp], aux[Register::sp], allocSpace)); //space for local variables

			//save to the stack position parameters spilled passed by register location
			int indexReg = 0;
			for(std::list<Symbol*>::iterator it2 = spilled_parameters.begin(); it2 != spilled_parameters.end(); ++it2, ++indexReg)
				current->insertCode("       " + Instruction::store(aux[indexReg], aux[Register::fp], current->getSpillOffset()[*it2]));



			for (std::list<BasicBlock*>::iterator it2 = it->first->begin(); it2 != it->first->end(); ++it2) {

				std::list<IRNode*> stats = (*it2)->getStats();

				currentBB = *it2;

				for (list<IRNode*>::iterator it3 = stats.begin(); it3 != stats.end(); ++it3) {

					Symbol* label = static_cast<Stat*> (*it3)->getLabel();

					if ((*it3)->NodeType() == "AssignStat") {
						genAssign(*it3, current, label);
					}

					if ((*it3)->NodeType() == "Branch") {
						genBranch(*it3, current, label);
					}

					if ((*it3)->NodeType() == "CallStat") {
						genCall(*it3, current, label);
					}

					if ((*it3)->NodeType() == "Load") {
						genLoad(*it3, current, label);
					}

					if ((*it3)->NodeType() == "StoreStat") {
						genStore(*it3, current, label);
					}

					if ((*it3)->NodeType() == "Stat") {
						genEmpty(*it3, current, label);
					}
				}
			}

			//restore lr, sp + calle save register

			std::list<Symbol*> to_pop;

			to_pop.push_back(aux[Register::fp]);
			to_pop.push_back(aux[Register::lr]);

			/*calle_save = 4;

			while(calle_save < it->second->num_reg()){
			to_pop.push_back(aux[calle_save]);
			calle_save++;
			}*/

			//restore from stack position parameters spilled passed by register location
			indexReg = 0;
			for(std::list<Symbol*>::iterator it2 = spilled_parameters.begin(); it2 != spilled_parameters.end(); ++it2, ++indexReg)
				current->insertCode("       " + Instruction::load(aux[indexReg], aux[Register::fp], current->getSpillOffset()[*it2]));

			//restore sp, fp, and lr
			current->insertCode("       " + Instruction::mov(aux[Register::fp], aux[Register::sp]));
			current->insertCode("       " + Instruction::pop(to_pop));

			cout << endl;

		}

		functions.push_back(mainCode);

		std::ofstream out("output.txt");

		for (std::list<FunctionCode*>::iterator it = functions.begin(); it != functions.end(); ++it) {
			(*it)->insertEnd();

			if(out.is_open()){
				std::list<string> code = (*it)->getCode();
				for (std::list<string>::iterator it2 = code.begin(); it2 != code.end(); ++it2) {
					out << (*it2) << endl;
				}

				out << endl;
			}
			else{
				(*it)->print();
				cout << endl;
			}
			
		}

		out.close();
	}

	~CodeGenerator() {
		for (std::list<FunctionCode*>::iterator it = functions.begin(); it != functions.end(); ++it) {
			delete *it;
		}

		delete mainSym;

		for (std::vector<Symbol*>::iterator it = aux.begin(); it != aux.end(); ++it) {
			delete *it;
		}

	}

private:

	void addInitialDeclaration(FunctionCode *current, SymbolTable* symtab){

		ostringstream convert;

		convert << 0;

		string num = convert.str();

		current->insertCode(".data");


		if(current->getFunctionSym() == mainSym){
			for (SymbolTable::iterator it = symtab->begin(); it != symtab->end(); ++it) {
				if((it->second)->isGlobal() && (it->second->getType().getName() != "Function" && it->second->getType().getName() != "Label")){
					if((it->second)->getType().getName() == "array"){
						current->insertCode(it->first + ": ");
						for(int i = 0; i < (it->second)->getType().getSize(); ++i){
							current->insertCode("       .int " + num);
						}
					}
					else{
						current->insertCode(it->first + ": .int " + num); 
					}
				}

			}
		}

		current->insertCode(".text");
		current->insertCode(".global " + current->getFunctionSym()->getName());
		current->insertCode(".type " + current->getFunctionSym()->getName() + ", %function");
		current->insertCode(current->getFunctionSym()->getName() + ":");

	}

	void genEmpty(IRNode* stat, FunctionCode* current, Symbol* label) {
		string label_name;

		if (label) {
			label_name = label->getName() + ": ";
		}
		else{
			label_name = "       ";
		}

		current->insertCode(label_name + Instruction::nop());
	}

	void genStore(IRNode* stat, FunctionCode* current, Symbol* label) {

		Symbol *r1, *r2, *r3 = NULL;

		int imm;

		string label_name;

		if (label) {
			label_name = label->getName() + ": ";
		}
		else{
			label_name = "       ";
		}

		StoreStat* store = static_cast<StoreStat*> (stat);

		Symbol* var = store->getSymbol();

		if (currentBB->getSpilled().find(var) != currentBB->getSpilled().end()) {

			if(var->isGlobal()){
				r3 = var;
				r2 = static_cast<Var*>(store->getChildren()->at(0))->getSymbol();
				r2 = currentBB->mapVarToReg()[r2];
				r1 = static_cast<Var*>(store->getChildren()->at(1))->getSymbol();
				r1 = currentBB->mapVarToReg()[r1];
			}
			else{
				imm = current->getSpillOffset()[var];
				r2 = aux[Register::fp];
				r1 = static_cast<Var*>(store->getChildren()->at(1))->getSymbol();
				r1 = currentBB->mapVarToReg()[r1];
			}

		} else {
			r1 = currentBB->mapVarToReg()[store->getSymbol()];
			imm = 0;

			if (store->getChildren()->at(0)->NodeType() == "BinExpr") {
			} else {
				r3 = (static_cast<Var*>(store->getChildren()->at(1)))->getSymbol();
				if(!store->isFinal()){
					r3 = currentBB->mapVarToReg()[r3];
				}
				r2 = (static_cast<Var*>(store->getChildren()->at(0)))->getSymbol();
				r2 = currentBB->mapVarToReg()[r2];
			}
		}

		if(r3)
			current->insertCode(label_name + Instruction::store(r1, r2, r3));
		else
			current->insertCode(label_name + Instruction::store(r1, r2, imm));

	}

	void genLoad(IRNode* stat, FunctionCode* current, Symbol* label) {

		Symbol *r1, *r2, *r3 = NULL;
		int imm = 0;

		string label_name;

		if (label) {
			label_name = label->getName() + ": ";
		}
		else{
			label_name = "       ";
		}

		LoadStat* load = static_cast<LoadStat*> (stat);

		r1 = currentBB->mapVarToReg()[load->getSymbol()];

		Symbol* var = static_cast<Var*>(load->getChildren()->at(1))->getSymbol();

		if (currentBB->getSpilled().find(var) != currentBB->getSpilled().end()) {

			if(var->isGlobal()){
				r2 = static_cast<Var*>(load->getChildren()->at(0))->getSymbol();
				r2 = currentBB->mapVarToReg()[r2];
				r3 = static_cast<Var*>(load->getChildren()->at(1))->getSymbol();
			}
			else{
				imm = current->getSpillOffset()[var];
				r2 = aux[Register::fp];
			}

		} else {

			if (load->getChildren()->at(0)->NodeType() == "BinExpr") {
			} else {
				r3 = (static_cast<Var*>(load->getChildren()->at(1)))->getSymbol();
				if(!load->isInitial()){
					r3 = currentBB->mapVarToReg()[r3];
				}
				r2 = (static_cast<Var*>(load->getChildren()->at(0)))->getSymbol();
				r2 = currentBB->mapVarToReg()[r2];
			}
		}

		if(r3)
			current->insertCode(label_name + Instruction::load(r1, r2, r3));
		else
			current->insertCode(label_name + Instruction::load(r1, r2, imm));
	}

	void genCall(IRNode* stat, FunctionCode* current, Symbol* label) {

		string label_name;

		if (label) {
			label_name = label->getName() + ": ";
		}
		else{
			label_name = "       ";
		}

		string op = "bl";
		string call_name;

		CallStat* call = static_cast<CallStat*> (stat);

		CallExpr* call_expr = static_cast<CallExpr*> (call->getChildren()->at(0));

		Symbol* call_sym = call_expr->getSymbol();

		call_name = call_sym->getName();

		string push;
		string pop;
		vector<string> movs;
		int num_args;

		for (std::list<IRNode*>::size_type i = 0; i < call_expr->getChildren()->size(); ++i) {
			if (i < MAX_NUM_ARGUMENTS) {
				IRNode* argument = call_expr->getChildren()->at(i);
				num_args = call_expr->getChildren()->size();
				ostringstream convert;
				string mov;

				convert << i;

				int min;

				if(num_args < MAX_NUM_ARGUMENTS)
					if(num_args < current->regalloc().num_reg())
						min = num_args;
					else
						min = current->regalloc().num_reg();
				else 
					min = MAX_NUM_ARGUMENTS;


				for(int j = 0; j < min; j++){
					ostringstream convert2;
					convert2 << j;
					if (!j) {
						push = label_name + "push { r" + convert2.str();
					} else {
						push += ", r" + convert2.str();
					}
				}

				mov = "mov r" + convert.str();
				mov += " ";
				mov += TermCode(argument, current);

				//very bad solution to eliminate mov rx rx with x == x
				if(mov[5] != mov[8])
					movs.push_back(mov);

				mov.clear();
			}
			else
				cout << "more than 4 arguments" << endl;
		}

		if (push.size()) {
			push += " }";

			current->insertCode(push);
		} else {
			call_name = label_name + call_name;
		}

		for (vector<string>::iterator it = movs.begin(); it != movs.end(); ++it) {
			current->insertCode("       " + *it);
		}

		current->insertCode("       " + op + " " + call_name);

		for (std::list<IRNode*>::size_type i = 0; i < call_expr->getChildren()->size(); ++i) {
			if (i < MAX_NUM_ARGUMENTS) {
				IRNode* argument = call_expr->getChildren()->at(i);
				num_args = call_expr->getChildren()->size();
				ostringstream convert;
				string mov;

				convert << i;

				int min = 0;

				if(num_args < MAX_NUM_ARGUMENTS)
					if(num_args < current->regalloc().num_reg())
						min = num_args;
					else
						min = current->regalloc().num_reg();
				else 
					min = MAX_NUM_ARGUMENTS;

				for(int j = 0; j < min; j++){
					ostringstream convert2;
					convert2 << j;
					if (!j) {
						pop = "pop { r" + convert2.str();
					} else {
						pop += ", r" + convert2.str();
					}
				}
			}
		}

		if (pop.size()) {
			pop += " }";

			current->insertCode(label_name + pop);
		}
	}

	void genCmp(IRNode* stat, FunctionCode* current, Symbol* label) {

		string label_name;

		if (label) {
			label_name = label->getName() + ": ";
		}
		else{
			label_name = "       ";
		}

		BranchStat* branch = static_cast<BranchStat*> (stat);

		string op = "cmp";

		string r1 = TermCode(branch->getChildren()->at(0)->getChildren()->at(1), current);

		string r2 = TermCode(branch->getChildren()->at(0)->getChildren()->at(2), current);

		current->insertCode(label_name + op + " " + r1 + " " + r2);
	}

	void genBranch(IRNode* stat, FunctionCode* current, Symbol* label_stat) {

		string label_name;

		label_name = "       ";

		string op;

		BranchStat* branch = static_cast<BranchStat*> (stat);

		Symbol* label_sym = branch->getSymbol();

		string label = label_sym->getName();

		if (branch->isUnconditional()) {

			current->insertCode(label_name + "b " + label);

		} else {
			genCmp(branch, current, label_stat);

			IRNode* oper = static_cast<Expr*> (branch->getChildren()->at(0))->getOperator();

			switch (static_cast<Tok*> (oper)->getOP()) {

			case token::leq:
				op = "ble";
				break;
			case token::geq:
				op = "bge";
				break;
			case token::gtr:
				op = "bgt";
				break;
			case token::lss:
				op = "blt";
				break;
			case token::eql:
				op = "beq";
				break;
			case token::neq:
				op = "bnq";
				break;
			}

			current->insertCode(label_name + " " + op + " " + label);

		}

	}

	void genAssign(IRNode* stat, FunctionCode* current, Symbol* label) {

		string op;
		string r1;
		string r2;
		string r3;

		string label_name;

		if (label) {
			label_name = label->getName() + ": ";
		}
		else{
			label_name = "       ";
		}

		IRNode* expr = expr = stat->getChildren()->at(0);

		Symbol* sym = static_cast<AssignStat*> (stat)->getSymbol();



		Symbol* reg = currentBB->mapVarToReg()[sym];

		r1 = reg->getName();

		if (expr->NodeType() == "BinExpr" || expr->NodeType() == "UnExpr") {

			IRNode* oper = static_cast<Expr*> (expr)->getOperator();

			switch (static_cast<Tok*> (oper)->getOP()) {
			case token::plus:
				op = "add";
				break;
			case token::minus:
				op = "sub";
				break;
			case token::slash:
				op = "div";
				break;
			case token::times:
				op = "mul";
				break;
			}

			r2 = TermCode(expr->getChildren()->at(1), current);

			if (expr->NodeType() == "BinExpr") {
				r3 = TermCode(expr->getChildren()->at(2), current);

				current->insertCode(label_name + op + " " + r1 + " " + r2 + " " + r3);
			} else {

				current->insertCode(label_name + op + " " + r1 + " " + r2);
			}



		} else {

			op = "mov";

			r2 = TermCode(expr, current);

			current->insertCode(label_name + op + " " + r1 + " " + r2);
		}

	}

	string TermCode(IRNode* expr, FunctionCode* current) {

		ostringstream convert;

		if (expr->NodeType() == "Const") {

			//const is offset of local array?
			Const* c = static_cast<Const*>(expr);
			if(c->isOffset()){
				if(c->symOffset()->isGlobal()){
					convert << "#0";
					return convert.str();
				}
				else{
					int offset = current->getSpillOffset()[c->symOffset()];
					convert << "#" << offset;
					return convert.str();
				}
			}


			Value val = static_cast<Const*> (expr)->getValue();
			convert << "#" << val.getValue();
			return convert.str();
		} else {
			Symbol* sym = static_cast<Var*> (expr)->getSymbol();

			if (currentBB->getSpilled().find(sym) != currentBB->getSpilled().end()) {
				if (sym->isGlobal()) {
					return sym->getName();
				} else {
					string ebs;
					int offset = current->getSpillOffset()[sym];
					convert << "#" << offset;
					ebs = "fp " + convert.str();
					return ebs;
				}
			}

			Symbol* reg = currentBB->mapVarToReg()[sym];
			return reg->getName();
		}
	}

	std::list<FunctionCode*> functions;

	std::map<CFG*, RegisterAlloc*>& procedures;

	std::vector<Symbol*> aux;

	Symbol* mainSym;

	FunctionCode* mainCode;

	BasicBlock* currentBB;

};



#endif