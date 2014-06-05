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


private:

    //bool select();

    void toSpill() {

        for (list<BasicBlock*>::iterator it = cfg.begin(); it != cfg.end(); ++it) {
            if (accessed_vars[*it]->size() + crossed_vars[*it]->size() > nregs)
                to_spill.push_back(*it);
        }
    }

    void usedRegs() {
        for (map<Symbol*, Symbol*>::iterator it = vars.begin(); it != vars.end(); ++it) {
            if (vars[it->first])
                used_regs.insert(it->second);
        }
    }

    void replace(Symbol* var, Symbol* reg) {

        for (list<BasicBlock*>::iterator it = cfg.begin(); it != cfg.end(); ++it) {

            if (accessed_vars[*it]->find(var) != accessed_vars[*it]->end()) {
                accessed_vars[*it]->erase(var);
                accessed_vars[*it]->insert(reg);
            }

            if (crossed_vars[*it]->find(var) != crossed_vars[*it]->end()) {
                crossed_vars[*it]->erase(var);
                crossed_vars[*it]->insert(reg);
            }

        }

        vars[var] = reg;
    }

    std::set<Symbol*>* get_not_interfering(Symbol* var) {

        std::set<Symbol*> the_vars = graph.getInterfering(var);
        std::set<Symbol*> interfering;
        std::set<Symbol*>* not_interfering = new std::set<Symbol*>();

        /*std::set<Symbol*> interfering;
        std::set<Symbol*> the_vars;
        std::set<Symbol*>* not_interfering = new std::set<Symbol*>();

        for (list<BasicBlock*>::iterator it = cfg.begin(); it != cfg.end(); ++it) {
            BasicBlock::Union(the_vars, *accessed_vars[*it]);
            BasicBlock::Union(the_vars, *crossed_vars[*it]);
            if (the_vars.find(var) != the_vars.end()) {
                                the_vars.erase(var);
                BasicBlock::Union(interfering, the_vars);
            }

            the_vars.clear();

                        BasicBlock::Union(the_vars, *accessed_vars[*it]);
            if (the_vars.find(var) != the_vars.end()) {
                                the_vars.erase(var);
                BasicBlock::Union(interfering, the_vars);
            }

            the_vars.clear();
        }*/

        for (std::set<Symbol*>::iterator it = the_vars.begin(); it != the_vars.end(); ++it) {
            if (vars[*it] != NULL)
                interfering.insert(vars[*it]);

        }

        *not_interfering = all_regs;
        BasicBlock::Difference(*not_interfering, interfering);

        return not_interfering;
    }

    Symbol* next_free_reg() {
        std::set<Symbol*> avaible_reg = all_regs;
        Symbol *reg;
        for (map<Symbol*, Symbol*>::iterator it = vars.begin(); it != vars.end(); ++it) {
            if (it->second)
                avaible_reg.erase(it->second);
        }

        if (avaible_reg.size()) {
            return *avaible_reg.begin();
        }

        return NULL;
    }

    Symbol* get_reg() {

        string name;

        ostringstream convert;

        convert << "R" << (nregs - counter_regs);

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
    unsigned int nregs;
    std::map<BasicBlock*, std::set<Symbol*>*> accessed_vars;
    std::map<BasicBlock*, std::set<Symbol*>*> crossed_vars;
    std::list<Symbol*> var_stack;
    std::list<BasicBlock*> to_spill;
    std::map<Symbol*, Symbol*> vars;
    std::set<Symbol*> used_regs;
    std::set<Symbol*> all_vars;
    std::set<Symbol*> all_regs;
    unsigned int counter_regs;
    Symbol* sym_to_spill;
    Graph graph;
};



#endif	/* REGISTERALLOC_H */

