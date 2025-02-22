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
#include <map>
#include <cmath>
#include <unordered_map>
#include <set>

using namespace std;

/*
Polynomial declaration:
POLY poly_name (variable_list) = term_list;

we need to store the rest of the poly in poly_decl_t struct

*/
void Parser::syntax_error()
{
    Token t = lexer.peek(1);
    // cout << "at line no: " << t.line_no << endl;
    // cout << "token type: " << t.token_type << endl;
    // cout << "lexeme: " << t.lexeme << endl;
     cout << "SYNTAX ERROR !!!!!&%!!\n";

    exit(1);
}

std::vector<AssignmentF *> assignments;



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



struct Comp_input
{
    vector<Token> Tasks;
    vector<Poly_eq> Polynomials;
    Exec execlist;
    vector<int> Inputs;
};

Comp_input comp_input;
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
        // cout << "at line no: " << t.line_no << endl;
        // cout << "expected type: " << expected_type << endl;
        // cout << "but got token type: " << t.token_type << endl;
        syntax_error();
    }
    return t;
}
Exec execlist;
//vector<Assignment> Assignments;
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
    if(lexer.peek(1).token_type != END_OF_FILE)
    {
       ///cout << "FUCKKKK";
    }
    expect(END_OF_FILE);
}
void Parser::parse_program()
{ // program -> tasks_section poly_section execute_section inputs_section
    parse_tasks_section();
    parse_poly_section();
    reparse_poly_section();
   // print_reparsed_polynomials();
    parse_execute_section();
    parse_inputs_section();
    FinalCalc();
    //vector <int> n =FinalCalc();
    // for (const int i: n )
    // {cout << i << " "; 
    // }
}

vector<Poly_eq> ParsedPolynomials;

void Parser::reparse_poly_section() {
    ParsedPolynomials.clear();  // Clear any existing data

    for (const auto& poly_decl : Polynomials) {
        Poly_eq poly_eq;
        poly_eq.header = poly_decl.header;

        vector<Token> tokens = poly_decl.terms;
        vector<term> terms;

        vector<int> separator_indices;
        for (size_t i = 0; i < tokens.size(); ++i) {
            if (tokens[i].token_type == PLUS || tokens[i].token_type == MINUS) {
                separator_indices.push_back(i);
            }
        }

        vector<vector<Token>> term_groups;
        size_t start = 0;
        for (size_t sep : separator_indices) {
            vector<Token> group(tokens.begin() + start, tokens.begin() + sep);
            if (!group.empty()) {
                term_groups.push_back(group);
            }
            start = sep + 1;
        }
        vector<Token> last_group(tokens.begin() + start, tokens.end());
        if (!last_group.empty()) {
            term_groups.push_back(last_group);
        }

        vector<Token> separators;
        for (int sep : separator_indices) {
            separators.push_back(tokens[sep]);
        }

        for (size_t i = 0; i < term_groups.size(); ++i) {
            term t;
            Token op;
            if (i == 0) {
                op.token_type = PLUS;
                op.lexeme = "+";
                op.line_no = 0;
            } else {
                op = separators[i - 1];
            }

            t.Sign = op;

            int sign = (op.token_type == PLUS) ? 1 : -1;
            int coef = 1;
            size_t pos = 0;

            if (pos < term_groups[i].size() && term_groups[i][pos].token_type == MINUS) {
                sign *= -1;
                pos++;
            }

            if (pos < term_groups[i].size() && term_groups[i][pos].token_type == NUM) {
                coef = stoi(term_groups[i][pos].lexeme) * sign;
                pos++;
            } else if (sign == -1) {
                coef = -1;
            }

            t.coef = coef;

            while (pos < term_groups[i].size()) {
                if (term_groups[i][pos].token_type == ID) {
                    variable var;
                    var.id = term_groups[i][pos];
                    var.exponent = 1;
                    pos++;
                    if (pos < term_groups[i].size() && term_groups[i][pos].token_type == POWER) {
                        pos++;
                        if (pos < term_groups[i].size() && term_groups[i][pos].token_type == NUM) {
                            var.exponent = stoi(term_groups[i][pos].lexeme);
                            pos++;
                        } else {
                            //cout << "caused by the parser1" << endl;
                            syntax_error();
                        }
                    }
                    t.var.push_back(var);
                } else if (term_groups[i][pos].token_type == LPAREN) {
                    pos++;
                    vector<variable> group_vars;
                    while (pos < term_groups[i].size() && term_groups[i][pos].token_type != RPAREN) {
                        if (term_groups[i][pos].token_type == ID) {
                            variable var;
                            var.id = term_groups[i][pos];
                            var.exponent = 1;
                            pos++;
                            if (pos < term_groups[i].size() && term_groups[i][pos].token_type == POWER) {
                                pos++;
                                if (pos < term_groups[i].size() && term_groups[i][pos].token_type == NUM) {
                                    var.exponent = stoi(term_groups[i][pos].lexeme);
                                    pos++;
                                } else {
                                   // cout << "caused by the parser2" << endl;
                                    syntax_error();
                                }
                            }
                            group_vars.push_back(var);
                        } else {
                            syntax_error();
                        }
                    }
                    // if (pos >= term_groups[i].size() || term_groups[i][pos].token_type != RPAREN) {
                    //     cout << "caused by the parser3" << endl;
                    //     syntax_error();
                    // }
                    pos++;
                    if (pos < term_groups[i].size() && term_groups[i][pos].token_type == POWER) {
                        pos++;
                        if (pos < term_groups[i].size() && term_groups[i][pos].token_type == NUM) {
                            int exp = stoi(term_groups[i][pos].lexeme);
                            for (auto& v : group_vars) {
                                v.exponent *= exp;
                            }
                            pos++;
                        } else {
                          //  cout << "caused by the parser4" << endl;
                            syntax_error();
                        }
                    }
                    for (const auto& v : group_vars) {
                        t.var.push_back(v);
                    }
                } else {
                   // cout << "caused by the parser5" << endl;
                    break;
                }
            }

            terms.push_back(t);
        }

        poly_eq.term_list = terms;
        ParsedPolynomials.push_back(poly_eq);
    }
}


