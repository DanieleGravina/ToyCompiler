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
	RegisterAlloc(CFG& _cfg, unsigned int _nregs ): cfg(_cfg), nregs(_nregs), counter_regs(nregs){
        
        std::set<Symbol*>* accessed;
        std::set<Symbol*>* crossed;
        
        std::set<Symbol*> temp;
        std::set<Symbol*> temp2;
        
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

		for(set<Symbol*>::iterator it = all_vars.begin(); it != all_vars.end(); ++it){
			vars[*it] = NULL;	
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
				if(it->second < var_freq[var_stack.back()])
                    var_stack.push_back(it->first);
                else{
                   for(list<Symbol*>::iterator it2 = var_stack.begin(); it2 != var_stack.end(); ++it2){
                       if(var_freq[it->first] >= var_freq[*it2])
                           var_stack.insert(it2, it->first);
                   } 
                }
            }
        }
        
    }
    
    bool register_alloc(){
        toSpill();
        
        if(to_spill.size()){
            cout << "BB to spill";
            for(list<BasicBlock*>::iterator it = to_spill.begin(); it != to_spill.end(); ++it){
                cout << (*it)->getId() << " ";
            }
            
            cout << endl;
        }
        
        while(var_stack.size()){
            Symbol* var = var_stack.front();
			Symbol* reg = NULL;
			std::set<Symbol*>* not_interfering;
            var_stack.pop_front();
            
			if(!vars[var]){
				reg = next_free_reg();
				if(reg){
					replace(var, reg);
				}
				else{
					not_interfering = get_not_interfering(var);
					if(not_interfering->size()){
						replace(var, vars[*not_interfering->begin()]);
						delete not_interfering;
					}
					else{
						std::cout << "spill needed" << std::endl;
						delete not_interfering;
						return false;
					}
				}
			}
            
        }

		return true;
            
    }

	void res(){
		for(map<Symbol*, Symbol*>::iterator it = vars.begin(); it != vars.end(); ++it){
			if(it->second)
				std::cout << it->first->getName() << " : " << it->second->getName() << std::endl;
		}
	}
           
    
private:
    
    
    void toSpill(){
        for(list<BasicBlock*>::iterator it = cfg.begin(); it != cfg.end(); ++it){
            if(accessed_vars[*it]->size() + crossed_vars[*it]->size() > nregs)
                to_spill.push_back(*it);
        }
    }

	 void usedRegs(){
		for(map<Symbol*, Symbol*>::iterator it = vars.begin(); it != vars.end(); ++it){
			if(vars[it->first])
				used_regs.insert(it->second);
		}
	}

	 void replace(Symbol* var, Symbol* reg){

		  for(list<BasicBlock*>::iterator it = cfg.begin(); it != cfg.end(); ++it){
            
            for(set<Symbol*>::iterator it2 = accessed_vars[*it]->begin(); it2 != accessed_vars[*it]->end(); ++it2){
                if(var == *it2){
					accessed_vars[*it]->erase(var);
					accessed_vars[*it]->insert(reg);
					break;
				}
				
            }
            
            for(set<Symbol*>::iterator it2 = crossed_vars[*it]->begin(); it2 != crossed_vars[*it]->end(); ++it2){
                if(var == *it2){
					crossed_vars[*it]->erase(var);
					crossed_vars[*it]->insert(reg);
					break;
				}
            }
            
        }

		  vars[var] = reg;
	 }

	 std::set<Symbol*>* get_not_interfering(Symbol* var){
		 std::set<Symbol*> interfering;
		 std::set<Symbol*> the_vars;
		 std::set<Symbol*>* not_interfering = new std::set<Symbol*>();

		 for(list<BasicBlock*>::iterator it = cfg.begin(); it != cfg.end(); ++it){
			 BasicBlock::Union(the_vars, *accessed_vars[*it]);
			 BasicBlock::Union(the_vars, *crossed_vars[*it]);
			 if(the_vars.find(var) != the_vars.end()){
				 BasicBlock::Union(interfering, the_vars);
			 }

			 the_vars.clear();
		 }

		 *not_interfering = used_regs;
		 BasicBlock::Difference(*not_interfering, interfering);
		 
		 return not_interfering;
	 }

	Symbol* next_free_reg(){

		string name;         

        ostringstream convert;   

		convert << "Reg" << (nregs - counter_regs);       

        name = convert.str();

		if(counter_regs){
			Symbol *reg;
			counter_regs--;
			reg = new Symbol(name);
			return reg;
		}
		else
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
};



#endif	/* REGISTERALLOC_H */

