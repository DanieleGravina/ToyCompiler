#include "registeralloc.h"

void Graph::addEdge(Symbol* v, Symbol* w)
{
	adj[v]->insert(w);
	adj[w]->insert(v);  // Note: the graph is undirected
}

void Graph::simplify(unsigned int neighbours){

	graph copy = adj;

	Symbol* to_spill = NULL;

	unsigned int max = 0;

	while(copy.empty()){

		for(graph::iterator it =  copy.begin(); it != copy.end(); ++it){
			if(it->second->size() < neighbours){
				for(graph::iterator it2 =  copy.begin(); it2 != copy.end(); ++it2){
					it2->second->erase(it->first);
				}
				st.push_back(it->first);
				copy.erase(it);
				it =  copy.begin();
			}
			else{
				if(it->second->size() > max)
					to_spill = it->first;
			}
		} 

		if(to_spill){
			st.push_back(to_spill);
			cout << "candidate spill " << to_spill->getName() << endl;
			copy.erase(to_spill);

			for(graph::iterator it =  copy.begin(); it != copy.end(); ++it){
				it->second->erase(to_spill);
			} 

			to_spill = NULL;
			max = 0;
		}
	}
}

Symbol* Graph::getNotInterfering(Symbol* var){
	for(graph::iterator it =  adj.begin(); it != adj.end(); ++it){
		if(it->first != var){
			if(it->second->find(var) == it->second->end())
				return it->first;
		}
	}
}

RegisterAlloc::RegisterAlloc(CFG& _cfg, unsigned int _nregs)
	: cfg(_cfg), nregs(_nregs), counter_regs(nregs) {

		std::set<Symbol*>* accessed;
		std::set<Symbol*>* crossed;

		std::set<Symbol*> temp;
		std::set<Symbol*> temp2;

		map<Symbol*, int> var_freq;


		/*for (list<BasicBlock*>::iterator it = cfg.begin(); it != cfg.end(); ++it) {

			BasicBlock::Union(temp, (*it)->getLiveIn());
			BasicBlock::Union(temp, (*it)->getLiveOut());

			for (set<Symbol*>::iterator it2 = temp.begin(); it2 != temp.end(); ++it2) {
				for (set<Symbol*>::iterator it3 = temp.begin(); it3 != temp.end(); ++it3) {
					if(*it3 != *it2)
						graph.addEdge(*it2, *it3);
				}	
			}
		}*/

		Symbol* reg = get_reg();

		while(reg){
			all_regs.insert(reg);
			reg = get_reg();
		}

		for (list<BasicBlock*>::iterator it = cfg.begin(); it != cfg.end(); ++it) {

			accessed = new std::set<Symbol*>();
			crossed = new std::set<Symbol*>();

			BasicBlock::Union(temp, (*it)->getGen());
			BasicBlock::Union(temp, (*it)->getKill());

			//BasicBlock::Union(temp, (*it)->getLiveIn());

			*accessed = temp;
			accessed_vars[*it] = accessed;

			BasicBlock::Union(temp2, (*it)->getLiveIn());
			BasicBlock::Union(temp2, (*it)->getLiveOut());

			BasicBlock::Difference(temp2, temp);

			*crossed = temp2;

			crossed_vars[*it] = crossed;

			BasicBlock::Union(all_vars, *accessed);
			BasicBlock::Union(all_vars, *crossed);
			//BasicBlock::Union(all_vars, (*it)->getGen());
			//BasicBlock::Union(all_vars, (*it)->getKill());

			temp.clear();
			temp2.clear();

		}

		for (set<Symbol*>::iterator it = all_vars.begin(); it != all_vars.end(); ++it) {
			vars[*it] = NULL;
		}

		for (list<BasicBlock*>::iterator it = cfg.begin(); it != cfg.end(); ++it) {

			for (set<Symbol*>::iterator it2 = accessed_vars[*it]->begin(); it2 != accessed_vars[*it]->end(); ++it2) {
				var_freq[*it2]++;
			}

			for (set<Symbol*>::iterator it2 = crossed_vars[*it]->begin(); it2 != crossed_vars[*it]->end(); ++it2) {
				var_freq[*it2]++;
			}

		}

		for (map<Symbol*, int>::iterator it = var_freq.begin(); it != var_freq.end(); ++it) {
			if (!var_stack.size())
				var_stack.push_back(it->first);
			else {
				if (it->second < var_freq[var_stack.back()])
					var_stack.push_back(it->first);
				else {
					for (list<Symbol*>::iterator it2 = var_stack.begin(); it2 != var_stack.end(); ++it2) {
						if (var_freq[it->first] >= var_freq[*it2]){
							if(it->first != *it2)
								var_stack.insert(it2, it->first);
							break;
						}
					}
				}
			}
		}

}

bool RegisterAlloc::TryAlloc() {
	toSpill();

	bool result = true;

	//graph.simplify(nregs);
	
	//return select();

	if (to_spill.size()) {
		cout << "BB to spill ";
		for (list<BasicBlock*>::iterator it = to_spill.begin(); it != to_spill.end(); ++it) {
			cout << (*it)->getId() << " ";
		}

		cout << endl;
	}

	Symbol* var = var_stack.back();
	Symbol* reg = NULL;
	std::set<Symbol*>* not_interfering;
	var_stack.pop_back();

	replace(var, next_free_reg());

	while (var_stack.size()) {
		var = var_stack.back();
		reg = NULL;
		var_stack.pop_back();

		if (!vars[var]) {
			not_interfering = get_not_interfering(var);
			if (not_interfering->size()) {
					replace(var, *not_interfering->begin());
					delete not_interfering;
			} else {
				delete not_interfering;
				reg = next_free_reg();
				if (reg) {
					replace(var, reg);
				}
				else {
					sym_to_spill = var;
					std::cout << "spill needed of var " << var->getName() <<  std::endl;
					result = false;
				}
			}
		}
	}

	return result;
}

/*bool RegisterAlloc::select(){
	list<Symbol*>& st = graph.getStack();

	Symbol* reg = NULL;
	Symbol* var = st.back();

	st.pop_back();
	while (var_stack.size()) {

		if (!vars[var]) {
			reg = next_free_reg();
			if (reg) {
				replace(var, reg);
			} else {
				Symbol *not_interfering = graph.getNotInterfering(var);
				if(not_interfering)
					replace(var, not_interfering);
				else{
					std::cout << "spill needed of var " << var->getName() << std::endl;
					sym_to_spill = var;
					return false;
				}
			}
		}
	}

	return true;
}*/
