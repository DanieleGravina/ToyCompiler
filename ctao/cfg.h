/* 
 * File:   cfg.h
 * Author: daniele
 *
 * Created on 20 maggio 2014, 19.29
 */

#ifndef CFG_H
#define	CFG_H

#include<list>
#include<set>
#include "ir.h"

class BasicBlock{
public:
    
    BasicBlock(std::list<IRNode*>* _stats = NULL, list<Symbol*>* _labels = NULL)
		:stats(_stats), labels(_labels), target(NULL), next(NULL), total_var_used(0){


				std::set<Symbol*> uses;	

				if(static_cast<Stat*>(stats->back())->getLabel())
					target = static_cast<IRNode*>(static_cast<Stat*>(stats->back())->getLabel()->getTarget());

				live_in = new std::set<IRNode*>();
				live_out = new std::set<IRNode*>();

				kill = new std::set<Symbol*>(); //assigned
				gen = new std::set<Symbol*>();  // use before assign

				
				for(list<IRNode*>::iterator it = stats->begin(); it != stats->end(); ++it){

					for(list<Symbol*>::iterator it2 = (*it)->get_uses()->begin(); it2 != (*it)->get_uses()->end(); ++it2){
						uses.insert(*it2);
					}

					delete (*it)->get_uses();

					for(set<Symbol*>::iterator it3 = kill->begin(); it3 != kill->end(); ++it3){
						uses.erase(*it3);
					}

					for(set<Symbol*>::iterator it4 = uses.begin(); it4 != uses.end(); ++it4){
						gen->insert(*it4);
					}

					if((*it)->NodeType() == "AssignStat" || (*it)->NodeType() == "StoreStat"){
						kill->insert(static_cast<Stat*>(*it)->getSymbol());
					}

				}

				for(set<Symbol*>::iterator it = kill->begin(); it != kill->end(); ++it){
						gen->insert(*it);
				}

				total_var_used = gen->size();

	}
    
    ~BasicBlock(){
        if(next){
            next->~BasicBlock();
            delete next;
        }
        delete stats;
        delete labels;
		delete live_in;
		delete live_out;
		delete kill;
		delete gen;
    }
    
    /**
     * Return next block, null if not exits
     * @return BasicBlock*
     */
    BasicBlock* getNext(){
        return next;
    }
    
    void setNext(BasicBlock* next_block){
        next = next_block;
    }
    
private:
    BasicBlock* next;
    BasicBlock* bb_target;
	IRNode* target;
    std::list<IRNode*>* stats;
    std::list<Symbol*>* labels;
	std::set<IRNode*>* live_in;
	std::set<IRNode*>* live_out;
	std::set<Symbol*>* kill;
	std::set<Symbol*>* gen;
	int total_var_used;
};

std::list<BasicBlock*>* IRtoBB(IRNode* l){
    
    BasicBlock* bb = NULL;
    std::list<BasicBlock*>* lBB = new std::list<BasicBlock*>();
    std::list<Symbol*>* labels = new std::list<Symbol*>();
    std::list<IRNode*>* stats = new std::list<IRNode*>();
    
    
    for(vector<IRNode*>::iterator it = l->getChildren()->begin(); it != l->getChildren()->end(); ++it){
        Symbol* label = static_cast<Stat*>(*it)->getLabel();
        
        if(label != NULL){
            
			if(stats->size()){ //start new bb
				bb = new BasicBlock(stats, labels);
                lBB->push_back(bb);
                stats = new std::list<IRNode*>();
                lBB->back()->setNext(bb);
                labels = new std::list<Symbol*>();
            }
            labels->push_back(label);
        }
        
        stats->push_back(*it);
        
        if((*it)->NodeType() == "Branch" || (*it)->NodeType() == "CallStat"){
            if(stats->size()){ //start new bb
				bb = new BasicBlock(stats, labels);
                lBB->push_back(bb);
                stats = new std::list<IRNode*>();
                lBB->back()->setNext(bb);
                labels = new std::list<Symbol*>();
            }
        }
    }
    
    if(stats->size() || labels->size()){
        lBB->push_back(bb);
        bb = new BasicBlock(stats, labels);
        lBB->back()->setNext(bb);
        lBB->push_back(bb);
    }
    
    return lBB;
    
    
}



#endif	/* CFG_H */

