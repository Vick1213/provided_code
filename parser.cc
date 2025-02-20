/*
 * Copyright (C) Rida Bazzi, 2020
 *
 * Do not share this file with anyone
 *
 * Do not post this file or derivatives of
 * of this file online
 *
 */
#include <iostream>
#include <cstdlib>
#include "parser.h"
#include <algorithm>
#include <queue>
#include <set>

using namespace std;

/*
Polynomial declaration:
POLY poly_name (variable_list) = term_list;

we need to store the rest of the poly in poly_decl_t struct

*/
void Parser::syntax_error()
{
   
    cout << "SYNTAX ERROR !!!!!&%!!\n";

    exit(1);
}

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

std::vector<AssignmentF *> assignments;

struct poly_header_t
{
    Token poly_name;
    std::vector<std::string> variables;
};

struct poly_decl_t
{
    poly_header_t header;
    std::vector<Token> terms; // Changed to vector of Tokens
};

void Parser::semantic_error(std::string code, std::vector<int> line_no)
{
    cout << "Semantic Error " << code << ":";
    for (int i = 0; i < (int)line_no.size(); i++)
    {
        cout << line_no[i] << " ";
    }
    cout << endl;
    exit(1);
}
vector<Token> AllPolyHeaders;


std::vector<poly_decl_t> Polynomials;

std::vector<Token> id_list; 
struct Assignment
{
    Token output_id;
    Token poly_id;
    vector<Token> arguments;
};

struct Exec
{
    vector<Token> Inputs;
    vector<Token> Outputs;
    vector<AssignmentF *> assignments;
};

struct Comp_input
{
    vector<Token> Tasks;
    vector<poly_decl_t> Polynomials;
    vector<Exec> Execute_stmt;
    vector<int> Inputs;
};

// this function gets a token and checks if it is
// of the expected type. If it is, the token is
// returned, otherwise, synatx_error() is generated
// this function is particularly useful to match
// terminals in a right hand side of a rule.
// Written by Mohsen Zohrevandi
Token Parser::expect(TokenType expected_type)
{
    
    Token t = lexer.GetToken();

    if (t.token_type != expected_type)
    {
        syntax_error();
    }
    return t;
}
Exec execlist;
vector<Assignment> Assignments;
vector<int> errorLines3;


Parser::Parser()
{
    // Constructor implementation (if needed)
}

int main()
{
    // note: the parser class has a lexer object instantiated in it. You should not be declaring
    // a separate lexer object. You can access the lexer object in the parser functions as shown in the
    // example method Parser::ConsumeAllInput
    // If you declare another lexer object, lexical analysis will not work correctly
    Parser parser;

    parser.parse_input();
    // delete execlist.assignments;
    for (auto assignment : execlist.assignments)
    {
        delete assignment;
    }
    execlist.assignments.clear();

    return 0;
}

void Parser::parse_input()
{ // the input consists of a program followed by nothing
    parse_program();
    expect(END_OF_FILE);
}
void Parser::parse_program()
{ // program -> tasks_section poly_section execute_section inputs_section
    parse_tasks_section();
    parse_poly_section();
    parse_execute_section();
    parse_inputs_section();
}

void Parser::parse_tasks_section()
{ // tasks_section -> TASKS { task_list }   // TASKS is a keyword
    expect(TASKS);

    vector<int> taskNumbers; // Store the numbers in a vector

    Token currentToken = lexer.peek(1);
    while (currentToken.token_type == NUM)
    {
        currentToken = lexer.GetToken(); // Consume the number token

        int num = stoi(currentToken.lexeme); // Convert lexeme to integer
        taskNumbers.push_back(num);

        currentToken = lexer.peek(1);
    }
}

