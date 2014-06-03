/* 
 * File:   ir.h
 * Author: daniele
 *
 * Created on 13 maggio 2014, 15.53
 */

#ifndef IR_H
#define	IR_H

#include<map>
#include<vector>
#include<list>
#include<string>
#include<sstream>
#include<iostream>

#include "lexer.h"

using namespace std;

static int id = 0;

class BaseType {
public:

    enum basetypes {
        INT,
        FLOAT,
        LABEL,
        STRUCT,
        FUNCTION

    };

private:
    BaseType();
};

class Type {
public:

    Type() : name("Null"), size(0), basetype(0) {
    }

    Type(string _name, size_t _size, int _basetype)
    : name(_name), size(_size), basetype(_basetype) {
    }

    size_t getSize() const {
        return size;
    }

    const string& getName() {
        return name;
    }

private:
    string name;
    size_t size;
    int basetype;

};

class ArrayType : public Type {
public:

    ArrayType() : Type() {
    }

    ArrayType(string name, size_t size, int basetype) :
    Type(name, size * 32, basetype) {
    };

};


static std::map <int, Type*> standard_types;

class Value {
public:

    Value(int val) : int_value(val) {
    }

	int getValue(){
		return int_value;
	}

private:

    int int_value;

};

class Symbol {
public:

    Symbol() : value(0), spilled(false) {
    }

    Symbol(string _name) : name(_name), value(0), spilled(false) {
    }

    Symbol(string _name, Type* _stype, Value _value = NULL)
    : name(_name), stype(_stype), value(_value), spilled(false) {
    }

    const string& getName() const {
        return name;
    }

    const Value& getValue() const {
        return value;
    }

    Type& getType() const {
        return *stype;
    }

    void setValue(Value val) {
        value = val;
    }

    void setTarget(void* _target) {
        target = _target;
    }

    void* getTarget() {
        return target;
    }

	void spill(){
		spilled = true;
	}

	bool isSpilled(){
		return spilled;
	}

    ~Symbol() {
        if (stype)
            delete stype;
    }

    static string genUniqueId() {

		static int ids = 0;

        string name; // string which will contain the result

        ostringstream convert; // stream used for the conversion

        convert << "Temp" << ++ids;

        name = convert.str();

        return name;

    }

private:
	bool spilled;
    const string name;
    Type* stype;
    Value value;
    void* target;
};

class SymbolTable {
public:

    SymbolTable(SymbolTable* lhs = NULL, SymbolTable* rhs = NULL) {

        if (lhs != NULL && rhs != NULL) {

            for (SymbolTable::iterator it = lhs->begin(); it != lhs->end(); ++it) {
                table[it->first] = it->second;
            }

            for (SymbolTable::iterator it = rhs->begin(); it != rhs->end(); ++it) {
                table[it->first] = it->second;
            }
        }

    }

    ~SymbolTable() {
        for (std::map<std::string, Symbol*>::iterator it = table.begin(); it != table.end(); ++it) {
            delete it->second;
        }
    }

    /**
     * find the element in the Symbol Table
     * if not found return null
     * if double return second element
     * @param symbol_name
     * @return 
     */
    Symbol* find(std::string symbol_name) {
        if (table.find(symbol_name) != table.end())
            return table[symbol_name];
        else {
            cout << "Error: symbol " << symbol_name << " not found" << endl;
            return NULL;
        }
    }

    void exclude() {

    }

    void append(Symbol *symbol) {
        table[symbol->getName()] = symbol;
    }

    typedef std::map<std::string, Symbol*>::iterator iterator;

    iterator begin() {
        return table.begin();
    }

    iterator end() {
        return table.end();
    }


private:
    std::map<std::string, Symbol*> table;
};

class LabelType : public Type {
public:

    LabelType() : Type("Label", 0, BaseType::LABEL) {
    };

    Symbol* Label(void* target = NULL) {

        Symbol* sym;

        static int ids = 0;

        string name; // string which will contain the result

        ostringstream convert; // stream used for the conversion

        convert << "label" << ++ids;

        name = convert.str();

        sym = new Symbol(name, this);

        sym->setTarget(target);

        return sym;
    }
};

class FunctionType : public Type {
public:

    FunctionType() : Type("Function", 0, BaseType::FUNCTION) {
    };
};

