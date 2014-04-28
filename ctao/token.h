/* 
 * File:   token.h
 * Author: daniele
 *
 * Created on 28 aprile 2014, 16.10
 */

#ifndef TOKEN_H
#define	TOKEN_H



class token{
    
public:
    
    enum TokenType{
        error,
        eof,
        number,
        identifier,
        lparen,
        rparen,
        times,
        slash,
        plus,
        minus,
        eql,
        neq,
        lss,
        leq,
        gtr,
        geq,
        callsym,
        beginsym,
        semicolon,
        endsym,
        ifsym,
        whilesym,
        becomes,
        thensym,
        dosym,
        constsym,
        comma,
        varsym,
        procsym,
        period,
        oddsym,
        print
    };
    
private:
    //just a wrapper class
    token();
};



#endif	/* TOKEN_H */