void Parser::parse_poly_section()
{
    expect(POLY);

    // Parse the polynomial sectin

    parse_poly_decl_list();

    check_semantic_error1();
    check_semantic_error2();

    // if there are multiple declaration we add the declarations to a list
}
void Parser::check_semantic_error1()
{
    // check if there are any duplicate header polynomial names
    vector<int> error_lines;
    for (int i = 0; i < (int)Polynomials.size(); i++)
    {
        for (int j = i + 1; j < (int)Polynomials.size(); j++)
        {
            if (Polynomials[i].header.poly_name.lexeme == Polynomials[j].header.poly_name.lexeme)
            {
                error_lines.push_back(Polynomials[j].header.poly_name.line_no);
                break;
            }
        }
    }
    if (!error_lines.empty())
    {
        std::sort(error_lines.begin(), error_lines.end());
        semantic_error("Code 1", error_lines);
    }
}
void Parser::check_semantic_error2()
{

    vector<int> error_lines;

    for (int i = 0; i < (int)Polynomials.size(); i++)
    {
        if (Polynomials[i].header.variables.empty())
        {
            for (int j = 0; j < (int)Polynomials[i].terms.size(); j++)
            {
                if (Polynomials[i].terms[j].token_type == ID && Polynomials[i].terms[j].lexeme != "x")
                {
                    error_lines.push_back(Polynomials[i].terms[j].line_no);
                }
            }
        }
        else
        {
            for (int j = 0; j < (int)Polynomials[i].terms.size(); j++)
            {
                if (Polynomials[i].terms[j].token_type == ID && std::find(Polynomials[i].header.variables.begin(), Polynomials[i].header.variables.end(), Polynomials[i].terms[j].lexeme) == Polynomials[i].header.variables.end())
                {
                    error_lines.push_back(Polynomials[i].terms[j].line_no);
                }
            }
        }
    }

    if (!error_lines.empty())
    {
        std::sort(error_lines.begin(), error_lines.end());
        semantic_error("Code 2 ", error_lines);
    }
}

void Parser::parse_poly_decl_list()
{

    // poly_dec_list -> poly_decl
    // poly_dec_list -> poly_decl poly_dec_list
    poly_decl_t polyDecl = parsePolyDecl();

    Polynomials.push_back(polyDecl); // parses one declaration
    if (lexer.peek(1).token_type != EXECUTE)
    {
        parse_poly_decl_list();
    }
}

struct poly_decl_t Parser::parsePolyDecl()
{

    poly_decl_t decl;
    decl.header = parsePolyHeader();
    expect(EQUAL);
    std::vector<Token> terms = parsePolyBody();

    for (Token &term : terms)
    {
        decl.terms.push_back(term);
    }
    expect(SEMICOLON);

    return decl;
}

struct poly_header_t Parser::parsePolyHeader()
{
    poly_header_t header;

    Token polyNameId = parsePolyName();
    header.poly_name = polyNameId;

    std::string polyName = polyNameId.lexeme;
    std::vector<std::string> variables;
    id_list.push_back(polyNameId); // add the id to the list

    if (lexer.peek(1).token_type == LPAREN)
    {

        Token t = lexer.GetToken(); // Consume LPAREN

        variables = parseIdList();
        expect(RPAREN); // Expect RPAREN
    }else
    {
        variables = {"x"};
    }


    header.variables = variables;
    return header;
}

Token Parser::parsePolyName()
{
    Token token = expect(ID);
    return token;
}

std::vector<std::string> Parser::parseIdList()
{
    std::vector<std::string> ids;
    ids.push_back(parseId());
    while (lexer.peek(1).token_type == COMMA)
    {
        lexer.GetToken(); // Consume COMMA
        ids.push_back(parseId());
        if (lexer.peek(1).token_type != COMMA || lexer.peek(1).token_type == RPAREN)
        {
            break;
        }
    }
    return ids;
}

std::string Parser::parseId()
{
    Token token = expect(ID);

    return token.lexeme;
}

std::vector<Token> Parser::parsePolyBody()
{
    std::vector<Token> terms;
    parseTermList(terms); // Pass terms vector to collect tokens
    return terms;
}

void Parser::parseTermList(std::vector<Token> &terms)
{
    parseTerm(terms);
    parseTermListPrime(terms);
}

void Parser::parseTermListPrime(std::vector<Token> &terms)
{
    Token t = lexer.peek(1);
    if (t.token_type == PLUS || t.token_type == MINUS)
    {
        Token op = lexer.GetToken();
        terms.push_back(op); // Add operator token
        parseTerm(terms);
        parseTermListPrime(terms);
    }
}

void Parser::parseTerm(std::vector<Token> &terms)
{
    // Handle coefficient or minus sign
    if (lexer.peek(1).token_type == NUM)
    {
        Token numToken = lexer.GetToken();
        terms.push_back(numToken);
    }
    else if (lexer.peek(1).token_type == MINUS)
    {
        Token minusToken = lexer.GetToken();
        terms.push_back(minusToken);
    }
    parseMonomialList(terms);
}

