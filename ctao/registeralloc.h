/* 
 * File:   registeralloc.h
 * Author: daniele
 *
 * Created on 26 maggio 2014, 10.32
 */

#ifndef REGISTERALLOC_H
#define	REGISTERALLOC_H


#include<map>
#include<set>
#include<list>
#include "cfg.h"

#include <iostream>
#include <list>
using namespace std;

// A class that represents an undirected graph

class Graph {
    typedef map<Symbol*, set<Symbol*>*> graph;
    typedef list<Symbol*> stack;
    graph adj;
    stack st;


public:
    // Constructor and destructor

    Graph(CFG& cfg) {

        set<Symbol*> all_vars;

        for (list<BasicBlock*>::iterator it = cfg.begin(); it != cfg.end(); ++it) {
            BasicBlock::Union(all_vars, (*it)->getGen());
            BasicBlock::Union(all_vars, (*it)->getKill());
            BasicBlock::Union(all_vars, (*it)->getLiveOut());
            BasicBlock::Union(all_vars, (*it)->getLiveIn());
        }

        for (std::set<Symbol*>::iterator it = all_vars.begin(); it != all_vars.end(); ++it) {
            adj[*it] = new set<Symbol*>();
        }
    }

    ~Graph() {
        for (graph::iterator it = adj.begin(); it != adj.end(); ++it) {
            delete it->second;
        }
    }

    // function to add an edge to graph
    void addEdge(Symbol* v, Symbol* w);

    // return true if simplified graph is empty
    void simplify(unsigned int neighbours);

    std::set<Symbol*>* getNotInterfering(Symbol* var);

    std::set<Symbol*>& getInterfering(Symbol* var) {
        return *adj[var];
    }

    stack& getStack() {
        return st;
    }

    void toSpill(int nregs) {
        for (graph::iterator it = adj.begin(); it != adj.end(); ++it) {
            if (it->second->size() > nregs)
                cout << it->first->getName() << " candidate to spill" << endl;
        }
    }

};

class RegisterAlloc {
public:

    RegisterAlloc(CFG& _cfg, unsigned int _nregs);

	void addInterference(Symbol* first, Symbol* second);

    bool TryAlloc();

    void res() {
        for (map<Symbol*, Symbol*>::iterator it = vars.begin(); it != vars.end(); ++it) {
            if (it->second)
                std::cout << it->first->getName() << " : " << it->second->getName() << std::endl;
        }
    }

    map<Symbol*, Symbol*>& mapVarReg() {
        return vars;
    }

    Symbol* SymToSpill() {
        return sym_to_spill;
    }

	std::set<Symbol*>& Vars(){
		return all_vars;
	}

	int getNumParameters() const{
		if (parameters)
			return parameters->size();
		else
			return 0;
	}

	SymbolTable* getParameters(){
		return parameters;
	}

	int num_reg(){
		usedRegs();
		return used_regs.size();
	}

	set<Symbol*>& allRegs(){
		return all_regs;
	}

	Symbol* getCandidateToSpill(){
		return *graph.getStack().rbegin();
	}




private:

    /*void toSpill() {

        for (list<BasicBlock*>::iterator it = cfg.begin(); it != cfg.end(); ++it) {
            if (accessed_vars[*it]->size() + crossed_vars[*it]->size() > nregs)
                to_spill.push_back(*it);
        }
    }*/

    void usedRegs() {
        for (map<Symbol*, Symbol*>::iterator it = vars.begin(); it != vars.end(); ++it) {
            if (vars[it->first])
                used_regs.insert(it->second);
        }
    }

    void replace(Symbol* var, Symbol* reg) {
        vars[var] = reg;
    }

    std::set<Symbol*>* get_not_interfering(Symbol* var) {

        std::set<Symbol*> the_vars = graph.getInterfering(var);
        std::set<Symbol*> interfering;
        std::set<Symbol*>* not_interfering = new std::set<Symbol*>();

        for (std::set<Symbol*>::iterator it = the_vars.begin(); it != the_vars.end(); ++it) {
            if (vars[*it] != NULL)
                interfering.insert(vars[*it]);

        }

        *not_interfering = all_regs;
        BasicBlock::Difference(*not_interfering, interfering);

        return not_interfering;
    }

    Symbol* next_free_reg() {
		if(ordered_regs.size()){
			Symbol* result = ordered_regs.front();
			ordered_regs.pop_front();
			return result;
		}
		else
			return NULL;
    }

    Symbol* get_reg() {

        string name;

        ostringstream convert;

        convert << "r" << (nregs - counter_regs);

        name = convert.str();

        if (counter_regs) {
            Symbol *reg;
            counter_regs--;
            reg = new Symbol(name);
            return reg;
        } else
            return NULL;

    }

	CFG &cfg;
	unsigned int real_regs;
    unsigned int nregs;
	unsigned int counter_regs;
    std::list<Symbol*> var_stack;
    std::list<BasicBlock*> to_spill;
    std::map<Symbol*, Symbol*> vars;
    std::set<Symbol*> used_regs;
    std::set<Symbol*> all_vars;
    std::set<Symbol*> all_regs;
	std::list<Symbol*> ordered_regs;
    Symbol* sym_to_spill;
    Graph graph;

	SymbolTable* parameters;
	Symbol* zero; //special var that contains always zero
};



#endif	/* REGISTERALLOC_H */