void Parser::print_reparsed_polynomials() {
    for (const auto& poly_eq : ParsedPolynomials) {
        // Print the polynomial header
        cout << "POLY " << poly_eq.header.poly_name.lexeme << "(";
        for (size_t i = 0; i < poly_eq.header.variables.size(); ++i) {
            cout << poly_eq.header.variables[i];
            if (i < poly_eq.header.variables.size() - 1) {
                cout << ", ";
            }
        }
        cout << ") = ";

        // Print each term
        for (size_t i = 0; i < poly_eq.term_list.size(); ++i) {
            const term& t = poly_eq.term_list[i];

            // Print the sign (if not the first term or if the first term is negative)
            if (i != 0) {
                
                cout << " " ;
                if(t.Sign.token_type == MINUS)
                {
                    cout << "-";
                }
                if (t.Sign.token_type == PLUS)
                {
                    cout << "+";
                }
                cout << " ";
            }

            // Print the coefficient (if not 1 or -1)
            if (abs(t.coef) != 1 || t.var.empty()) {
                cout << abs(t.coef);
            }

            // Print the variables and their exponents
            for (const auto& var : t.var) {
                cout << var.id.lexeme;
                if (var.exponent != 1) {
                    cout << "^" << var.exponent;
                }
            }
        }

        cout << ";" << endl;
    }
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
        Token inputToken = expect(NUM);
        comp_input.Inputs.push_back(stoi(inputToken.lexeme));
    }
}








execLinkedList* Parser::convertExecToLinkedList(Exec& exec) {
    // Sort assignments by their id's line number
    std::vector<AssignmentF*> sortedAssignments = exec.assignments;
    std::sort(sortedAssignments.begin(), sortedAssignments.end(),
              [](AssignmentF* a, AssignmentF* b) {
                  return a->id.line_no < b->id.line_no;
              });

    // Create maps from line number to Input and Output tokens
    std::unordered_map<int, Token*> inputMap;
    for (auto& inputToken : exec.Inputs) {
        inputMap[inputToken.line_no] = &inputToken;
    }

    std::unordered_map<int, Token*> outputMap;
    for (auto& outputToken : exec.Outputs) {
        outputMap[outputToken.line_no] = &outputToken;
    }

    // Build the linked list
    execLinkedList* head = nullptr;
    execLinkedList* current = nullptr;

    for (AssignmentF* assignment : sortedAssignments) {
        int line = assignment->id.line_no;

        Token* input = nullptr;
        auto inputIt = inputMap.find(line);
        if (inputIt != inputMap.end()) {
            input = inputIt->second;
        }

        Token* output = nullptr;
        auto outputIt = outputMap.find(line);
        if (outputIt != outputMap.end()) {
            output = outputIt->second;
        }

        execLinkedList* newNode = new execLinkedList();
        newNode->assignment = assignment;
        newNode->input = input;
        newNode->output = output;
        newNode->next = nullptr;

        if (head == nullptr) {
            head = newNode;
            current = head;
        } else {
            current->next = newNode;
            current = newNode;
        }
    }

    return head;
}


