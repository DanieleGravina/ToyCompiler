#include "cfg.h"

/**
* BasicBlock
*/

BasicBlock::BasicBlock(std::list<IRNode*>* _stats = NULL, list<Symbol*>* _labels = NULL)
	: stats(_stats), labels(_labels), target(NULL), next(NULL), bb_target(NULL), total_var_used(0),
	function_sym(NULL) {

		myId = ++Id;

		init();

}

void BasicBlock::init() {
	std::set<Symbol*> uses;

	uses.clear();

	if (stats->back()->NodeType() == "Branch")
		target = static_cast<IRNode*> (static_cast<BranchStat*> (stats->back())->getSymbol()->getTarget());

	live_in.clear();
	live_out.clear();
	kill.clear(); //assigned
	gen.clear(); // use before assign

	for (list<IRNode*>::iterator it = stats->begin(); it != stats->end(); ++it) {


		for (list<Symbol*>::iterator it2 = (*it)->get_uses().begin(); it2 != (*it)->get_uses().end(); ++it2) {
			uses.insert(*it2);
		}

		Difference(uses, spilled);

		Difference(uses, kill);

		Union(gen, uses);

		if ((*it)->NodeType() == "AssignStat" || (*it)->NodeType() == "Load") {
			kill.insert(static_cast<Stat*> (*it)->getSymbol());
			Difference(kill, spilled);
		}

	}

	set<Symbol*> temp;

	Union(temp, gen);
	Union(temp, kill);

	total_var_used = temp.size();
}

bool BasicBlock::liveness_iteration() {

	int lin = live_in.size();
	int lout = live_out.size();

	if (next != NULL) {
		Union(getLiveOut(), next->getLiveIn());
	}

	if (bb_target != NULL) {
		Union(getLiveOut(), bb_target->getLiveIn());
	}

	if (next == NULL && bb_target == NULL) {
		if (getFunction()) { //not global
			function_sym = static_cast<FunctionDef*> (getFunction())->getSymbol();
			std::list<Symbol*>& global = static_cast<FunctionDef*> (getFunction())->getGlobalSymbol();
			for (std::list<Symbol*>::iterator it = global.begin(); it != global.end(); ++it) {
				live_out.insert(*it);
			}
			Difference(live_out, spilled);
		}
	}

	std::set<Symbol*> temp;

	Union(temp, live_out);

	Difference(temp, kill);

	Union(temp, gen);

	live_in = temp;

	return (lin == live_in.size() && lout == live_out.size());

}

void BasicBlock::spill(Symbol* to_spill) {

	if (!((*stats->begin())->getSymTab()->find("zero"))) {
		(*stats->begin())->getSymTab()->append(new Symbol("zero"));
	}

	std::list<IRNode*>::iterator it = stats->begin();

	while( it != stats->end()) {
		if ((*it)->NodeType() == "Load"){
			if(static_cast<LoadStat*>(*it)->isInitial() && static_cast<LoadStat*>(*it)->getSymbol() == to_spill){
				stats->remove(*it);
				it = stats->begin();
				continue;
			}
		}

		if ((*it)->NodeType() == "StoreStat"){
			if(static_cast<StoreStat*>(*it)->isFinal() && static_cast<StoreStat*>(*it)->getSymbol() == to_spill){
				stats->remove(*it);
				it = stats->begin();
				continue;
			}
		}

		++it;
	}


	for (std::list<IRNode*>::iterator it = stats->begin(); it != stats->end(); ++it) {

		if ((*it)->NodeType() == "AssignStat") {
			AssignStat* assign = static_cast<AssignStat*> (*it);
			IRNode* assign_zero = NULL;
			IRNode* var = NULL;

			if (assign->getSymbol() == to_spill) {
				Symbol* temp = new Symbol(Symbol::genUniqueId());

				IRNode* store;

				if (to_spill->isGlobal()) {
					Symbol* zero = (*it)->getSymTab()->find("zero");
					assign_zero = new AssignStat(zero, new Const(0, (*it)->getSymTab()), (*it)->getSymTab());
					var = new Var(to_spill, (*it)->getSymTab());
					store = new StoreStat(temp, new Var((*it)->getSymTab()->find("zero"), (*it)->getSymTab()), var, (*it)->getSymTab());
				}
				else{
					var = new Const(0, (*it)->getSymTab());
					store = new StoreStat(temp, var, var, (*it)->getSymTab());
				}

				assign->setSymbol(temp);

				(*it)->getSymTab()->append(temp);

				if (it == stats->end()){
					if(assign_zero)
						stats->push_back(assign_zero);
					stats->push_back(store);
				}
				else{
					++it;
					if(assign_zero){
						stats->insert(it, assign_zero);
					}

					stats->insert(it, store);

					for (std::list<IRNode*>::iterator it = stats->begin(); it != stats->end(); ++it) {
						cout << (*it)->NodeType() << " " << endl;
					}

				}



				std::list<IRNode*>::iterator it_old = it;
				it = stats->begin();
				while(it != it_old)
					it++;
				continue;
			}
		}


		for (list<Symbol*>::iterator it2 = (*it)->get_uses().begin(); it2 != (*it)->get_uses().end(); ++it2) {
			if ((*it2) == to_spill && (*it)->NodeType() != "CallStat") {

				IRNode* assign_zero = NULL;
				IRNode* var = NULL;

				IRNode* load;

				Symbol* temp = new Symbol(Symbol::genUniqueId());


				if (to_spill->isGlobal()) {
					Symbol* zero = (*it)->getSymTab()->find("zero");
					assign_zero = new AssignStat(zero, new Const(0, (*it)->getSymTab()), (*it)->getSymTab());
					var = new Var(to_spill, (*it)->getSymTab());
					load = new LoadStat(temp, new Var(zero, (*it)->getSymTab()), var, (*it)->getSymTab());
				}
				else{
					var = new Const(0, (*it)->getSymTab());
					load = new LoadStat(temp, var, new Var(to_spill, (*it)->getSymTab()), (*it)->getSymTab());
				}

				if(assign_zero)
					stats->insert(it, assign_zero);
				stats->insert(it, load);

				(*it)->replace_uses(to_spill, temp);

				(*it)->getSymTab()->append(temp);

				std::list<IRNode*>::iterator it_old = it;
				it = stats->begin();
				while(it != it_old)
					it++;

				break;
			}
		}

		to_spill->spill();

		spilled.insert(to_spill);


	}

	init();

}