class IRNode {
public:

    IRNode(IRNode* _parent = NULL, vector<IRNode*>* _children = NULL, SymbolTable* _symtab = NULL) :
    parent(_parent), children(_children), symtab(_symtab) {
        myId = ++id;
    }

    virtual ~IRNode() {

        for (vector<IRNode*>::iterator it = children->begin(); it != children->end(); ++it) {
            delete *it;
        }

        delete children;
        delete parent;

        for (vector<IRNode*>::iterator it = delete_list.begin(); it != delete_list.end(); ++it)
            delete *it;

    }

    /**
     * return node parent, if not exists, return null
     * @return *IRNode
     */
    IRNode* getParent() {
        return parent;
    }

    void setParent(IRNode* par) {
        parent = par;
    }

    bool hasChildren() {
        return (children != NULL);
    }

    void addChildren(IRNode* node) {
        if (!hasChildren()) {
            children = new vector<IRNode*>();
        }

        children->push_back(node);
    }

    vector<IRNode*>* getChildren() const {
        return children;
    }

    void setChildren(vector<IRNode*>* child) {
        if (hasChildren()) {
            delete children;
        }
        children = child;
    }

    SymbolTable* getSymTab() {
        return symtab;
    }

    /**
     * return uses, if not implemented return NULL
     */
    virtual std::list<Symbol*>& get_uses() {
        return uses;
    }

    virtual void replace_uses(Symbol* old, Symbol *sym) {
        if (hasChildren()) {
            for (vector<IRNode*>::size_type i = 0; i < children->size(); ++i) {
                (children->at(i))->replace_uses(old, sym);
            }
			uses.clear();

			for (vector<IRNode*>::size_type i = 0; i < getChildren()->size(); ++i) {
				uses.merge((getChildren()->at(i)->get_uses()));
			}
        }
    }

    void repr(int space = 0);

    virtual void lower() {
    }

    virtual void flatten() {

    }

    void lowering() {

        if (hasChildren()) {

            for (vector<IRNode*>::size_type i = 0; i < children->size(); ++i) {
                (children->at(i))->lowering();
            }
        }

        lower();
    }

    void flattening() {
        if (hasChildren()) {

            for (vector<IRNode*>::size_type i = 0; i < children->size(); ++i) {
                (children->at(i))->flattening();
            }
        }

        flatten();
    }

    void getStatLists(list<IRNode*>& statlists) {

        if (hasChildren()) {

            for (vector<IRNode*>::size_type i = 0; i < children->size(); ++i) {
                (children->at(i))->getStatLists(statlists);
            }
        }

        if (NodeType() == "StatList")
            statlists.push_back(this);
    }

    /**
     * Replace with elimination
     * @param old_node
     * @param new_node
     */
    void replace(IRNode* old_node, IRNode* new_node);

    /**
     * Replace without elimination
     * @param old_node
     * @param new_node
     */
    void substitute(IRNode* old_node, IRNode* new_node);

    virtual const string& NodeType() {
        static string s = "IRNode";

        return s;
    }

    int Id() const {
        return myId;
    }

    virtual void lower_expr(std::list<IRNode*>* stack) {
    }

private:
    int myId;
    IRNode* parent;
    vector<IRNode*>* children;
    vector<IRNode*> delete_list;
    SymbolTable* symtab;
    std::list<Symbol*> uses;

};

class Var : public IRNode {
public:

    Var(Symbol* _symbol, SymbolTable* symtab) : symbol(_symbol), IRNode(NULL, NULL, symtab) {

    }

    virtual std::list<Symbol*>& get_uses() {
        if (!IRNode::get_uses().size())
            IRNode::get_uses().push_back(symbol);

        return IRNode::get_uses();
    }
    
    virtual void replace_uses(Symbol *old, Symbol* sym) {
		if(symbol == old){
			symbol = sym;
			IRNode::get_uses().clear();
			IRNode::get_uses().push_back(symbol);
		}
    }

    virtual const string& NodeType() {
        static string s = "Var";

        return s;
    }

	Symbol* getSymbol(){
		return symbol;
	}

private:
    Symbol* symbol;
};

class Const : public IRNode {
public:

    Const(Value _value, SymbolTable* symtab) : value(_value), IRNode(NULL, NULL, symtab) {
    };

    virtual const string& NodeType() {
        static string s = "Const";

        return s;
    }

    Value& getValue() {
        return value;
    }
private:
    Value value;
};

class ArrayVar : public IRNode {
public:

    ArrayVar(Symbol* _symbol, IRNode* index, SymbolTable* symtab) :
    symbol(_symbol), IRNode(NULL, NULL, symtab) {

        index->setParent(this);
        vector<IRNode*>* children = new vector<IRNode*>();
        children->push_back(index);
        setChildren(children);
    }

    virtual std::list<Symbol*>& get_uses() {

        if (hasChildren() && !IRNode::get_uses().size()) {

            for (vector<IRNode*>::size_type i = 0; i < getChildren()->size(); ++i) {
                IRNode::get_uses().merge((getChildren()->at(i)->get_uses()));
            }

			IRNode::get_uses().push_back(symbol);

        }

        return IRNode::get_uses();
    }

	 virtual void replace_uses(Symbol *old, Symbol* sym) {
		if(symbol == old){
			symbol = sym;
		}
		IRNode::get_uses().clear();

		if (hasChildren()) 
			for (vector<IRNode*>::size_type i = 0; i < getChildren()->size(); ++i) {
				getChildren()->at(i)->replace_uses(old, sym);
				IRNode::get_uses().merge((getChildren()->at(i)->get_uses()));
			}
		IRNode::get_uses().push_back(symbol);
		
    }

    virtual const string& NodeType() {
        static string s = "ArrayVar";

        return s;
    }

private:
    Symbol* symbol;
};

class Tok : public IRNode {
public:

    Tok(int _tok, SymbolTable* symtab) : tok(_tok), IRNode(NULL, NULL, symtab) {
    }

	int getOP(){
		return tok;
	}

    virtual const string& NodeType() {
        static string s = "Tok";

        return s;
    }

private:
    int tok;

};

class Expr : public IRNode {
public:

    Expr(int _op, SymbolTable* symtab)
    : IRNode(NULL, NULL, symtab), op(_op, symtab) {
    }

    IRNode* getOperator() const {
        return getChildren()->at(0);
    }

    virtual std::list<Symbol*>& get_uses() {

        if (hasChildren() && !IRNode::get_uses().size()) {

            for (vector<IRNode*>::size_type i = 0; i < getChildren()->size(); ++i) {
                IRNode::get_uses().merge((getChildren()->at(i)->get_uses()));
            }
        }

        return IRNode::get_uses();
    }

	virtual void replace_uses(Symbol *old, Symbol *sym) {

		IRNode::get_uses().clear();

		if (hasChildren()) {

            for (vector<IRNode*>::size_type i = 0; i < getChildren()->size(); ++i) {
				getChildren()->at(i)->replace_uses(old, sym);
                IRNode::get_uses().merge((getChildren()->at(i)->get_uses()));
            }
        }
    }

    virtual const string& NodeType() {
        static string s = "Expr";

        return s;
    }

    virtual void lower_expr(std::list<IRNode*>* stack) {
    }

private:
    Tok op;

};

class Stat : public IRNode {
public:

    Stat(SymbolTable* symtab) :
    IRNode(NULL, NULL, symtab), label(NULL) {
    }

    void setLabel(Symbol* _label) {
        label = _label;
        label->setTarget(this);
    }

    Symbol* getLabel() {
        return label;
    }

    virtual Symbol* getSymbol() {
        return NULL;
    }

    /**
     *return null if global
     *@return *IRNode
     */
    IRNode* getFunction() {
        if (getParent()) {
            if (getParent()->NodeType() == "FunctionDef")
                return getParent();
            else
                return static_cast<Stat*> (getParent())->getFunction();
        } else {
            return NULL;
        }
    }

    virtual ~Stat() {
        if (label)
            delete label;
    }

    virtual void lower_expr(std::list<IRNode*>* stack) {

    }

    virtual const string& NodeType() {
        static string s = "Stat";

        return s;
    }

private:
    Symbol* label;
};

class AssignStat : public Stat {
public:

    AssignStat(Symbol* _sym, IRNode* expr, SymbolTable* symtab)
    : Stat(symtab), sym(_sym) {

        expr->setParent(this);

        vector<IRNode*> *children = new vector<IRNode*>();
        children->push_back(expr);

        setChildren(children);
    }

