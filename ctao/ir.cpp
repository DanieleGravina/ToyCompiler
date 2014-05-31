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