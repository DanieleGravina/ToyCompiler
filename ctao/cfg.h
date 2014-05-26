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

static int Id = 0;

class BasicBlock{
public:
    
    BasicBlock(std::list<IRNode*>* _stats = NULL, list<Symbol*>* _labels = NULL)
		:stats(_stats), labels(_labels), target(NULL), next(NULL), bb_target(NULL), total_var_used(0){

				myId = ++Id;

				std::set<Symbol*> uses;	

				if(static_cast<Stat*>(stats->back())->getLabel())
					target = static_cast<IRNode*>(static_cast<Stat*>(stats->back())->getLabel()->getTarget());

				kill.clear(); //assigned
				gen.clear();  // use before assign

				for(list<IRNode*>::iterator it = stats->begin(); it != stats->end(); ++it){
                                        
                                        
					for(list<Symbol*>::iterator it2 = (*it)->get_uses().begin(); it2 != (*it)->get_uses().end(); ++it2){
						uses.insert(*it2);
					}

					for(set<Symbol*>::iterator it3 = kill.begin(); it3 != kill.end(); ++it3){
						uses.erase(*it3);
					}

					for(set<Symbol*>::iterator it4 = uses.begin(); it4 != uses.end(); ++it4){
						gen.insert(*it4);
					}

					if((*it)->NodeType() == "AssignStat" || (*it)->NodeType() == "StoreStat"){
						kill.insert(static_cast<Stat*>(*it)->getSymbol());
					}

				}

				for(set<Symbol*>::iterator it = kill.begin(); it != kill.end(); ++it){
						gen.insert(*it);
				}

				total_var_used = gen.size();

	}
                
        ~BasicBlock(){
            if(next){
                next->~BasicBlock();
                delete next;
            }
            delete stats;
            delete labels;
        }

	bool liveness_iteration(){
		int lin = live_in.size();
		int lout = live_out.size();

		if(next){
			Union(getLiveOut(), next->getLiveIn());		
		}

		if(bb_target){
			Union(getLiveOut(), bb_target->getLiveIn());		
		}

		if(!next && !bb_target){
			if(getFunction()){ //not global
				std::list<Symbol*>& global = static_cast<FunctionDef*>(getFunction())->getGlobalSymbol();
				for(std::list<Symbol*>::iterator it = global.begin(); it != global.end(); ++it){
					live_out.insert(*it);
				}

			}
		}

		std::set<Symbol*> temp(live_out);

		Difference(temp, kill);

		Union(gen, temp);

		live_in = gen;

		return (lin == live_in.size() && lout == live_out.size());

	}

	IRNode* getFunction(){
		return static_cast<Stat*>(stats->front())->getFunction();
	}

	std::set<Symbol*>& getLiveOut(){
		return live_out;
	}

	std::set<Symbol*>& getLiveIn(){
		return live_in;
	}
        
        std::set<Symbol*>& getKill(){
            return kill;
        }
        
        std::set<Symbol*>& getGen(){
            return gen;
        }

	void repr(){

		cout << "BB " << myId << " instr : ";

		for(std::list<IRNode*>::iterator it2 = stats->begin(); it2 != stats->end(); ++it2){
			cout << (*it2)->Id() << " " << (*it2)->NodeType() << " ";
		}

		cout << endl;

	}
    
    
    
    /**
     * Return next block, null if not exits
     * @return BasicBlock*
     */
    BasicBlock* getNext(){
        return next;
    }

	BasicBlock* getBBTarget(){
		return bb_target;
	}
    
    void setNext(BasicBlock* next_block){
        next = next_block;
    }
	
	void setBBTarget(BasicBlock* tar){
		bb_target = tar;
	}

	IRNode* getTarget(){
		return target;
	}

	std::list<Symbol*>& getLabels(){
		return *labels;
	}

	std::list<IRNode*>& getStats(){
		return *stats;
	}

	void remove_useless_next(){

		IRNode* temp = stats->back();

		if(temp->NodeType() == "BranchStat" && static_cast<BranchStat*>(temp)->isUnconditional()){
			next = NULL;
		}
	}

	int getId() const{
		return myId;
	}
        
        static void Union(std::set<Symbol*>& lhs, std::set<Symbol*>& rhs){
		for(std::set<Symbol*>::iterator it = rhs.begin(); it != rhs.end(); ++it){
			lhs.insert(*it);
		}
	}

	static void Difference(std::set<Symbol*>& lhs, std::set<Symbol*>& rhs){
		for(std::set<Symbol*>::iterator it = rhs.begin(); it != rhs.end(); ++it){
			lhs.erase(*it);
		}
	}
    
private:
    
