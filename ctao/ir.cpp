#include "ir.h"

/**
* IRNode
*/

void IRNode::repr(int space) {

	string parent = "Global";
	string id = "";
	ostringstream convert;

	if (getParent() != NULL) {
		parent = getParent()->NodeType();
		convert << getParent()->Id();
		id = convert.str();
	}

	cout << "Node: " << NodeType() << " " << Id() << " parent: " << parent << " " << id << endl;

	if (hasChildren()) {

		space++;

		for (vector<IRNode*>::iterator it = children->begin(); it != children->end(); ++it) {
			int index = space;
			while (index != 0) {
				index--;
				cout << "  ";
			}
			(*it)->repr(space);
		}
	}
}

void IRNode::replace(IRNode* old_node, IRNode* new_node) {

	cout << "Replace in  " << Id() << " children: " << old_node->Id() << "with " << new_node->Id() << endl;

	if (hasChildren()) {
		for (vector<IRNode*>::size_type i = 0; i < children->size(); ++i) {
			if (children->at(i) == old_node) {
				children->at(i) = new_node;
				delete_list.push_back(old_node);
			}
		}
	} else {
		children = new vector<IRNode*>();
		children->push_back(new_node);
	}

}

void IRNode::substitute(IRNode* old_node, IRNode* new_node) {

	cout << "Substitute in  " << Id() << " children: " << old_node->Id() << "with " << new_node->Id() << endl;

	if (hasChildren()) {
		for (vector<IRNode*>::size_type i = 0; i < children->size(); ++i) {
			if (children->at(i) == old_node) {
				children->at(i) = new_node;
			}
		}
	} else {
		children = new vector<IRNode*>();
		children->push_back(new_node);
	}
}

/**
* ArrayVar
*/

void ArrayVar::lower() {
	Symbol* index = new Symbol(Symbol::genUniqueId());
	Symbol* result = new Symbol(Symbol::genUniqueId());
	getSymTab()->append(index);
	getSymTab()->append(result);

	std::vector<IRNode*>* child = new std::vector<IRNode*>();
	child->push_back(new Tok(token::times, getSymTab()) );
	child->push_back(new Var(index, getSymTab()) );
	child->push_back(new Const(4, getSymTab()));

	Var* result_var = new Var(result, getSymTab());

	IRNode* assign = new AssignStat(index, getChildren()->at(0), getSymTab());

	IRNode* mul = new BinExpr(token::times, child , getSymTab());

	IRNode* new_index = new AssignStat(index, mul, getSymTab());

	vector<IRNode*>* childs = new vector<IRNode*>();
	childs->push_back(new Tok(token::plus, getSymTab()));
	childs->push_back(new Var(index, getSymTab()));
	childs->push_back(new Const(0, getSymTab(), symbol));

	IRNode* sum = new BinExpr(token::times, childs, getSymTab());

	IRNode* index_sum = new AssignStat(index, sum, getSymTab());

	IRNode* load = new LoadStat(result, new Var(symbol, getSymTab()), new Var(index, getSymTab()), getSymTab());

	IRNode* parent = getParent();
	IRNode* current = this;

	parent->replace(this, result_var);

	while(parent->NodeType() != "StatList"){
		current = parent;
		parent = parent->getParent();
	}

	StatList *statements = new StatList(getSymTab());

	statements->append(assign);
	statements->append(new_index);
	statements->append(index_sum);
	statements->append(load);

	statements->setParent(parent);

	parent->insertChildren(current, statements);
}

/**
* BinExpr
*/
void BinExpr::lower_expr(std::list<IRNode*>* stack) {


	if (getChildren()->at(1)->NodeType() == "Const" || getChildren()->at(1)->NodeType() == "Var") {

		if (getChildren()->at(2)->NodeType() == "Const" || getChildren()->at(2)->NodeType() == "Var") {
			//i'm in a leaf
			if (getParent()->NodeType() == "BinExpr" || getParent()->NodeType() == "UnExpr")
				lowerBinExpr(stack);
		} else {
			getChildren()->at(2)->lower_expr(stack);
			if (getParent()->NodeType() == "BinExpr" || getParent()->NodeType() == "UnExpr")
				lowerBinExpr(stack);
		}

	} else {

		if (getChildren()->at(2)->NodeType() == "Const" || getChildren()->at(2)->NodeType() == "Var") {
			if (getParent()->NodeType() == "BinExpr" || getParent()->NodeType() == "UnExpr")
				lowerBinExpr(stack);
		} else {
			getChildren()->at(2)->lower_expr(stack);
			if (getParent()->NodeType() == "BinExpr" || getParent()->NodeType() == "UnExpr")
				lowerBinExpr(stack);
		}
	}

}

void BinExpr::lowerBinExpr(std::list<IRNode*>* stack) {
	Symbol* temp_sym = new Symbol(Symbol::genUniqueId());
	IRNode* temp_var = new Var(temp_sym, getSymTab());

	getSymTab()->append(temp_sym);

	temp_var->setParent(getParent());

	getParent()->substitute(this, temp_var);

	IRNode* temp = new AssignStat(temp_sym, static_cast<IRNode*> (this), getSymTab());

	stack->push_back(temp);
}

/**
* UnExpr
*/

void UnExpr::lower_expr(std::list<IRNode*>* stack) {

	if (getChildren()->at(1)->NodeType() == "Const" || getChildren()->at(1)->NodeType() == "Var") {
		//i'm in a leaf
		if (getParent()->NodeType() == "BinExpr" || getParent()->NodeType() == "UnExpr")
			lowerUnExpr(stack);
	} else {
		getChildren()->at(1)->lower_expr(stack);
		if (getParent()->NodeType() == "BinExpr" || getParent()->NodeType() == "UnExpr")
			lowerUnExpr(stack);
	}

}

void UnExpr::lowerUnExpr(std::list<IRNode*>* stack) {
	Symbol* temp_sym = new Symbol(Symbol::genUniqueId());
	IRNode* temp_var = new Var(temp_sym, getSymTab());

	getSymTab()->append(temp_sym);

	temp_var->setParent(getParent());
	getParent()->substitute(this, temp_var);

	IRNode* temp = new AssignStat(temp_sym, static_cast<IRNode*> (this), getSymTab());
	stack->push_back(temp);
}