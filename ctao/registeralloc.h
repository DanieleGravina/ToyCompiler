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
#include "cfg.h"

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
    
    Symbol* SymToSpill(){
        return sym_to_spill;
    }


private:

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

            for (set<Symbol*>::iterator it2 = accessed_vars[*it]->begin(); it2 != accessed_vars[*it]->end(); ++it2) {
                if (var == *it2) {
                    accessed_vars[*it]->erase(var);
                    accessed_vars[*it]->insert(reg);
                    break;
                }

            }

            for (set<Symbol*>::iterator it2 = crossed_vars[*it]->begin(); it2 != crossed_vars[*it]->end(); ++it2) {
                if (var == *it2) {
                    crossed_vars[*it]->erase(var);
                    crossed_vars[*it]->insert(reg);
                    break;
                }
            }

        }

        vars[var] = reg;
    }

    std::set<Symbol*>* get_not_interfering(Symbol* var) {
        std::set<Symbol*> interfering;
        std::set<Symbol*> the_vars;
        std::set<Symbol*>* not_interfering = new std::set<Symbol*>();

        for (list<BasicBlock*>::iterator it = cfg.begin(); it != cfg.end(); ++it) {
            BasicBlock::Union(the_vars, *accessed_vars[*it]);
            BasicBlock::Union(the_vars, *crossed_vars[*it]);
            if (the_vars.find(var) != the_vars.end()) {
                BasicBlock::Union(interfering, the_vars);
            }

            the_vars.clear();
        }

        *not_interfering = used_regs;
        BasicBlock::Difference(*not_interfering, interfering);

        return not_interfering;
    }

    Symbol* next_free_reg() {

        string name;

        ostringstream convert;

        convert << "Reg" << (nregs - counter_regs);

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
    unsigned int counter_regs;
    Symbol* sym_to_spill;
};



#endif	/* REGISTERALLOC_H */