void Parser::parseMonomialList(std::vector<Token> &terms)
{
    while (lexer.peek(1).token_type == ID || lexer.peek(1).token_type == LPAREN)
    {
        parseMonomial(terms);
    }
}


void Parser::parseMonomial(std::vector<Token> &terms)
{
    if (lexer.peek(1).token_type == ID)
    {
        Token idToken = lexer.GetToken();
        terms.push_back(idToken);
    }
    else
    {
        // Handle parenthesized terms
        Token lparen = lexer.GetToken();
        terms.push_back(lparen);
        parseTermList(terms);
        Token rparen = lexer.GetToken();
        if (rparen.token_type != RPAREN)
        {

            syntax_error();
        }
        terms.push_back(rparen);
    }

    // Handle exponent
    if (lexer.peek(1).token_type == POWER)
    {
        Token powerToken = lexer.GetToken();
        terms.push_back(powerToken);
        Token expToken = lexer.GetToken();
        if (expToken.token_type != NUM)
        {

            syntax_error();
        }
        terms.push_back(expToken);
    }
}

void Parser::getAllPolyHeader()
{
    for (auto &polynomial : Polynomials)
    {
        {
            AllPolyHeaders.push_back(polynomial.header.poly_name);
        }
    }
}

void Parser::parse_execute_section()
{
    // need this to check againt all the other assigned poly headers
    getAllPolyHeader();
    // execute_section -> EXECUTE { execute_list }
   

    expect(EXECUTE);
    while (lexer.peek(1).token_type != INPUTS && lexer.peek(1).token_type != END_OF_FILE)
    {
        statementList();
    }
    
    check_semantic_error4();
    
}
void Parser::check_semantic_error4()
{
    // check if all the polynomial arguments for each assignment statement has the right number of arguments
    vector<int> error_lines;
    for (AssignmentF* assignment : execlist.assignments)
    {
        check_assignment_arguments(assignment, error_lines);
    }
    if (!error_lines.empty())
    {
        semantic_error("Code 4 ", error_lines);
    }
}

void Parser::check_assignment_arguments(AssignmentF* assignment, vector<int>& error_lines)
{

    
    for (const auto &polynomial : Polynomials) // Use const auto& for iteration
    {
        

        if (assignment->id.lexeme == polynomial.header.poly_name.lexeme)
        {
           
            int num_args = 0;
            AssignmentF *current = assignment->child;
            while (current != nullptr)
            {
                num_args++;
                current = current->sibling;
            }
          

            if (num_args != (int)polynomial.header.variables.size())
            {
               
                error_lines.push_back(assignment->id.line_no);
            } else {

            }
            break; // Important: Exit loop after finding the polynomial
        }
    }



    AssignmentF* current_child = assignment->child;
    while(current_child != nullptr) {
     
        
        check_assignment_arguments(current_child, error_lines);
        current_child = current_child->sibling;
    }

}
void Parser::statementList()
{
    while (lexer.peek(1).token_type != INPUTS && lexer.peek(1).token_type != END_OF_FILE)
    {
        statement();

        expect(SEMICOLON);
    }
    execlist.assignments = std::move(assignments);
    if (!errorLines3.empty())
    {
        semantic_error("Code 3 ", errorLines3);
    }
}

void Parser::statement()
{
    if (lexer.peek(1).token_type == INPUT)
    {
        inputStatement();
    }
    else if (lexer.peek(1).token_type == OUTPUT)
    {
        outputStatement();
    }
    else if (lexer.peek(1).token_type == ID)
    {

        assignmentStatement();
    }
    else
    {

        syntax_error();
    }
}

void Parser::inputStatement()
{
    expect(INPUT);
    Token id = expect(ID);

    execlist.Inputs.push_back(id);
}

void Parser::outputStatement()
{
    expect(OUTPUT);
    Token id = expect(ID);
    execlist.Outputs.push_back(id);
}
vector<string> ArgumentNames;
void Parser::assignmentStatement()
{
    Token output_id = expect(ID);
    AssignmentF *newAssignment = new AssignmentF;
    ArgumentNames.push_back(output_id.lexeme);

    newAssignment->id = output_id;
    expect(EQUAL);
    newAssignment->child = nullptr;
    newAssignment->sibling = nullptr;
    assignment_recur(newAssignment);
  // test_print_assignments_statement(newAssignment);

    assignments.push_back(newAssignment);
}

