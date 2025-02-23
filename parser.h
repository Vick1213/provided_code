/*
 * Copyright (C) Rida Bazzi, 2019
 *
 * Do not share this file with anyone
 */
#ifndef __PARSER_H__
#define __PARSER_H__
#include <map>
#include <string>
#include <iostream>
#include <utility>
#include <vector> // Include for std::vector
#include "lexer.h"
struct variable
{
    Token id;
    int val;
    int exponent =1;
};
struct term
{
    int coef =1;
    std::vector<variable> var;
    Token Sign;
};




struct poly_header_t
{
    Token poly_name;
    std::vector<std::string> variables;
};
struct Poly_eq{
    poly_header_t header;
    std::vector <term> term_list;  
  };

  struct AssignmentF
  {
      Token id;
      AssignmentF *child;
      AssignmentF *sibling;
  
      AssignmentF() : child(nullptr), sibling(nullptr) {}
  
      ~AssignmentF()
      {
          delete child;
          delete sibling;
      }
  };
  
  struct Exec
{
    std::vector<Token> Inputs;
    std::vector<Token> Outputs;
    std::vector<AssignmentF *> assignments;
};

struct execLinkedList
{
    AssignmentF* assignment;
    Token *input;
    Token *output;
    execLinkedList* next;
};

class Parser {
private:
    LexicalAnalyzer lexer;
    void reportError(const std::string& message, int line_no);
    void syntax_error();
    void semantic_error(std::string code, std::vector<int> line_no);
    Token expect(TokenType expected_type);
    struct AssignmentF* assignment_recur(AssignmentF* assignment);
    // Parsing functions
    
    void parse_program();
    void check_semantic_error2();
    void parse_tasks_section();
    void parse_poly_section();
    struct poly_decl_t parsePolyDecl();
    struct poly_header_t parsePolyHeader();
    struct execLinkedList* convertExecToLinkedList(Exec& exec);
    Token parsePolyName();
    std::vector<std::string> parseIdList();
    std::string parseId();
    std::vector<Token> parsePolyBody();
    void parseTermList(std::vector<Token> &terms);
    void parse_poly_decl_list();
    void reparse_poly_section();
    void print_reparsed_polynomials();
    std::vector <int> FinalCalc();
    void parseTerm(std::vector<Token> &terms);
    void parseTermListPrime(std::vector<Token>& terms);
    void parseMonomialList(std::vector<Token>& terms);
    void parseMonomial(std::vector<Token>& terms);
    void parse_execute_section();
    void statementList();
    void check_semantic_error4();
    void statement();
    void check_assignment_arguments(AssignmentF* assignment, std::vector<int>& error_lines);
    struct AssignmentF *parse_argument_list();
    void inputStatement();
    void outputStatement();
    int evaluate_expression(AssignmentF *node ,std::map<std::string,int>& symbol_table);
    int evaluate_polynomial(const Poly_eq& poly, const std::vector<int>& args);
    void getAllPolyHeader();
    void assignmentStatement();
    void test_print_assignments_statement(AssignmentF* assignment);
    std::vector <Token> argument_list();
    void parseExpression();  // If needed
    void parse_inputs_section();
    void check_semantic_error1();
    void parseInput();
    void check_semantic_error3(Token id);
    int cal_assign(AssignmentF* assin, std::vector<variable> &active );
    int cal_Poly(Poly_eq* poly, const std::vector<int>& args);
    std::pair<std::vector<variable>, size_t> parse_parenthetical(const std::vector<Token>& tokens, size_t pos); 
    variable parse_variable(const std::vector<Token>& tokens, size_t& pos);

public:
    Parser(); // Constructor is important!
    void ConsumeAllInput(); // If you still need it for testing
    void parse_input();

};

#endif // __PARSER_H__