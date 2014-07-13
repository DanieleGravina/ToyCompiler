#include "registeralloc.h"

void Graph::addEdge(Symbol* v, Symbol* w) {
    adj[v]->insert(w);
    adj[w]->insert(v); // Note: the graph is undirected
}

void Graph::simplify(unsigned int neighbours) {

    graph copy;

    for (graph::iterator it = adj.begin(); it != adj.end(); ++it) {
        copy[it->first] = new std::set<Symbol*>(*it->second);
    }

    Symbol* to_spill = NULL;

    unsigned int max = 0;

    while (!copy.empty()) {

        for (graph::iterator it = copy.begin(); it != copy.end(); ++it) {
            if (it->second->size() < neighbours) {
                for (graph::iterator it2 = copy.begin(); it2 != copy.end(); ++it2) {
                    it2->second->erase(it->first);
                }
                st.push_back(it->first);
                copy.erase(it);
                if (copy.empty())
                    break;
                it = copy.begin();
            }
        }

		 for (graph::iterator it = copy.begin(); it != copy.end(); ++it){
            if (it->second->size() > max) {
                to_spill = it->first;
                max = it->second->size();
            }
		 }

        if (to_spill) {
            st.push_back(to_spill);
            cout << "candidate spill " << to_spill->getName() << endl;
            copy.erase(to_spill);

            for (graph::iterator it = copy.begin(); it != copy.end(); ++it) {
                it->second->erase(to_spill);
            }

            to_spill = NULL;
            max = 0;
        }
    }

}

std::set<Symbol*>* Graph::getNotInterfering(Symbol* var) {

    std::set<Symbol*>* not_interfering = new std::set<Symbol*>();

    for (graph::iterator it = adj.begin(); it != adj.end(); ++it) {
        if (it->first != var) {
            if (it->second->find(var) == it->second->end())
                not_interfering->insert(it->first);
        }
    }

    return not_interfering;
}

RegisterAlloc::RegisterAlloc(CFG& _cfg, unsigned int _nregs)
	: cfg(_cfg), real_regs(_nregs), nregs(real_regs - 1), counter_regs(nregs), graph(cfg), parameters(NULL), zero(NULL) {


    std::set<Symbol*> temp;
    std::set<Symbol*> temp2;
    
	if((*cfg.begin())->getFunction()){
		parameters = static_cast<FunctionDef*> ((*cfg.begin())->getFunction())->getParameters();
	}
    
    
    for (list<BasicBlock*>::iterator it = cfg.begin(); it != cfg.end(); ++it) {
        BasicBlock::Union(all_vars, (*it)->getGen());
        BasicBlock::Union(all_vars, (*it)->getKill());
        BasicBlock::Union(all_vars, (*it)->getLiveOut());
        BasicBlock::Union(all_vars, (*it)->getLiveIn());

		if(!zero)
			zero = (*it)->getSymTab()->find("zero");
    }


    for (list<BasicBlock*>::iterator it = cfg.begin(); it != cfg.end(); ++it) {

        BasicBlock::Union(temp, (*it)->getLiveIn());
        BasicBlock::Union(temp2, (*it)->getLiveOut());

        for (set<Symbol*>::iterator it = all_vars.begin(); it != all_vars.end(); ++it) {
            if (temp.find(*it) != temp.end()) {
                for (set<Symbol*>::iterator it2 = temp.begin(); it2 != temp.end(); ++it2) {
                    if (*it != *it2)
                        graph.addEdge(*it, *it2);
                }
            }

            if (temp2.find(*it) != temp2.end()) {
                for (set<Symbol*>::iterator it2 = temp2.begin(); it2 != temp2.end(); ++it2) {
                    if (*it != *it2)
                        graph.addEdge(*it, *it2);
                }
            }
        }

        temp.clear();
        temp2.clear();
    }




    for (std::set<Symbol*>::iterator it = all_vars.begin(); it != all_vars.end(); ++it) {
        std::set<Symbol*>* not_i = graph.getNotInterfering(*it);

        cout << (*it)->getName() << " not interfere ";

        for (std::set<Symbol*>::iterator it2 = not_i->begin(); it2 != not_i->end(); ++it2) {
            cout << " " << (*it2)->getName();
        }

        cout << endl;
    }



    Symbol* reg = get_reg();

    while (reg) {
        all_regs.insert(reg);
        reg = get_reg();
    }


    for (set<Symbol*>::iterator it = all_vars.begin(); it != all_vars.end(); ++it) {
        vars[*it] = NULL;
    }

    

}

bool RegisterAlloc::TryAlloc() {
    //toSpill();

    bool result = true;

    graph.simplify(nregs);

    /*if (to_spill.size()) {
            cout << "BB to spill ";
            for (list<BasicBlock*>::iterator it = to_spill.begin(); it != to_spill.end(); ++it) {
                    cout << (*it)->getId() << " ";
            }

            cout << endl;
    }*/

    //Symbol* var = var_stack.back();

	std::list<Symbol*> stack;
	set<Symbol*> parameters_inserted; 

	if(parameters){
		for(SymbolTable::iterator it = parameters->begin(); it != parameters->end(); ++it){
			stack.push_back(it->second);
			parameters_inserted.insert(it->second);
		}

		for(std::list<Symbol*>::iterator it = graph.getStack().begin(); it != graph.getStack().end(); ++it){
			if(parameters_inserted.find(*it) == parameters_inserted.end())
				stack.push_back(*it);
		}
	}
	else{
		stack = graph.getStack();
	}

    Symbol* var = stack.front();
    Symbol* reg = NULL;
    std::set<Symbol*>* not_interfering;
    stack.pop_front();

    replace(var, next_free_reg());

    while (stack.size()) {
        var = stack.front();
        reg = NULL;
        stack.pop_front();

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
                } else {
                    sym_to_spill = var;
                    std::cout << "spill needed of var " << var->getName() << std::endl;
                    result = false;
                }
            }
        }
    }

	//assign zero to a dedicated register

	string reg_zero;

    ostringstream convert;

	convert << "r" << real_regs - 1;

    reg_zero = convert.str();

	if(zero)
		replace(zero, new Symbol(reg_zero));

    return result;
}

