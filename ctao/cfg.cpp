#include "cfg.h"

/**
 * BasicBlock
 */

BasicBlock::BasicBlock(std::list<IRNode*>* _stats = NULL, list<Symbol*>* _labels = NULL)
: stats(_stats), labels(_labels), target(NULL), next(NULL), bb_target(NULL), total_var_used(0) {

    myId = ++Id;
    
    init();

}

void BasicBlock::init() {
    std::set<Symbol*> uses;

    if (stats->back()->NodeType() == "Branch")
        target = static_cast<IRNode*> (static_cast<BranchStat*> (stats->back())->getSymbol()->getTarget());

    live_in.clear();
    live_out.clear();
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

bool BasicBlock::liveness_iteration() {

    int lin = live_in.size();
    int lout = live_out.size();

    if (next != NULL) {
        Union(getLiveOut(), next->getLiveIn());
    }

    if (bb_target != NULL) {
        Union(getLiveOut(), bb_target->getLiveIn());
    }

    if (next == NULL && bb_target == NULL) {
        if (getFunction()) { //not global
            std::list<Symbol*>& global = static_cast<FunctionDef*> (getFunction())->getGlobalSymbol();
            for (std::list<Symbol*>::iterator it = global.begin(); it != global.end(); ++it) {
                live_out.insert(*it);
            }

        }
    }

    std::set<Symbol*> temp;

    Union(temp, live_out);

    Difference(temp, kill);

    Union(temp, gen);

    live_in = temp;

    return (lin == live_in.size() && lout == live_out.size());

}

void BasicBlock::spill(Symbol* to_spill) {

    for (std::list<IRNode*>::iterator it = stats->begin(); it != stats->end(); ++it) {

        if ((*it)->NodeType() == "AssignStat") {
            AssignStat* assign = static_cast<AssignStat*> (*it);

            if (assign->getSymbol() == to_spill) {
                Symbol* temp = new Symbol(Symbol::genUniqueId());
                IRNode* store = new StoreStat(to_spill, new Var(temp, (*it)->getSymTab()), (*it)->getSymTab());

                assign->setSymbol(temp);

                (*it)->getSymTab()->append(temp);

                stats->insert(it, store);
                it = stats->begin(); //restart from first stat
            }
        }



        for (list<Symbol*>::iterator it2 = (*it)->get_uses().begin(); it2 != (*it)->get_uses().end(); ++it2) {
            if ((*it2) == to_spill) {
                Symbol* temp = new Symbol(Symbol::genUniqueId());
                IRNode* load = new LoadStat(to_spill, new Var(temp, (*it)->getSymTab()), (*it)->getSymTab());

                (*it)->replace_uses(temp);

                (*it)->getSymTab()->append(temp);

                stats->insert(--it, load);
                it = stats->begin(); //restart from first stat
            }
        }



    }

    init();

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

/**
 * CFG
 */

CFG::CFG(IRNode* root) {

    list<IRNode*> statlists;

    root->getStatLists(statlists);

    for (list<IRNode*>::iterator it = statlists.begin(); it != statlists.end(); ++it) {

        cout << "Statlist : " << (*it)->Id() << "to BBs" << endl;

        std::list<BasicBlock*> *temp = IRtoBB(*it);

        for (list<BasicBlock*>::iterator it2 = temp->begin(); it2 != temp->end(); ++it2) {
            cfg.push_back(*it2);
        }

        delete temp;
    }

    for (list<BasicBlock*>::iterator it2 = cfg.begin(); it2 != cfg.end(); ++it2) {

        (*it2)->repr();

        if ((*it2)->getTarget() != NULL) {
            Symbol *label = static_cast<Stat*> ((*it2)->getTarget())->getLabel();
            (*it2)->setBBTarget(find_target_bb(label));
        }

        (*it2)->remove_useless_next();
    }

}

void CFG::spill(Symbol* sym){
    for(CFG::iterator it = begin(); it != end(); ++it){
        (*it)->spill(sym);
    }
}