void BasicBlock::insertLoadGlobal(){

	IRNode* parent = (*stats->begin())->getParent();

	if (!((*stats->begin())->getSymTab()->find("zero"))) {
		(*stats->begin())->getSymTab()->append(new Symbol("zero"));
	}

	Symbol* zero = (*stats->begin())->getSymTab()->find("zero");

	IRNode* assign_zero = new AssignStat(zero, new Const(0, (*stats->begin())->getSymTab()), (*stats->begin())->getSymTab());

	assign_zero->setParent(parent);

	SymbolTable* symtab = (*stats->begin())->getSymTab();

	for(SymbolTable::iterator it = symtab->begin(); it != symtab->end(); ++it){
		if(it->second->isGlobal() && !it->second->isSpilled() && (it->second->getType().getName() != "Function" && it->second->getType().getName() != "Label")){
			IRNode* load = new LoadStat(it->second, new Var(zero, symtab), new Var(it->second, symtab), symtab);
			static_cast<LoadStat*>(load)->setInitial();
			load->setParent(parent);
			stats->push_front(load);
		}
	}

	stats->push_front(assign_zero);

	init();
}

void BasicBlock::insertStoreGlobal(){

	IRNode* parent = (*stats->rbegin())->getParent();

	if (!((*stats->rbegin())->getSymTab()->find("zero"))) {
		(*stats->rbegin())->getSymTab()->append(new Symbol("zero"));
	}

	Symbol* zero = (*stats->rbegin())->getSymTab()->find("zero");

	IRNode* assign_zero = new AssignStat(zero, new Const(0, (*stats->rbegin())->getSymTab()), (*stats->rbegin())->getSymTab());

	assign_zero->setParent(parent);

	SymbolTable* symtab = (*stats->rbegin())->getSymTab();

	stats->push_back(assign_zero);

	for(SymbolTable::iterator it = symtab->begin(); it != symtab->end(); ++it){
		if(it->second->isGlobal() && !it->second->isSpilled() && (it->second->getType().getName() != "Function" && it->second->getType().getName() != "Label")){
			IRNode* store = new StoreStat(it->second, new Var(zero, symtab), new Var(it->second, symtab), symtab);
			static_cast<StoreStat*>(store)->setFinal();
			store->setParent(parent);
			stats->push_back(store);
		}
	}

	init();
}

