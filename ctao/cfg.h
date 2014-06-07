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

class BasicBlock {
public:

	BasicBlock(std::list<IRNode*>* _stats, list<Symbol*>* _labels);

	~BasicBlock() {
		if (next) {
			next->~BasicBlock();
			delete next;
		}
		delete stats;
		delete labels;
	}

	/**
	* Initialize gen and kill
	*/
	void init();

	/**
	* Perform one liveness iteration
	* @return true if completed, else false
	*/
	bool liveness_iteration();

	IRNode* getFunction() {
		return static_cast<Stat*> (stats->front())->getFunction();
	}

	std::set<Symbol*>& getLiveOut() {
		return live_out;
	}

	std::set<Symbol*>& getLiveIn() {
		return live_in;
	}

	std::set<Symbol*>& getKill() {
		return kill;
	}

	std::set<Symbol*>& getGen() {
		return gen;
	}

	void repr() {

		cout << "BB " << myId << " instr : ";

		for (std::list<IRNode*>::iterator it2 = stats->begin(); it2 != stats->end(); ++it2) {
			cout << (*it2)->Id() << " " << (*it2)->NodeType() << " ";
		}

		cout << endl;

	}

	/**
	* Return next block, null if not exits
	* @return BasicBlock*
	*/
	BasicBlock* getNext() {
		return next;
	}

	BasicBlock* getBBTarget() {
		return bb_target;
	}

	void setNext(BasicBlock* next_block) {
		next = next_block;
	}

	void setBBTarget(BasicBlock* tar) {
		bb_target = tar;
	}

	IRNode* getTarget() {
		return target;
	}

	std::list<Symbol*>& getLabels() {
		return *labels;
	}

	std::list<IRNode*>& getStats() {
		return *stats;
	}

	void remove_useless_next() {

		IRNode* temp = stats->back();

		if (temp->NodeType() == "BranchStat" && static_cast<BranchStat*> (temp)->isUnconditional()) {
			next = NULL;
		}
	}

	int getId() const {
		return myId;
	}

	static void Union(std::set<Symbol*>& lhs, std::set<Symbol*>& rhs) {
		for (std::set<Symbol*>::iterator it = rhs.begin(); it != rhs.end(); ++it) {
			lhs.insert(*it);
		}
	}

	static void Difference(std::set<Symbol*>& lhs, std::set<Symbol*>& rhs) {
		for (std::set<Symbol*>::iterator it = rhs.begin(); it != rhs.end(); ++it) {
			lhs.erase(*it);
		}
	}

	void spill(Symbol* to_spill);

	Symbol* getSymOfFunction(){
		return function_sym;
	}

	SymbolTable* parameters(){
		
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
	std::set<Symbol*> spilled;
	int total_var_used;

	Symbol* function_sym;

};

/**
* IR to basic block
* @param root of IR
* @return list of Basic Block
*/
std::list<BasicBlock*>* IRtoBB(IRNode* l);

class CFG {
public:

	CFG(std::list<BasicBlock*>& proc);

	void liveness() {

		int counter = 0;

		while (counter != cfg.size()) {
			counter = 0;
			for (list<BasicBlock*>::iterator it = cfg.begin(); it != cfg.end(); ++it) {
				if ((*it)->liveness_iteration())
					counter++;
			}
		}

	}

	void print_liveness() {

		for (list<BasicBlock*>::iterator it = cfg.begin(); it != cfg.end(); ++it) {
			cout << "BB " << (*it)->getId() << endl;

			if ((*it)->getNext() != NULL) {
				cout << "next " << (*it)->getNext()->getId() << endl;
			}

			if ((*it)->getBBTarget() != NULL) {
				cout << "BB target " << (*it)->getBBTarget() ->getId() << endl;
			}

			(*it)->repr();

			cout << "gen : ";
			for (std::set<Symbol*>::iterator it2 = (*it)->getGen().begin(); it2 != (*it)->getGen().end(); ++it2) {
				cout << (*it2)->getName() << " ";
			}
			cout << endl;

			cout << "kill : ";
			for (std::set<Symbol*>::iterator it2 = (*it)->getKill().begin(); it2 != (*it)->getKill().end(); ++it2) {
				cout << (*it2)->getName() << " ";
			}
			cout << endl;

			cout << "live_in : ";
			for (std::set<Symbol*>::iterator it2 = (*it)->getLiveIn().begin(); it2 != (*it)->getLiveIn().end(); ++it2) {
				cout << (*it2)->getName() << " ";
			}
			cout << endl;

			cout << "live_out : ";

			for (std::set<Symbol*>::iterator it2 = (*it)->getLiveOut().begin(); it2 != (*it)->getLiveOut().end(); ++it2) {
				cout << (*it2)->getName() << " ";
			}

			cout << endl;
		}

	}

	BasicBlock* find_target_bb(Symbol* label) {
		for (list<BasicBlock*>::iterator it = cfg.begin(); it != cfg.end(); ++it) {
			std::list<Symbol*>& labels = (*it)->getLabels();
			for (list<Symbol*>::iterator it2 = labels.begin(); it2 != labels.end(); ++it2) {
				if (*it2 == label) {

					return *it;
				}
			}
		}
		if (label)
			cout << "Error cfg, label " << label->getName() << " not found" << endl;
	}

	void spill(Symbol* sym);

	std::set<Symbol*>& varSpilled(){
		return spilled;
	}

	typedef std::list<BasicBlock*>::iterator iterator;

	iterator begin() {
		return cfg.begin();
	}

	iterator end() {
		return cfg.end();
	}

private:
	std::list<BasicBlock*> cfg;
	std::set<Symbol*> spilled;
};



#endif	/* CFG_H */

