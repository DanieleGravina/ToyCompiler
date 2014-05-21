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
    
    BasicBlock(BasicBlock *_next = NULL, std::list<IRNode*>* _stats, list<Symbol*>* _labels = NULL)
            :next(_next), stats(_stats), labels(_labels){}
    
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
    
    void setNext(BasicBlock* _next){
        next = _next;
    }
    
private:
    BasicBlock *next;
    BasicBlock *branch;
    std::list<IRNode*>* stats;
    std::list<Symbol*>* labels;
};

std::list<BasicBlock*>* IRtoBB(list<Stat*> &l){
    
    BasicBlock* bb;
    std::list<BasicBlock*>* lBB = new std::list<BasicBlock*>();
    std::list<Symbol*>* labels = new std::list<Symbol*>();
    std::list<Stat*>* stats = new std::list<Stat*>();
    
    
    for(list<Stat*>::iterator it = l.begin(); it != l.end(); ++it){
        Symbol* label = *it->getLabel();
        
        if(label){
            
            if(bb){ //start new bb
                lBB->push_back(bb);
                bb = new BasicBlock(NULL, stats, labels);
                stats = new std::list<Stat*>();
                lBB->back()->setNext(bb);
                labels = new std::list<Symbol*>();
            }
            labels->push_back(label);
        }
        
        stats->push_back(*it);
        
        if(*it->NodeType() == "Branch" || *it->NodeType() == "CallStat"){
             if(bb){ //start new bb
                lBB->push_back(bb);
                bb = new BasicBlock(NULL, stats, labels);
                stats = new std::list<Stat*>();
                lBB->back()->setNext(bb);
                labels = new std::list<Symbol*>();
            }
        }
    }
    
    if(stats->size() || labels->size()){
        lBB->push_back(bb);
        bb = new BasicBlock(NULL, stats, labels);
        lBB->back()->setNext(bb);
        lBB->push_back(bb);
    }
    
    return lBB;
    
    
}



#endif	/* CFG_H */