    virtual std::list<Symbol*>& get_uses() {

        if (hasChildren() && !IRNode::get_uses().size()) {

            for (vector<IRNode*>::size_type i = 0; i < getChildren()->size(); ++i) {
                IRNode::get_uses().merge((getChildren()->at(i)->get_uses()));
            }
        }

        return IRNode::get_uses();
    }

    virtual Symbol* getSymbol() {
        return sym;
    }

    void setSymbol(Symbol* _sym) {
        sym = _sym;
    }

    virtual const string& NodeType() {
        static string s = "AssignStat";

        return s;
    }

    virtual void lower_expr(std::list<IRNode*>* stack) {

        if (hasChildren()) {

            for (vector<IRNode*>::size_type i = 0; i < getChildren()->size(); ++i) {
                getChildren()->at(i)->lower_expr(stack);
            }
        }
    }

private:
    Symbol* sym;
};

class BinExpr : public Expr {
public:

    BinExpr(int _op, vector<IRNode*>* children, SymbolTable* symtab) :
    Expr(_op, symtab) {

        for (vector<IRNode*>::size_type i = 0; i < children->size(); ++i)
            children->at(i)->setParent(this);

        setChildren(children);

    }

    virtual const string& NodeType() {
        static string s = "BinExpr";

        return s;
    }

    virtual void lower_expr(std::list<IRNode*>* stack);

    void lowerBinExpr(std::list<IRNode*>* stack);

};

class UnExpr : public Expr {
public:

    UnExpr(int tok, IRNode* expr, SymbolTable* symtab) : Expr(tok, symtab) {
        Tok* tok_node = new Tok(tok, symtab);

        tok_node->setParent(this);
        expr->setParent(this);

        vector<IRNode*>* children = new vector<IRNode*>();
        children->push_back(tok_node);
        children->push_back(expr);

        setChildren(children);
    }

    virtual const string& NodeType() {
        static string s = "UnExpr";

        return s;
    }

    virtual void lower_expr(std::list<IRNode*>* stack);

    void lowerUnExpr(std::list<IRNode*>* stack);

};

class CallExpr : public Expr {
public:

    CallExpr(Symbol* sym, int op, vector<IRNode*>* parameters, SymbolTable* symtab) :
    Expr(op, symtab), function(sym) {
        for (vector<IRNode*>::size_type i = 0; i < parameters->size(); ++i)
            parameters->at(i)->setParent(this);

        setChildren(parameters);
    }

    virtual const string& NodeType() {
        static string s = "CallExpr";

        return s;
    }

	virtual Symbol* getSymbol(){
		return function;
	}

private:

    Symbol* function;


};

class LoadStat : public Stat {
public:

    LoadStat(Symbol* _sym, IRNode* _expr, SymbolTable* symtab) :
    Stat(symtab) {
        _expr->setParent(this);
        vector<IRNode*>* children = new vector<IRNode*>();
        children->push_back(_expr);
        setChildren(children);
        sym = _sym;
    }

    virtual const string& NodeType() {
        static string s = "Load";

        return s;
    }

    virtual void lower_expr(std::list<IRNode*>* stack) {

        if (hasChildren()) {

            for (vector<IRNode*>::size_type i = 0; i < getChildren()->size(); ++i) {
                getChildren()->at(i)->lower_expr(stack);
            }
        }
    }

	virtual Symbol* getSymbol(){
		return sym;
	}

	virtual std::list<Symbol*>& get_uses() {
        return IRNode::get_uses();
    }
    
private:
    Symbol* sym;
};

class StoreStat : public Stat {
public:

    StoreStat(Symbol* _sym, IRNode* expr_right, SymbolTable* symtab) :
    sym(_sym), Stat(symtab) {
        expr_right->setParent(this);
        vector<IRNode*>* children = new vector<IRNode*>();
        children->push_back(expr_right);
        setChildren(children);
    }

    virtual std::list<Symbol*>& get_uses() {
        if (!IRNode::get_uses().size())
			IRNode::get_uses().merge(getChildren()->at(0)->get_uses());
        return IRNode::get_uses();
    }