vector<int> Parser::FinalCalc() {
    execLinkedList* head = convertExecToLinkedList(execlist);
    vector<variable> active;
    execLinkedList* current = head;

    while (current != nullptr) {
        if (current->input != nullptr) {
            bool found = false;
            for (auto& active_var : active) {
                if (active_var.id.lexeme == current->input->lexeme) {
                    if (!comp_input.Inputs.empty()) {
                        active_var.val = comp_input.Inputs[0];
                        comp_input.Inputs.erase(comp_input.Inputs.begin());
                        found = true;
                    } else {
                        syntax_error();
                    }
                    break;
                }
            }
            if (!found) {
                if (!comp_input.Inputs.empty()) {
                    variable var;
                    var.id = *current->input;
                    var.val = comp_input.Inputs[0];
                    comp_input.Inputs.erase(comp_input.Inputs.begin());
                    active.push_back(var);
                } else {
                    syntax_error();
                }
            }
        } else if (current->output != nullptr) {
            bool found = false;
            for (const auto& active_var : active) {
                if (active_var.id.lexeme == current->output->lexeme) {
                    cout << active_var.val << endl;
                    found = true;
                    break;
                }
            }
            if (!found) {
                syntax_error();
            }
        } else if (current->assignment != nullptr) {
            int val = cal_assign(current->assignment, active);
            bool found = false;
            for (auto& active_var : active) {
                if (active_var.id.lexeme == current->assignment->id.lexeme) {
                    active_var.val = val;
                    found = true;
                    break;
                }
            }
            if (!found) {
                variable new_var;
                new_var.id = current->assignment->id;
                new_var.val = val;
                active.push_back(new_var);
            }
        }
        current = current->next;
    }

    return {}; // Return empty vector as per original code structure
}

int Parser::cal_assign(AssignmentF* assign, vector<variable> &active ) {
    Poly_eq* poly = nullptr;
    for (auto& p : ParsedPolynomials) {
        if (p.header.poly_name.lexeme == assign->id.lexeme) {
            poly = &p;
            break;
        }
    }
    if (poly == nullptr) {
        syntax_error();
        return 0;
    }

    vector<int> args;
    AssignmentF* arg = assign->child;
    while (arg != nullptr) {
        if (arg->id.token_type == ID) {
            bool found_arg = false;
            for (const auto& var : active) {
                if (var.id.lexeme == arg->id.lexeme) {
                    args.push_back(var.val);
                    found_arg = true;
                    break;
                }
            }
            if (!found_arg) {
                syntax_error();
                return 0;
            }
        } else if (arg->id.token_type == NUM) {
            args.push_back(stoi(arg->id.lexeme));
        } else {
            syntax_error();
            return 0;
        }
        arg = arg->sibling;
    }

    if (args.size() != poly->header.variables.size()) {
        syntax_error();
        return 0;
    }

    return cal_Poly(poly, args);
}

int Parser::cal_Poly(Poly_eq* poly, const vector<int>& args) {
    int total = 0;
    for (const auto& term : poly->term_list) {
        int term_val = term.coef;
        for (const auto& var : term.var) {
            auto it = find(poly->header.variables.begin(), poly->header.variables.end(), var.id.lexeme);
            if (it == poly->header.variables.end()) {
                syntax_error();
                return 0;
            }
            int index = it - poly->header.variables.begin();
            int arg_val = args[index];
            term_val *= static_cast<int>(pow(arg_val, var.exponent));
        }
        total += term_val;
    }
    return total;
}