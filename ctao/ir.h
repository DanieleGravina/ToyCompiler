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

#include "lexer.h"


class BaseType{
    
public:
    enum basetypes{
        INT, 
        FLOAT,
        LABEL, 
        STRUCT,
        FUNCTION
                
    };
    
private:
    BaseType();
};


class Type{

public :    
    Type(string& _name, size_t _size, BaseType _basetype)
            :name(_name), size(_size), basetype(_basetype){}
    
private:
    const string& name;
    size_t size;
    BaseType basetype;
    
};


std::map <int, Type&> standard_types;


class Value{
    
public:
    
    Value(int val):int_value(val){
    }
    
private:
    
    int int_value;
    
};

class Symbol{
    
public:
    
    Symbol(string _name, Type& _stype, Value _value = NULL)
        : name(_name), stype(_stype), value(_value) {
    }
        
    const string& getName() const{
        return name;
    }
            
private:
    const string name;
    Type& stype;
    Value value;
};


class SymbolTable{
    
public:
    
    Symbol& find(const string& symbol_name){
        return table[symbol_name];
    }
    
    void exclude();
    
    void append(Symbol symbol){
        table[symbol.getName()] = symbol;
    }
    
    
private :
    std::map<const string&, Symbol> table;
};


class IRNode {
public:
    
    IRNode(IRNode* _parent = NULL, vector<IRNode&>* _children = NULL, SymbolTable* _symtab = NULL):
        parent(_parent), children(_children), symtab(_symtab)
    {
            
    }
        
    virtual ~IRNode();
    
    IRNode& getParent() const{
        return *parent; 
    }
    
    void setParent(IRNode& par){
        parent = &par;
    }
    
    vector<IRNode&>& getChildren() const{
        return *children;
    }
    
    void setChildren(vector<IRNode>& child){
        children = &child;
    }
    
    std::ostream& operator<<(ostream& rhs) {
        //TODO
        
    }
    
    /*inline bool operator==(IRNode& rhs){
        return this == &rhs;
    }*/
    
    void replace(IRNode& old_node, IRNode& new_node){
        if(children){
            for(vector<IRNode&>::size_type i = 0; i < children->size(); ++i){
                if(children->at(i) == old_node){
                    children->at(i) = new_node;
                }
            }
        }
        
    }
    
friend bool operator==(IRNode& lhs, IRNode& rhs);    
    
private:
    
    IRNode* parent;
    vector<IRNode&>* children;
    SymbolTable* symtab;
    
};

inline bool operator==(IRNode& lhs, IRNode& rhs){
    return (&lhs == &rhs);
}

class Tok : public IRNode{
public:
    Tok(int _tok, SymbolTable& symtab):tok(_tok), IRNode(NULL,NULL,&symtab){
    }
    
private:
    int tok;
    
};

class Expr : public IRNode{
public:    
    
    Expr(int _op, SymbolTable& symtab):
         IRNode(NULL, NULL, &symtab), op(_op, symtab){
    }
    
    IRNode& getOperator() const{
        return getChildren()[0];
    }
    
private:
    Tok op;
    
};

class BinExpr : public Expr{
public:
    
    BinExpr(int _op, vector<IRNode>& children, SymbolTable& symtab):
        Expr(_op, symtab){
        
        for(vector<IRNode>::size_type i = 0; i < children.size(); ++i)
            children[i].setParent(*this);
        
        setChildren(children);
        
    }
   
};

class UnExpr : public Expr{
public:
    UnExpr(int tok, IRNode expr, SymbolTable& symtab):Expr(tok, symtab) {
        Tok tok_node(tok, symtab);
        
        tok_node.setParent(*this);
        expr.setParent(*this);
        
        vector<IRNode> children;
        children.push_back(tok_node);
        children.push_back(expr);
        
        setChildren(children);
    }
    
};

class CallExpr : public Expr{
public:    
    CallExpr(Symbol& sym, int op, SymbolTable& symtab):
        Expr(op, symtab), function(sym)
    {
        
    }
    
private:
    