    virtual Symbol* getSymbol() {
        return sym;
    }

    virtual const string& NodeType() {
        static string s = "StoreStat";

        return s;
    }

    virtual void lower_expr(std::list<IRNode*>* stack) {

        if (hasChildren()) {

            for (vector<IRNode*>::size_type i = 0; i < getChildren()->size(); ++i) {
                getChildren()->at(i)->lower_expr(stack);
            }
        }
    }

private:
    Symbol* sym;
};

class BranchStat : public Stat {
public:

    BranchStat(IRNode* _expr, Symbol* _exit, SymbolTable* symtab) :
    exit(_exit), Stat(symtab) {
        _expr->setParent(this);
        vector<IRNode*>* children = new vector<IRNode*>();
        children->push_back(_expr);
        setChildren(children);

    }

    virtual std::list<Symbol*>& get_uses() {

        if (hasChildren() && !IRNode::get_uses().size()) {

            for (vector<IRNode*>::size_type i = 0; i < getChildren()->size(); ++i) {
                IRNode::get_uses().merge((getChildren()->at(i)->get_uses()));
            }
        }

        return IRNode::get_uses();
    }

    virtual const string& NodeType() {
        static string s = "Branch";

        return s;
    }

    virtual Symbol* getSymbol() {
        return exit;
    }

    bool isUnconditional() {
        if (getChildren()->at(0)->NodeType() == "Const") {
            return true;
        }

        return false;
    }

    virtual void lower_expr(std::list<IRNode*>* stack) {

        if (hasChildren()) {

            for (vector<IRNode*>::size_type i = 0; i < getChildren()->size(); ++i) {
                getChildren()->at(i)->lower_expr(stack);
            }
        }
    }

private:

    Symbol* exit;
};

class StatList : public Stat {
public:

    StatList(SymbolTable* symtab)
    : Stat(symtab) {

        cout << "StatList new " << Id() << endl;
    }

    void append(IRNode* stat) {

        stat->setParent(this);

        cout << "Append stat " << stat->Id() << " to " << Id() << endl;

        addChildren(stat);
    }

    /**
     * Makes one statlist from  nested statlists
     */
    virtual void flatten() {
        //TODO review the code
        if (getParent()->NodeType() == "StatList") {
            cout << "flattening of StatList " << Id() << "in " << getParent()->Id() << endl;

            printContent();
            static_cast<StatList*> (getParent())->printContent();

            if (getLabel()) {
                static_cast<Stat*> (getChildren()->at(0))->setLabel(getLabel()); //??delete
            }

            for (vector<IRNode*>::size_type i = 1; i < getChildren()->size(); ++i) {
                getChildren()->at(i)->setParent(getParent());
            }

            vector<IRNode*>* childrenParent = getParent()->getChildren();

            vector<IRNode*>::iterator it = childrenParent->begin();
            vector<IRNode*>::iterator it2 = getChildren()->begin();

            while (*it != this) {
                it++;
            }

            it++;
            it2++;

            for (; it2 != getChildren()->end(); ++it, ++it2) {
                childrenParent->insert(it, *it2);
                it = childrenParent->begin();
                while (*it != *it2) {
                    it++;
                }
            }

            getParent()->replace(this, getChildren()->at(0));


            static_cast<StatList*> (getParent())->printContent();
        }
    }

    void printContent() {
        if (hasChildren()) {

            cout << "StatList " << Id() << ": ";

            cout << "[";

            for (vector<IRNode*>::size_type i = 0; i < getChildren()->size(); ++i) {
                cout << getChildren()->at(i)->Id() << " ";
            }

            cout << "]" << endl;
        }
    }

    virtual const string& NodeType() {
        static string s = "StatList";

        return s;
    }
};

class AssignArrayStat : public Stat {
public:

    AssignArrayStat(Symbol* _sym, IRNode* expr_left, IRNode* expr_right, SymbolTable* symtab)
    : Stat(symtab), sym(_sym) {

        expr_left->setParent(this);
        expr_right->setParent(this);

        vector<IRNode*> *children = new vector<IRNode*>();
        children->push_back(expr_left);
        children->push_back(expr_right);

        setChildren(children);
    }

