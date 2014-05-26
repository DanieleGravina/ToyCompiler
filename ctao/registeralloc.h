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
#include "ir.h"

class RegisterAlloc{
public:
    RegisterAlloc(CFG& _cfg, unsigned int _nregs ): cfg(_cfg), nregs(_nregs), vars(nregs, NULL){
        
        std::set<Symbol*>* accessed;
        std::set<Symbol*>* crossed;
        
        std::set<Symbol*> temp;
        std::set<Symbol*> temp2;
        
        std::set<Symbol*> all_vars;
        
        std::map<Symbol* , int> var_freq;
        
        for(list<BasicBlock*>::iterator it = cfg.begin(); it != cfg.end(); ++it){
            
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
        
        for(list<BasicBlock*>::iterator it = cfg.begin(); it != cfg.end(); ++it){
            
            for(set<Symbol*>::iterator it2 = accessed_vars[*it]->begin(); it2 != accessed_vars[*it]->end(); ++it2){
                var_freq[*it2]++;
            }
            
            for(set<Symbol*>::iterator it2 = crossed_vars[*it]->begin(); it2 != crossed_vars[*it]->end(); ++it2){
                var_freq[*it2]++;
            }
            
        }
        
        for(map<Symbol*, int>::iterator it = var_freq.begin(); it != var_freq.end(); ++it){
            if(!var_stack.size())
                var_stack.push_back(it->first);
            else{
                if(var_freq[*it] < var_stack.back())
                    var_stack.push_back(it->first);
                else{
                   for(list<Symbol*>::iterator it2 = var_stack.begin(); it2 != var_stack.end(); ++it2){
                       if(var_freq[it->first] > var_freq[*it2])
                           var_stack.insert(it2, it->first);
                   } 
                }
            }
        }
        
    }
    
    register_alloc(){
        toSpill();
        
        if(!to_spill.size()){
            cout << "BB to spill";
            for(list<BasicBlock*>::iterator it = to_spill.begin(); it != to_spill.end(); ++it){
                cout << (*it)->getId() << " ";
            }
            
            cout << endl;
        }
        
        while(var_stack.size()){
            Symbol* var = var_stack.front();
            var_stack.pop_front();
            
            
            
        }
            
    }
           
    
private:
    
    
    void toSpill(){
        for(list<BasicBlock*>::iterator it = cfg.begin(); it != cfg.end(); ++it){
            if(accessed_vars[*it]->size() + crossed_vars[*it]->size() > nregs)
                to_spill.push_back(*it);
        }
    }
    
    CFG &cfg;
    unsigned int nregs;
    std::map<BasicBlock*, std::set<Symbol*>*> accessed_vars;
    std::map<BasicBlock*, std::set<Symbol*>*> crossed_vars;
    std::list<Symbol*> var_stack;
    std::list<BasicBlock*> to_spill;
    std::vector<Symbol*> vars;
};



#endif	/* REGISTERALLOC_H */

