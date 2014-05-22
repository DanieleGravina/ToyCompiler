/* 
 * File:   cfg.h
 * Author: daniele
 *
 * Created on 20 maggio 2014, 19.29
 */

#ifndef CFG_H
#define	CFG_H

#include<list>
#include "ir.h"

class BasicBlock{
public:
    
    BasicBlock(std::list<IRNode*>* _stats = NULL, list<Symbol*>* _labels = NULL)
            :stats(_stats), labels(_labels){}
    
    ~BasicBlock(){
        if(next){
            next->~BasicBlock();
            delete next;
        }
        delete stats;
        delete labels;
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
    BasicBlock* branch;
    std::list<IRNode*>* stats;
    std::list<Symbol*>* labels;
};

std::list<BasicBlock*>* IRtoBB(IRNode* l){
    
    BasicBlock* bb = NULL;
    std::list<BasicBlock*>* lBB = new std::list<BasicBlock*>();
    std::list<Symbol*>* labels = new std::list<Symbol*>();
    std::list<IRNode*>* stats = new std::list<IRNode*>();
    
    
    for(vector<IRNode*>::iterator it = l->getChildren()->begin(); it != l->getChildren()->end(); ++it){
        Symbol* label = static_cast<StatList*>(*it)->getLabel();
        
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