    virtual void lower() {
        IRNode* expr_left = getChildren()->at(0);
        IRNode* expr_right = getChildren()->at(1);


        vector<IRNode*>* child = new vector<IRNode*>();
        child->push_back(expr_left);
        child->push_back(new Const(standard_types[BaseType::INT]->getSize(), getSymTab()));

        Expr* mul = new BinExpr(token::times, child, getSymTab());

        Var* array = new Var(sym, getSymTab());

        LoadStat* mem = new LoadStat(sym, array, getSymTab());

        vector<IRNode*>* child2 = new vector<IRNode*>();
        child2->push_back(mul);
        child2->push_back(mem);

        IRNode* sum = new BinExpr(token::plus, child2, getSymTab());

        Symbol* temp = new Symbol();

        AssignStat* assign_temp = new AssignStat(temp, sum, getSymTab());

        StoreStat* assign = new StoreStat(temp, expr_right, getSymTab());

        StatList *statements = new StatList(getSymTab());

        statements->append(assign_temp);
        statements->append(assign);

        statements->setParent(getParent());

        getParent()->replace(this, assign);

    }

    virtual const string& NodeType() {
        static string s = "AssignArrayStat";

        return s;
    }

private:
    Symbol* sym;

};

class CallStat : public Stat {
public:

    CallStat(CallExpr* expr, SymbolTable* symtab)
    : Stat(symtab), call(expr) {

        call->setParent(this);

        vector<IRNode*>* children = new vector<IRNode*>();
        children->push_back(call);

        setChildren(children);
    }

    virtual std::list<Symbol*>& get_uses() {

        if (!IRNode::get_uses().size()) {

            IRNode::get_uses().merge(getChildren()->at(0)->get_uses());

            for (SymbolTable::iterator it = getSymTab()->begin(); it != getSymTab()->end(); ++it) {
                if (it->second->getType().getName() != "Function" && it->second->getType().getName() != "Label")
                    IRNode::get_uses().push_back(it->second);
            }
        }



        return IRNode::get_uses();
    }

    virtual const string& NodeType() {
        static string s = "CallStat";

        return s;
    }



private:
    CallExpr* call;
};

class IfStat : public Stat {
public:

    IfStat(IRNode* cond, IRNode* then, SymbolTable* symtab)
    : Stat(symtab) {

        cond->setParent(this);
        then->setParent(this);

        vector<IRNode*>* children = new vector<IRNode*>();
        children->push_back(cond);
        children->push_back(then);

        setChildren(children);
    }

    ~IfStat() {
        //TODO chiamare ~Stat(){ 
        if (exitStat != NULL)
            delete exitStat;
    }

    virtual void lower() {

        cout << "lowering IfStat : " << Id() << endl;

        exitStat = new Stat(getSymTab());
        Symbol* exitLabel = static_cast<LabelType*> (standard_types[BaseType::LABEL])->Label();
        exitStat->setLabel(exitLabel);

        Symbol* thenLabel = static_cast<LabelType*> (standard_types[BaseType::LABEL])->Label();
        static_cast<Stat*> (getChildren()->at(1))->setLabel(thenLabel);

        BranchStat* branchExit =
                new BranchStat(new UnExpr(0, getChildren()->at(1), getSymTab()), thenLabel, getSymTab());

        StatList *statlist = new StatList(getSymTab());

        statlist->append(branchExit);
        statlist->append(getChildren()->at(1));
        statlist->append(exitStat);

        statlist->setParent(getParent());

        getParent()->replace(this, statlist);


    }

    virtual const string& NodeType() {
        static string s = "IfStat";

        return s;
    }

private:
    Stat* exitStat;
};

class WhileStat : public Stat {
public:

    WhileStat(IRNode *cond, IRNode *body, SymbolTable *symtab)
    : Stat(symtab) {

        cond->setParent(this);
        body->setParent(this);

        vector<IRNode*>* children = new vector<IRNode*>();
        children->push_back(cond);
        children->push_back(body);

        setChildren(children);
    }

    ~WhileStat() {
        if (exitStat != NULL)
            delete exitStat;
    }

