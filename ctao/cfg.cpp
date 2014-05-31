#include "cfg.h"

/**
 * BasicBlock
 */

BasicBlock::BasicBlock(std::list<IRNode*>* _stats = NULL, list<Symbol*>* _labels = NULL)
: stats(_stats), labels(_labels), target(NULL), next(NULL), bb_target(NULL), total_var_used(0) {

    myId = ++Id;

    std::set<Symbol*> uses;

    if (stats->back()->NodeType() == "Branch")
        target = static_cast<IRNode*> (static_cast<BranchStat*> (stats->back())->getSymbol()->getTarget());

    kill.clear(); //assigned
    gen.clear(); // use before assign

    for (list<IRNode*>::iterator it = stats->begin(); it != stats->end(); ++it) {


        for (list<Symbol*>::iterator it2 = (*it)->get_uses().begin(); it2 != (*it)->get_uses().end(); ++it2) {
            uses.insert(*it2);
        }

        for (set<Symbol*>::iterator it3 = kill.begin(); it3 != kill.end(); ++it3) {
            uses.erase(*it3);
        }

        for (set<Symbol*>::iterator it4 = uses.begin(); it4 != uses.end(); ++it4) {
            gen.insert(*it4);
        }

        if ((*it)->NodeType() == "AssignStat" || (*it)->NodeType() == "StoreStat") {
            kill.insert(static_cast<Stat*> (*it)->getSymbol());
        }

    }

    set<Symbol*> temp;

    Union(temp, gen);
    Union(temp, kill);

    total_var_used = temp.size();

}

/**
 * IR to BasicBlock 
 */
std::list<BasicBlock*>* IRtoBB(IRNode* l) {

    BasicBlock* bb = NULL;
    std::list<BasicBlock*>* lBB = new std::list<BasicBlock*>();
    std::list<Symbol*>* labels = new std::list<Symbol*>();
    std::list<IRNode*>* stats = new std::list<IRNode*>();

    IRNode* temp;
    std::list<IRNode*> stack;


    for (vector<IRNode*>::iterator it = l->getChildren()->begin(); it != l->getChildren()->end(); ++it) {
        Symbol* label = static_cast<Stat*> (*it)->getLabel();

        if (label != NULL) {

            if (stats->size()) { //start new bb
                bb = new BasicBlock(stats, labels);
                stats = new std::list<IRNode*>();
                if (lBB->size())
                    lBB->back()->setNext(bb);
                lBB->push_back(bb);
                labels = new std::list<Symbol*>();
            }

            labels->push_back(label);
        }

        (*it)->lower_expr(&stack);

        for (std::list<IRNode*>::reverse_iterator it2 = stack.rbegin(); it2 != stack.rend(); ++it2) {
            stats->push_back(*it2);
        }
        
        stats->push_back(*it);
        
        for (std::list<IRNode*>::iterator it2 = stats->begin(); it2 != stats->end(); ++it2) {
            (*it2)->repr();
        }

        stack.clear();


        if ((*it)->NodeType() == "Branch" || (*it)->NodeType() == "CallStat") {

            if (stats->size()) { //start new bb
                bb = new BasicBlock(stats, labels);
                stats = new std::list<IRNode*>();
                if (lBB->size())
                    lBB->back()->setNext(bb);
                lBB->push_back(bb);
                labels = new std::list<Symbol*>();
            }
        }
    }

    if (stats->size() || labels->size()) {
        bb = new BasicBlock(stats, labels);
        if (lBB->size())
            lBB->back()->setNext(bb);
        lBB->push_back(bb);
    }

    return lBB;


}