void BasicBlock::registerAllocation(){

	std::list<std::set<Symbol*>*> live_in;
	std::list<std::set<Symbol*>*> live_out;

	std::set<Symbol*>* temp;

	temp = new std::set<Symbol*>();

	*temp = getLiveOut();

	live_out.push_front(temp);

	temp = new std::set<Symbol*>();

	for(std::list<IRNode*>::reverse_iterator it = stats->rbegin(); it != stats->rend(); ++it){

		Union(*temp, *(live_out.front()));

		for (list<Symbol*>::iterator it2 = (*it)->get_uses().begin(); it2 != (*it)->get_uses().end(); ++it2) {
			temp->insert(*it2);
		}

		Difference(*temp, spilled);

		if ((*it)->NodeType() == "AssignStat" || (*it)->NodeType() == "Load") {
			temp->erase(static_cast<Stat*> (*it)->getSymbol());
		}

		live_in.push_front(temp);

		live_out.push_front(temp);

		temp = new std::set<Symbol*>(); 

	}

	std::set<Symbol*> all_vars;

	BasicBlock::Union(all_vars, getGen());
    BasicBlock::Union(all_vars, getKill());
    BasicBlock::Union(all_vars, getLiveOut());
    BasicBlock::Union(all_vars, getLiveIn());

	RegisterAlloc* regalloc = new RegisterAlloc(*this, 8);

	for (set<Symbol*>::iterator it = all_vars.begin(); it != all_vars.end(); ++it) {
		for(std::list<std::set<Symbol*>*>::iterator it2 = live_out.begin(); it2 != live_out.end(); ++it2){
			if( (*it2)->find(*it) != (*it2)->end()){
				for (std::set<Symbol*>::iterator it3 = (*it2)->begin(); it3 != (*it2)->end(); ++it3) {
                    if (*it != *it3)
						regalloc->addInterference(*it, *it3);
                }
			}
		}
	}

	while(regalloc->TryAlloc()){
		spill(regalloc->SymToSpill());
		regalloc =  new RegisterAlloc(*this, 8);
	}
}

/**
* IR to BasicBlock 
*/
std::list<BasicBlock*>* IRtoBB(IRNode* l) {

	BasicBlock* bb = NULL;
	std::list<BasicBlock*>* lBB = new std::list<BasicBlock*>();
	std::list<Symbol*>* labels = new std::list<Symbol*>();
	std::list<IRNode*>* stats = new std::list<IRNode*>();

	IRNode* temp;
	std::list<IRNode*> stack;


	for (vector<IRNode*>::iterator it = l->getChildren()->begin(); it != l->getChildren()->end(); ++it) {
		Symbol* label = static_cast<Stat*> (*it)->getLabel();

		if (label != NULL) {

			if (stats->size()) { //start new bb
				bb = new BasicBlock(stats, labels);
				stats = new std::list<IRNode*>();
				if (lBB->size())
					lBB->back()->setNext(bb);
				lBB->push_back(bb);
				labels = new std::list<Symbol*>();
			}

			labels->push_back(label);
		}

		(*it)->lower_expr(&stack);

		for (std::list<IRNode*>::reverse_iterator it2 = stack.rbegin(); it2 != stack.rend(); ++it2) {
			stats->push_back(*it2);
		}

		stats->push_back(*it);

		stack.clear();


		if ((*it)->NodeType() == "Branch" || (*it)->NodeType() == "CallStat") {

			if (stats->size()) { //start new bb
				bb = new BasicBlock(stats, labels);
				stats = new std::list<IRNode*>();
				if (lBB->size())
					lBB->back()->setNext(bb);
				lBB->push_back(bb);
				labels = new std::list<Symbol*>();
			}
		}
	}

	if (stats->size() || labels->size()) {
		bb = new BasicBlock(stats, labels);
		if (lBB->size())
			lBB->back()->setNext(bb);
		lBB->push_back(bb);
	}

	return lBB;


}

/**
* CFG
*/

CFG::CFG(std::list<BasicBlock*>& proc) {


	for (list<BasicBlock*>::iterator it2 = proc.begin(); it2 != proc.end(); ++it2) {
		cfg.push_back(*it2);
	}

	for (list<BasicBlock*>::iterator it2 = cfg.begin(); it2 != cfg.end(); ++it2) {

		(*it2)->repr();

		if ((*it2)->getTarget() != NULL) {
			Symbol *label = static_cast<Stat*> ((*it2)->getTarget())->getLabel();
			(*it2)->setBBTarget(find_target_bb(label));
		}

		(*it2)->remove_useless_next();
	}

	insertLoadAndStoreGlobal();
}

void CFG::spill(Symbol* sym) {
	for (CFG::iterator it = begin(); it != end(); ++it) {
		(*it)->spill(sym);
	}

	spilled.insert(sym);
}