    virtual void lower() {

        cout << "lowering WhileStat : " << Id() << endl;

        exitStat = new Stat(getSymTab());
        Symbol* exitLabel = static_cast<LabelType*> (standard_types[BaseType::LABEL])->Label();
        Symbol* entryLabel = static_cast<LabelType*> (standard_types[BaseType::LABEL])->Label();

        exitStat->setLabel(exitLabel);


        BranchStat* branchEntry =
                new BranchStat((getChildren())->at(0), exitLabel, getSymTab());

        IRNode* const_var = new Const(1, getSymTab());

        BranchStat* branchEnd =
                new BranchStat(const_var, entryLabel, getSymTab());

        branchEntry->setLabel(entryLabel);

        StatList* statlist = new StatList(getSymTab());

        statlist->append(branchEntry);
        statlist->append(getChildren()->at(1));
        statlist->append(branchEnd);
        statlist->append(exitStat);

        statlist->setParent(getParent());

        getParent()->replace(this, statlist);

    }

    virtual const string& NodeType() {
        static string s = "WhileStat";

        return s;
    }

private:
    Stat* exitStat;
};

class PrintStat : public Stat {
public:

    PrintStat(Symbol *_sym, SymbolTable *symtab)
    : sym(_sym), Stat(symtab) {

    }

    virtual std::list<Symbol*>& get_uses() {
        if (!IRNode::get_uses().size())
            IRNode::get_uses().push_back(sym);
        return IRNode::get_uses();
    }

	virtual void replace_uses(Symbol* old, Symbol* _sym) {
        if(old == sym){
			sym = _sym;
			IRNode::get_uses().clear();
			IRNode::get_uses().push_back(sym);
		}
    }

    virtual const string& NodeType() {
        static string s = "Print";

        return s;
    }
private:
    Symbol* sym;
};

class Definition : public IRNode {
public:

    Definition(Symbol* _symbol) : symbol(_symbol) {

    }

    virtual const string& NodeType() {
        static string s = "Def";

        return s;
    }

	virtual Symbol* getSymbol(){
		return symbol; 
	}

private:
    Symbol* symbol;

};

class DefinitionList : public IRNode {
public:

    void append(Definition* elem) {
        elem->setParent(this);

        cout << "Append stat " << elem->Id() << " to " << Id() << endl;

        addChildren(elem);
    }

    virtual const string& NodeType() {
        static string s = "DefList";

        return s;
    }

};

class Block : public Stat {
public:

    Block(SymbolTable* _gl_sym, SymbolTable* _lc_sym,
            IRNode* _body, DefinitionList* _defs) :
    Stat(_gl_sym), gl_sym(_gl_sym), lc_sym(_lc_sym), body(_body), defs(_defs) {
        body->setParent(this);
        defs->setParent(this);

        vector<IRNode*>* children = new vector<IRNode*>();
        children->push_back(body);
        children->push_back(defs);

        setChildren(children);
    }

    ~Block() {
        delete lc_sym;
    }

    virtual const string& NodeType() {
        static string s = "Block";

        return s;
    }

    SymbolTable& getGlobalSymbolTable() {
        return *gl_sym;
    }

private:
    IRNode* body;
    DefinitionList* defs;
    SymbolTable* gl_sym;
    SymbolTable* lc_sym;

};

class FunctionDef : public Definition {
public:

    FunctionDef(Symbol* _symbol, SymbolTable* _parameters, IRNode* _body)
    : Definition(_symbol), parameters(_parameters), body(_body) {

        body->setParent(this);

        vector<IRNode*>* children = new vector<IRNode*>();
        children->push_back(body);

        setChildren(children);

    }

    std::list<Symbol*>& getGlobalSymbol() {

        if (!global_symbol.size()) {

            Block *b = static_cast<Block*> (body);
            SymbolTable& symt = b->getGlobalSymbolTable();

            for (SymbolTable::iterator it = symt.begin(); it != symt.end(); ++it) {
                if (it->second->getType().getName() != "Function" && it->second->getType().getName() != "Label")
                    global_symbol.push_back(it->second);
            }
        }

        return global_symbol;
    }

    ~FunctionDef() {
        delete parameters;
        delete body;
    }

    virtual const string& NodeType() {
        static string s = "FunctionDef";

        return s;
    }

    const SymbolTable* getParameters() const {
        return parameters;
    }

private:
    const SymbolTable* parameters;
    IRNode* body;
    std::list<Symbol*> global_symbol;

};



#endif	/* IR_H */

