#ifndef ASSEMBLER_H
#define	ASSEMBLER_H

#include "registeralloc.h"

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

	void insertEnd() {
		int counter = 0;
		for (std::map<Symbol*, int>::iterator it = spill_offset.begin(); it != spill_offset.end(); ++it, ++counter) {
			ostringstream convert;
			convert << "POP " << "{ R" << counter << "}";
			insertCode(convert.str());
		}

		insertCode("BX LR");

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

	CodeGenerator(std::map<CFG*, RegisterAlloc*>& _procedures) : procedures(_procedures), aux(20) {

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

			if(!cursym)
				cursym = mainSym;
			else
				cursym = static_cast<FunctionDef*>((*it->first->begin())->getFunction())->getSymbol();

			FunctionCode* current = new FunctionCode(cursym);
			functions.push_front(current);

			current->insertRegAlloc(it->second);

			//save lr, sp + calle save register

			std::list<Symbol*> to_push;

			to_push.push_back(aux[Register::fp]);
			to_push.push_back(aux[Register::lr]);

			std::set<Symbol*> vars = it->second->Vars();
			SymbolTable* params = it->second->getParameters();

			int calle_save = 4;

			for (std::set<Symbol*>::iterator it2 = vars.begin(); it2 != vars.end(); ++it2) {
				if(current->getFunctionSym() != mainSym && !(*it2)->isSpilled()){
					if(params && !params->find((*it2)->getName()) ){
						to_push.push_back(aux[calle_save]);
						calle_save++;
					}
				}
			}

			current->insertCode(Instruction::push(to_push));

			current->insertCode(Instruction::mov(aux[Register::fp], aux[Register::sp]));

			int numLocalVariable = 0;

			for (std::set<Symbol*>::iterator it2 = vars.begin(); it2 != vars.end(); ++it2) {
				if(!(*it2)->isGlobal()){
					if((*it2)->getType().getName() == "array"){
						int size = (*it2)->getType().getSize();
						current->insertCode(Instruction::sub(aux[Register::sp], aux[Register::sp], size) ); //space for local array
					}
					else{
						if(params && !params->find((*it2)->getName()) && (*it2)->isSpilled() )
							numLocalVariable++;
					}

					current->insertLocal(*it2);
				}
			}

			if(numLocalVariable)
				current->insertCode(Instruction::sub(aux[Register::sp], aux[Register::sp], OFFSET*numLocalVariable)); //space for local variables


			for (std::list<BasicBlock*>::iterator it2 = it->first->begin(); it2 != it->first->end(); ++it2) {

				std::list<IRNode*> stats = (*it2)->getStats();

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

			calle_save = 4;

			for (std::set<Symbol*>::iterator it = vars.begin(); it != vars.end(); ++it) {
				if(current->getFunctionSym() != mainSym && !(*it)->isSpilled()){
					if(params && !params->find((*it)->getName()) ){
						to_pop.push_back(aux[calle_save]);
						calle_save++;
					}
				}
			}

			current->insertCode(Instruction::mov(aux[Register::fp], aux[Register::sp]));
			current->insertCode(Instruction::pop(to_pop));

			cout << endl;

		}



		for (std::list<FunctionCode*>::iterator it = functions.begin(); it != functions.end(); ++it) {
			(*it)->insertEnd();

			cout << (*it)->getFunctionSym()->getName() << " : " << endl;

			(*it)->print();

			cout << endl;
		}
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

	void genEmpty(IRNode* stat, FunctionCode* current, Symbol* label) {
		string label_name;

		if (label) {
			label_name = label->getName() + ": ";
		}

		cout << label_name + "NOP" << endl;

		current->insertCode(label_name + "NOP");
	}

	void genStore(IRNode* stat, FunctionCode* current, Symbol* label) {

		string r1;
		string r2;
		string r3;

		string label_name;

		if (label) {
			label_name = label->getName() + ": ";
		}

		StoreStat* store = static_cast<StoreStat*> (stat);

		Symbol* reg = store->getSymbol();

		if (reg->isSpilled()) {

			if(reg->isGlobal()){
				IRNode *expr = store->getChildren()->at(0);

				if(expr->NodeType() == "Var" && static_cast<Var*>(expr)->getSymbol()->isGlobal()){
					Symbol* zero = current->regalloc().mapVarReg()[static_cast<Var*>(expr)->getSymTab()->find("zero")];
					r2 = zero->getName();
				}
				r3 = reg->getName();
			}
			else{
				ostringstream convert;
				int offset = current->getSpillOffset()[reg];
				convert << "#" << offset;
				r2 = "EBS " + convert.str();
			}
		} else {
			reg = current->regalloc().mapVarReg()[store->getSymbol()];
			r2 = reg->getName();
		}

		if (store->getChildren()->at(0)->NodeType() == "BinExpr") {
		} else {
			r1 = TermCode(store->getChildren()->at(0), current);
		}

		cout << label_name + "STR " + r1 + " " + r2 + " " + r3 << endl;

		current->insertCode(label_name + "STR " + r1 + " " + r2 + " " + r3);
	}

	void genLoad(IRNode* stat, FunctionCode* current, Symbol* label) {

		string r1;
		string r2;
		string r3;

		string label_name;

		if (label) {
			label_name = label->getName() + ": ";
		}

		LoadStat* load = static_cast<LoadStat*> (stat);

		Symbol* reg = current->regalloc().mapVarReg()[load->getSymbol()];

		r1 = reg->getName();

		if (load->getChildren()->at(0)->NodeType() == "BinExpr") {
		} else {

			IRNode *expr = load->getChildren()->at(0);

			if(expr->NodeType() == "Var" && static_cast<Var*>(expr)->getSymbol()->isGlobal()){
				Symbol* zero = current->regalloc().mapVarReg()[static_cast<Var*>(expr)->getSymTab()->find("zero")];
				r3 = zero->getName();
			}
			r2 = TermCode(load->getChildren()->at(0), current);
		}

		cout << "LDR " + r1 + " " + r2 + " " + r3 << endl;

		current->insertCode(label_name + "LDR " + r1 + " " + r2 + " " + r3);
	}

	void genPush(IRNode* stat, FunctionCode* current) {

		if (!stat) {
			cout << "PUSH {#0}" << endl;
			current->insertCode("PUSH {#0}");
		}
	}

	void genPop(IRNode* stat, FunctionCode* current) {
	}

	void genCall(IRNode* stat, FunctionCode* current, Symbol* label) {

		string label_name;

		if (label) {
			label_name = label->getName() + ": ";
		}

		string op = "BL";
		string call_name;

		CallStat* call = static_cast<CallStat*> (stat);

		CallExpr* call_expr = static_cast<CallExpr*> (call->getChildren()->at(0));

		Symbol* call_sym = call_expr->getSymbol();

		call_name = call_sym->getName();

		string push;
		string pop;
		vector<string> movs;

		for (std::list<IRNode*>::size_type i = 0; i < call_expr->getChildren()->size(); ++i) {
			if (i < MAX_NUM_ARGUMENTS) {
				IRNode* argument = call_expr->getChildren()->at(i);
				ostringstream convert;
				string mov;

				convert << i;

				if (!i) {
					push = label_name + "PUSH { R" + convert.str();
				} else {
					push += ", R" + convert.str();
				}

				mov = "MOV R" + convert.str();
				mov += " ";
				mov += TermCode(argument, current);

				movs.push_back(mov);

				mov.clear();
			}
			else
				cout << "more than 4 arguments" << endl;
		}

		if (push.size()) {
			push += " }";

			cout << push << endl;

			current->insertCode(push);
		} else {
			call_name = label_name + call_name;
		}

		for (vector<string>::iterator it = movs.begin(); it != movs.end(); ++it) {
			cout << *it << endl;
			current->insertCode(*it);
		}

		cout << op + " " + call_name << endl;

		current->insertCode(op + " " + call_name);

		for (std::list<IRNode*>::size_type i = 0; i < call_expr->getChildren()->size(); ++i) {
			if (i < MAX_NUM_ARGUMENTS) {
				IRNode* argument = call_expr->getChildren()->at(i);
				ostringstream convert;
				string mov;

				convert << i;

				if (!i) {
					pop = "POP { R" + convert.str();
				} else {
					pop += ", R" + convert.str();
				}
			}
		}

		if (pop.size()) {
			pop += " }";

			cout << pop << endl;

			current->insertCode(pop);
		}
	}

	void genCmp(IRNode* stat, FunctionCode* current, Symbol* label) {

		string label_name;

		if (label) {
			label_name = label->getName() + ": ";
		}

		BranchStat* branch = static_cast<BranchStat*> (stat);

		string op = "CMP";

		string r1 = TermCode(branch->getChildren()->at(0)->getChildren()->at(1), current);

		string r2 = TermCode(branch->getChildren()->at(0)->getChildren()->at(2), current);

		cout << label_name + op + " " + r1 + " " + r2 << endl;

		current->insertCode(label_name + op + " " + r1 + " " + r2);
	}

	void genBranch(IRNode* stat, FunctionCode* current, Symbol* label_stat) {

		string op;

		BranchStat* branch = static_cast<BranchStat*> (stat);

		Symbol* label_sym = branch->getSymbol();

		string label = label_sym->getName();

		if (branch->isUnconditional()) {
			cout << "B " + label << endl;
			current->insertCode("B " + label);
		} else {
			genCmp(branch, current, label_stat);

			IRNode* oper = static_cast<Expr*> (branch->getChildren()->at(0))->getOperator();

			switch (static_cast<Tok*> (oper)->getOP()) {

			case token::leq:
				op = "BLE";
				break;
			case token::geq:
				op = "BGE";
				break;
			case token::gtr:
				op = "BGT";
				break;
			case token::lss:
				op = "BLT";
				break;
			case token::eql:
				op = "BEQ";
				break;
			case token::neq:
				op = "BNQ";
				break;
			}

			cout << op + " " + label << endl;
			current->insertCode(op + " " + label);

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

		IRNode* expr = expr = stat->getChildren()->at(0);

		Symbol* sym = static_cast<AssignStat*> (stat)->getSymbol();



		Symbol* reg = current->regalloc().mapVarReg()[sym];

		r1 = reg->getName();

		if (expr->NodeType() == "BinExpr" || expr->NodeType() == "UnExpr") {

			IRNode* oper = static_cast<Expr*> (expr)->getOperator();

			switch (static_cast<Tok*> (oper)->getOP()) {
			case token::plus:
				op = "ADD";
				break;
			case token::minus:
				op = "SUB";
				break;
			case token::slash:
				op = "DIV";
				break;
			case token::times:
				op = "MUL";
				break;
			}

			r2 = TermCode(expr->getChildren()->at(1), current);

			if (expr->NodeType() == "BinExpr") {
				r3 = TermCode(expr->getChildren()->at(2), current);

				cout << label_name + op + " " + r1 + " " + r2 + " " + r3 << endl;

				current->insertCode(label_name + op + " " + r1 + " " + r2 + " " + r3);
			} else {
				cout << label_name + op + " " + r1 + " " + r2 << endl;

				current->insertCode(label_name + op + " " + r1 + " " + r2);
			}



		} else {

			op = "MOV";

			r2 = TermCode(expr, current);

			cout << label_name + op + " " + r1 + " " + r2 << endl;

			current->insertCode(label_name + op + " " + r1 + " " + r2);
		}

	}

	string TermCode(IRNode* expr, FunctionCode* current) {

		ostringstream convert;

		if (expr->NodeType() == "Const") {
			Value val = static_cast<Const*> (expr)->getValue();
			convert << "#" << val.getValue();
			return convert.str();
		} else {
			Symbol* sym = static_cast<Var*> (expr)->getSymbol();

			if (sym->isSpilled()) {
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

			Symbol* reg = current->regalloc().mapVarReg()[sym];
			return reg->getName();
		}
	}

	std::list<FunctionCode*> functions;

	std::map<CFG*, RegisterAlloc*>& procedures;

	std::vector<Symbol*> aux;

	Symbol* mainSym;

};



#endif