	int myId;
    BasicBlock* next;
    BasicBlock* bb_target;
	IRNode* target;
    std::list<IRNode*>* stats;
    std::list<Symbol*>* labels;
	std::set<Symbol*> live_in;
	std::set<Symbol*> live_out;
	std::set<Symbol*> kill;
	std::set<Symbol*> gen;
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
				stats = new std::list<IRNode*>();
				if(lBB->size())
					lBB->back()->setNext(bb);
                lBB->push_back(bb);
                labels = new std::list<Symbol*>();
            }
            labels->push_back(label);
        }
        
        stats->push_back(*it);
        
        if((*it)->NodeType() == "Branch" || (*it)->NodeType() == "CallStat"){
            if(stats->size()){ //start new bb
				bb = new BasicBlock(stats, labels);
				stats = new std::list<IRNode*>();
				if(lBB->size())
					lBB->back()->setNext(bb);
                lBB->push_back(bb);
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

class CFG{

public:
	CFG(IRNode* root){

		list<IRNode*> statlists;
        
        root->getStatLists(statlists);
        
        for(list<IRNode*>::iterator it = statlists.begin(); it != statlists.end(); ++it){
			std::list<BasicBlock*> *temp = IRtoBB(*it);
			
			for(list<BasicBlock*>::iterator it2 = temp->begin(); it2 != temp->end(); ++it2){
				cfg.push_back(*it2);

				(*it2)->repr();

				if((*it2)->getTarget() != NULL){
					Symbol *label = static_cast<Stat*>((*it2)->getTarget())->getLabel();
					(*it2)->setBBTarget(find_target_bb(label));
				}

				(*it2)->remove_useless_next();
			}

			delete temp;
        }
        
	}

	std::map<int, BasicBlock*>* heads(){

		std::list<BasicBlock*> defs;
		std::map<int, BasicBlock*>* res = new std::map<int, BasicBlock*>();

		bool head = true;
		for(list<BasicBlock*>::iterator it = cfg.begin(); it != cfg.end(); ++it){
			head = true;
			for(list<BasicBlock*>::iterator it2 = cfg.begin(); it2 != cfg.end(); ++it2){
				if((*it2)->getNext() == *it || (*it2)->getBBTarget() == *it){
					head = false;
					break;
				}
				if(head) 
					defs.push_back(*it);
			}

			for(list<BasicBlock*>::iterator it3 = defs.begin(); it3 != defs.end(); ++it3){
				IRNode* first = (*it3)->getStats().front();
				IRNode* parent = first->getParent();

				while(parent != NULL && parent->NodeType() != "FunctionDef"){
					parent = parent->getParent();
				}

				if(parent == NULL){
					(*res)[0] = *it3;
				}else{
					(*res)[parent->Id()] =  *it3;
				}
			}
		}

		return res;
	}

	void liveness(){

		int counter = 0;

		while(counter != cfg.size()){
			counter = 0;
			for(list<BasicBlock*>::iterator it = cfg.begin(); it != cfg.end(); ++it){
				if ((*it)->liveness_iteration())
					counter++;
			}
		}

	}

	void print_liveness(){

		for(list<BasicBlock*>::iterator it = cfg.begin(); it != cfg.end(); ++it){
			cout << "BB " << (*it)->getId() << endl;

			if((*it)->getNext() != NULL){
				cout << "next " << (*it)->getNext()->getId() << endl;
			}

			if((*it)->getBBTarget() != NULL){
				cout << "BB target " << (*it)->getBBTarget() ->getId() << endl;
			}

			(*it)->repr();

			cout << "live_in : ";
			for(std::set<Symbol*>::iterator it2 = (*it)->getLiveIn().begin(); it2 != (*it)->getLiveIn().end(); ++it2){
				cout << (*it2)->getName() << " ";
			}
			cout << endl;

			cout << "live_out : ";

			for(std::set<Symbol*>::iterator it2 = (*it)->getLiveOut().begin(); it2 != (*it)->getLiveOut().end(); ++it2){
				cout << (*it2)->getName() << " ";
			}

			cout << endl;
		}

	}

	BasicBlock* find_target_bb(Symbol* label){
		for(list<BasicBlock*>::iterator it = cfg.begin(); it != cfg.end(); ++it){
			std::list<Symbol*>& labels = (*it)->getLabels();
			for(list<Symbol*>::iterator it2 = labels.begin(); it2 != labels.end(); ++it2){
				if(*it2 == label){
					return *it;
				}
			}
		}

		cout << "Error cfg, label "<< label->getName() << " not found" << endl;
	}
        
        typedef std::list<BasicBlock*>::iterator iterator;
        
        iterator begin(){
            return cfg.begin();
        }
        
        iterator end(){
            return cfg.end();
        }

private:
	std::list<BasicBlock*> cfg;
};



#endif	/* CFG_H */