void Parser::test_print_assignments_statement(AssignmentF *assignment)
{
    // create a dfs to print the assignment statement
    if (assignment == nullptr)
    {
        return;
    }
    AssignmentF *current = assignment;
    cout << current->id.lexeme << " ";
    if (current->child != nullptr)
    {
        cout << "(";
        test_print_assignments_statement(current->child);
        cout << ")";
    }
    if (current->sibling != nullptr)
    {
        cout << ", ";
        test_print_assignments_statement(current->sibling);
    }
}

void Parser::check_semantic_error3(Token id)
{
    bool found = false;
    
    for (const auto &header : AllPolyHeaders)
    {
       
        if(!execlist.Inputs.empty())
        {  
              for(const auto &inputs : execlist.Inputs) {
                
                if(id.lexeme == header.lexeme || id.lexeme == inputs.lexeme) {   
                    
                    found = true;
                    
                }
            }
        }
        else
        {
            if(id.lexeme == header.lexeme) {   
               
                found = true;
                
            }
        }

            if(std::find(ArgumentNames.begin(), ArgumentNames.end(), id.lexeme) != ArgumentNames.end()) {
             
                found = true;
                
            }
        if(found) {
            break;
        }
    }
    if(!found)
    {
        errorLines3.push_back(id.line_no);
    }
}
struct AssignmentF *Parser::assignment_recur(struct AssignmentF *assignment)
{
    if (lexer.peek(1).token_type == ID)
    {
        Token id = expect(ID);
        // check agaisnt getAllPolyHeader
        check_semantic_error3(id);
        assignment->id = id;
        if (lexer.peek(1).token_type == LPAREN)
        {
            expect(LPAREN);
            assignment->child = parse_argument_list(); // Use new function with error check
            expect(RPAREN); // Expect the closing parenthesis of the function call
        }
    }
    else if (lexer.peek(1).token_type == NUM)
    {
        assignment->id = expect(NUM);
    }
    else if (lexer.peek(1).token_type == COMMA)
    {

        expect(COMMA);
        assignment->sibling = assignment_recur(new AssignmentF);
    }
    else if (lexer.peek(1).token_type == RPAREN || lexer.peek(1).token_type == SEMICOLON || lexer.peek(1).token_type == OUTPUT)
    {
        return nullptr;
    }
    else
    {
        syntax_error();
    }
    return assignment;
}

struct AssignmentF *Parser::parse_argument_list()
{
    AssignmentF *head = nullptr;
    AssignmentF *current = nullptr;

    if (lexer.peek(1).token_type == RPAREN) // Handle empty argument list: F()
    {
        return nullptr;
    }

    while (true)
    {
        AssignmentF *arg = assignment_recur(new AssignmentF()); // Parse an argument (ID or NUM)
        if (!arg) // If assignment_recur returns nullptr, it might be an error or end of list
        {
            // 
            syntax_error(); // Or more specific error message if possible from assignment_recur
            return head; // Return whatever we parsed so far, might be partially built list
        }

        if (!head)
        {
            head = arg;
            current = head;
        }
        else
        {
            current->sibling = arg;
            current = arg;
        }

        if (lexer.peek(1).token_type == COMMA)
        {
            expect(COMMA); // Consume the comma
            if (lexer.peek(1).token_type == RPAREN)
            {
                syntax_error(); // Trailing comma detected: F(a,b,) - error here!
                return head; // Return the list parsed so far
            }
            // Continue to next argument
        }
        else if (lexer.peek(1).token_type == RPAREN)
        {
            break; // End of argument list, no trailing comma
        }
        else
        {
            syntax_error(); // Unexpected token after argument (not comma or RPAREN)
            return head; // Return the list parsed so far
        }
    }

    return head;
}
void Parser::parse_inputs_section()
{
    expect(INPUTS);
    // Parse the inputs section
    while (lexer.peek(1).token_type != END_OF_FILE)
    {
        parseInput();
    }
}

void Parser::parseInput()
{
    while (lexer.peek(1).token_type != END_OF_FILE)
    {
        parseId();
    }
}
