#include "registeralloc.h"

RegisterAlloc::RegisterAlloc(CFG& _cfg, unsigned int _nregs)
: cfg(_cfg), nregs(_nregs), counter_regs(nregs) {

    std::set<Symbol*>* accessed;
    std::set<Symbol*>* crossed;

    std::set<Symbol*> temp;
    std::set<Symbol*> temp2;

    std::map<Symbol*, int> var_freq;

    for (list<BasicBlock*>::iterator it = cfg.begin(); it != cfg.end(); ++it) {

        accessed = new std::set<Symbol*>();
        crossed = new std::set<Symbol*>();

        BasicBlock::Union(temp, (*it)->getGen());
        BasicBlock::Union(temp, (*it)->getKill());

        *accessed = temp;
        accessed_vars[*it] = accessed;

        BasicBlock::Union(temp, (*it)->getLiveIn());
        BasicBlock::Union(temp, (*it)->getLiveOut());

        BasicBlock::Difference(temp, temp2);

        *crossed = temp;

        crossed_vars[*it] = crossed;

        BasicBlock::Union(all_vars, *accessed);
        BasicBlock::Union(all_vars, *crossed);

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
                    if (var_freq[it->first] >= var_freq[*it2])
                        var_stack.insert(it2, it->first);
                }
            }
        }
    }

}

bool RegisterAlloc::TryAlloc() {
    toSpill();

    if (to_spill.size()) {
        cout << "BB to spill";
        for (list<BasicBlock*>::iterator it = to_spill.begin(); it != to_spill.end(); ++it) {
            cout << (*it)->getId() << " ";
        }

        cout << endl;
    }

    while (var_stack.size()) {
        Symbol* var = var_stack.back();
        Symbol* reg = NULL;
        std::set<Symbol*>* not_interfering;
        var_stack.pop_back();

        if (!vars[var]) {
            reg = next_free_reg();
            if (reg) {
                replace(var, reg);
            } else {
                not_interfering = get_not_interfering(var);
                if (not_interfering->size()) {
                    replace(var, vars[*not_interfering->begin()]);
                    delete not_interfering;
                } else {
                    std::cout << "spill needed" << std::endl;
                    sym_to_spill = var;
                    delete not_interfering;
                    return false;
                }
            }
        }

    }
}