    Symbol& function;
    
    
};

class Stat : public IRNode{
public:
    
    Stat(SymbolTable& symtab): 
        IRNode(NULL, NULL, &symtab){}
    
    void setLabel();
    
    IRNode& getLabel() const{
        
    }
    
    IRNode& getFunction(){
        if(1){
            
        }
        else{
            //TODO
        }
    }
};

class StatList : public IRNode {
public:
    StatList(SymbolTable& symtab )
    :IRNode(NULL, NULL, &symtab){
    }
    
    void append(IRNode stat){
        stat_list.push_back(stat);
    }
    
private:
    std::list<IRNode> stat_list;
};

class AssignStat : public Stat{
public:
    AssignStat(Symbol& _sym, IRNode& _expr, SymbolTable& symtab)
    : Stat(symtab), sym(_sym), expr(_expr) {
        
    }
private:
    Symbol& sym;
    Expr& expr;
};

class CallStat : public Stat{
public:    
    
    CallStat(CallExpr expr, SymbolTable& symtab)
            :Stat(symtab), call(expr){
        
        call.setParent(*this);
    }
    
private:
    CallExpr call;
};

class IfStat : public Stat{
public:
    IfStat(IRNode& cond, IRNode& then, SymbolTable &symtab)
        :Stat(symtab)
    {
        
        cond.setParent(*this);
        then.setParent(*this);
        
        vector<IRNode&> children;
        children.push_back(cond);
        children.push_back(then);
        
        setChildren(children);
    }
};

class WhileStat : public Stat{
public:
    WhileStat(IRNode& cond, IRNode& body, SymbolTable &symtab)
         :Stat(symtab)   
    {
        
        cond.setParent(*this);
        body.setParent(*this);
        
        vector<IRNode&> children;
        children.push_back(cond);
        children.push_back(body);
        
        setChildren(children);
    }
};

class PrintStat : public Stat{
public:
    PrintStat(Symbol& _sym, SymbolTable &symtab)
        :sym(_sym), Stat(symtab)
    {
        
    }
private:
    Symbol& sym;
};

class Definition : public IRNode{
public:    
    Definition(Symbol& _symbol): symbol(_symbol){
        
    }

private:    
    Symbol& symbol;
    
};

class DefinitionList : public IRNode{
public: 
    DefinitionList():list(){}
    
    void append(Definition elem){
        list.push_back(elem);
    }   
    
    
private:

    std::list<IRNode> list;
};

class Block : public Stat{
public :
    
    Block(SymbolTable& _gl_sym, SymbolTable& _lc_sym,
            IRNode& _body, DefinitionList& _defs):
    Stat(_gl_sym), gl_sym(_gl_sym), lc_sym(_lc_sym), body(_body), defs(_defs)
    {
         body.setParent(*this);
    }
  
    

private:
    IRNode& body;
    DefinitionList& defs;
    SymbolTable& gl_sym;
    SymbolTable& lc_sym;
    
};

class FunctionDef : public Definition{
public :
    FunctionDef(Symbol& _symbol, vector<Symbol&>& _parameters, IRNode& _body)
        :Definition(_symbol), parameters(_parameters), body(_body){
            
            body.setParent(*this);
           
            /*for(vector<Symbol&>::size_type i = 0; i < parameters.size(); ++i){
                parameters[i].setParent(this);
            }*/
        
    }
        
    
    const vector<Symbol&>& getParameters() const{
        return parameters;
    }

private:
    const vector<Symbol&>& parameters;
    IRNode& body;
    
};

class Var : public IRNode{
public:    
    Var(Symbol& _symbol, SymbolTable& symtab) : symbol(_symbol), IRNode(NULL, NULL, &symtab){
        
    }
    
private:
    Symbol& symbol;
};

class Const : public IRNode{
public:
    Const(Value _value, SymbolTable& symtab):value(_value), IRNode(NULL, NULL, &symtab){};

private:
    Value& value;
};





#endif	/* IR_H */

