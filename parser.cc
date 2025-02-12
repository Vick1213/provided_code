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




struct poly_header_t
{
    Token poly_name;
    std::vector <std::string> variables;
};



struct poly_decl_t
{
    poly_header_t header;
    std::vector<Token> terms; // Changed to vector of Tokens
};

void Parser::semantic_error(std::string code,std::vector <int> line_no)
{
    cout << "Semantic Error: " << code << ":";
    for(int i = 0; i < line_no.size(); i++)
    {
        cout << line_no[i] << " ";
    }
    cout << endl;
    exit(1);
}

// void Parser::reportError(const std::string& message, int line_no)
// {
//     cout << "Semantic Error: " << message << " on line " << line_no << endl;
//     exit(1);
// }

// Global vector to store the polynomials
std::vector<poly_decl_t> Polynomials;

std::vector<Token> id_list; // for storing the ids of poly_name

struct Comp_input
{
    vector <Token> Tasks;
    vector <poly_decl_t> Polynomials;
    vector <Exec> Execute_stmt;
    vector <int> Inputs;
};


struct Exec
{
    vector <Token> Inputs;
    vector <Token> Outputs;
    vector <Assignment> Assignments;
};
struct Assignment
{
    Token output_id;
    Token poly_id;
    vector <Token> arguments;
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
        syntax_error();
    return t;
}
Exec execlist;
vector < Assignment > Assignments;  

// Parsing

// This function is simply to illustrate the GetToken() function
// you will not need it for your project and you can delete it
// the function also illustrates the use of peek(1)
void Parser::ConsumeAllInput()
{
    Token token;
    int i = 1;

    token = lexer.peek(i);
    token.Print();
    while (token.token_type != END_OF_FILE)
    {
        i = i + 1;
        token = lexer.peek(i);
        token.Print();
    }

    token = lexer.GetToken();
    token.Print();
    while (token.token_type != END_OF_FILE)
    {
        token = lexer.GetToken();
        token.Print();
    }
}
int main()
{
    // note: the parser class has a lexer object instantiated in it. You should not be declaring
    // a separate lexer object. You can access the lexer object in the parser functions as shown in the
    // example method Parser::ConsumeAllInput
    // If you declare another lexer object, lexical analysis will not work correctly
    Parser parser;

    parser.parse_input();


    //this->ConsumeAllInput();

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
this->expect(TASKS);


 vector<int> taskNumbers;  // Store the numbers in a vector

 Token currentToken = lexer.peek(1);

while (currentToken.token_type == NUM) {
        currentToken = lexer.GetToken();   // Consume the number token
        try {
            int num = stoi(currentToken.lexeme); // Convert lexeme to integer
            taskNumbers.push_back(num);
        } catch (const std::invalid_argument& e) {
            // Handle error: Invalid number format
             syntax_error();
        } catch (const std::out_of_range& e) {
            // Handle error: Number out of range
            syntax_error();
        }
        currentToken = lexer.peek(1); //peek to see if there are more numbers after current number
    }
    std::cout << "Tasks: " +  taskNumbers.size() << std::endl;

}


void Parser::parse_poly_section()
{
    this->expect(POLY);

    // Parse the polynomial sectin
      
      parse_poly_decl_list();
     
    //if there are multiple declaration we add the declarations to a list
    
}

void Parser::parse_poly_decl_list()
{
// poly_dec_list -> poly_decl
// poly_dec_list -> poly_decl poly_dec_list
poly_decl_t polyDecl = parsePolyDecl();
Polynomials.push_back(polyDecl); // parses one declaration
Token t = lexer.peek(1);
if (t.token_type == ID) // ID is the start of a new poly_decl_list
{
    bool found = false;
    std::vector<int> error_lines;
    for (const auto& token : id_list) {
        if (token.lexeme == t.lexeme) {
            found = true;
            error_lines.push_back(token.line_no);
        }
    }
    if (found) {
        error_lines.push_back(t.line_no);
        semantic_error("Code 1", error_lines);
    }
    parse_poly_decl_list();

parse_poly_decl_list();
} else if (t.token_type == EXECUTE) // polynomial declaration list
return;
// is followed by EXECUTE
}
poly_decl_t Parser::parsePolyDecl() {
    poly_decl_t decl;
    decl.header = parsePolyHeader();
    expect(EQUAL);
    std::vector<Token> terms = parsePolyBody();
    if (decl.header.variables.empty()) {
        //all the terms in the body must be x
        for (const Token& term : terms) {
            if (term.token_type == NUM || term.lexeme != "x" || term.token_type == MINUS|| term.token_type == PLUS || term.token_type == POWER || term.token_type == LPAREN  || term.token_type == RPAREN) {
                decl.terms.push_back(term);
            }
            else
            {
                semantic_error("Code 2", {term.line_no});
            }
        } 
    }
    for (Token& term : terms) {
        decl.terms.push_back(term);
    }
    expect(SEMICOLON);
    Polynomials.push_back(decl); // Add to global vector
    return decl;
}

struct poly_header_t Parser::parsePolyHeader() {
        poly_header_t header;
        
        Token polyNameId = parsePolyName();
        header.poly_name = polyNameId;

        std::string polyName = polyNameId.lexeme;
        std::vector<std::string> variables;
        id_list.push_back(polyNameId); // add the id to the list
        
        if (lexer.peek(1).token_type == LPAREN) {
            lexer.GetToken(); // Consume LPAREN
            variables = this->parseIdList();
            this->expect(RPAREN); // Expect RPAREN
        }

        std::cout << "Polynomial: " << polyName;
        if (!variables.empty()) {
            std::cout << "(";
            for (size_t i = 0; i < variables.size(); ++i) {
                std::cout << variables[i];
                if (i < variables.size() - 1) {
                    std::cout << ", ";
                }
            }
            std::cout << ")";
        }
        std::cout << " = ";
        header.variables = variables;
        return header;
    }


Token Parser::parsePolyName() {
        Token token = lexer.GetToken();
        this->expect(ID);
        return token;
    }


std::vector<std::string> Parser::parseIdList() {
        std::vector<std::string> ids;
        ids.push_back(parseId());
        while (lexer.peek(1).token_type == COMMA) {
            lexer.GetToken(); // Consume COMMA
            ids.push_back(parseId());
        }
        return ids;
    }

std::string Parser::parseId() {
        Token token = lexer.GetToken();
        this->expect(ID);
        return token.lexeme;
    }

std::vector<Token> Parser::parsePolyBody() {
        std::vector<Token> terms;
        parseTermList(terms); // Pass terms vector to collect tokens
        return terms;
    }

void Parser::parseTermList(std::vector<Token>& terms) {
    parseTerm(terms);
    parseTermListPrime(terms);
}
    

void Parser::parseTermListPrime(std::vector<Token>& terms) {
    Token t = lexer.peek(1);
    if (t.token_type == PLUS || t.token_type == MINUS) {
        Token op = lexer.GetToken();
        terms.push_back(op); // Add operator token
        parseTerm(terms);
        parseTermListPrime(terms);
    }
}

void Parser::parseTerm(std::vector<Token>& terms) {
    // Handle coefficient or minus sign
    if (lexer.peek(1).token_type == NUM) {
        Token numToken = lexer.GetToken();
        terms.push_back(numToken);
    } else if (lexer.peek(1).token_type == MINUS) {
        Token minusToken = lexer.GetToken();
        terms.push_back(minusToken);
    }
    parseMonomialList(terms);
}

void Parser::parseMonomialList(std::vector<Token>& terms) {
    while (lexer.peek(1).token_type == ID || lexer.peek(1).token_type == LPAREN) {
        parseMonomial(terms);
    }
}


/**
 * @brief Parses a monomial from the input.
 * 
 * This function handles the parsing of a monomial, which can either be an identifier (ID) 
 * or a parenthesized term list followed by an optional exponent. The function uses the lexer 
 * to get tokens and determine the structure of the monomial.
 * 
 * The function performs the following steps:
 * 1. Checks if the next token is an identifier (ID). If so, it consumes the token and stores the lexeme.
 * 2. If the next token is not an ID, it expects a left parenthesis (LPAREN), parses a term list, 
 *    and expects a right parenthesis (RPAREN). If the RPAREN is not found, it calls `syntax_error()`.
 * 3. Checks if the next token is a power operator (POWER). If so, it consumes the token, expects a number (NUM) 
 *    token for the exponent, and converts the lexeme to an integer. If the conversion fails, it calls `syntax_error()`.
 * 
 * @throws std::runtime_error if the expected tokens are not found or if the exponent conversion fails.
 */
void Parser::parseMonomial(std::vector<Token>& terms) {
    if (lexer.peek(1).token_type == ID) {
        Token idToken = lexer.GetToken();
        terms.push_back(idToken);
    } else {
        // Handle parenthesized terms
        Token lparen = lexer.GetToken();
        terms.push_back(lparen);
        parseTermList(terms);
        Token rparen = lexer.GetToken();
        if (rparen.token_type != RPAREN) {
            syntax_error();
        }
        terms.push_back(rparen);
    }

    // Handle exponent
    if (lexer.peek(1).token_type == POWER) {
        Token powerToken = lexer.GetToken();
        terms.push_back(powerToken);
        Token expToken = lexer.GetToken();
        if (expToken.token_type != NUM) {
            syntax_error();
        }
        terms.push_back(expToken);
    }
}



void Parser::parse_execute_section()
{
// execute_section -> EXECUTE { execute_list }
this->expect(EXECUTE);
while (lexer.peek(1).token_type != INPUTS && lexer.peek(1).token_type != END_OF_FILE) {
        statementList();
    }
}

void Parser::statementList()
{
   while (lexer.peek(1).token_type != SEMICOLON) {
        statement();
    }
    this->expect(SEMICOLON);
    execlist.Assignments = Assignments;
    check_semantic_error3();
    
}
void Parser::check_semantic_error3()
{
    vector <int> error_lines;
    // check if all the assignents.output id are also in polynomials vector->poly_name
    for (auto& assignment : Assignments) {
        bool found = false;
        for (auto& polynomial : Polynomials) {
            if (assignment.output_id.lexeme == polynomial.header.poly_name.lexeme) {
                found = true;
                break;
            }
        }
        if (!found) {
            error_lines.push_back(assignment.output_id.line_no);
        }
    }

    if (!error_lines.empty()) {
        semantic_error("Code 3", error_lines);
    }
}


void Parser::statement()
{
    if (lexer.peek(1).token_type == INPUTS) {
        inputStatement();
    } else if (lexer.peek(1).token_type == OUTPUT) {
        outputStatement();
    } else if (lexer.peek(1).token_type == ID) {
        assignmentStatement();
    } else {
        throw std::runtime_error("Invalid statement on line " + std::to_string(lexer.peek(1).line_no));
    }
}


void Parser::inputStatement()
{
    this->expect(INPUTS);
    Token id = this->expect(ID);

    execlist.Inputs.push_back(id);

    std::cout << "Input: " << id.lexeme << std::endl;
}

void Parser::outputStatement()
{
    this->expect(OUTPUT);
    Token id = this->expect(ID);
    execlist.Outputs.push_back(id);

    std::cout << "Output: " << id.lexeme << std::endl;
}

void Parser::assignmentStatement()
{
    Token output_id =  this->expect(ID);
    Assignment newAssignment;
    newAssignment.output_id = output_id;
     
    if(lexer.peek(1).token_type == LPAREN)
    {
       vector <string> vars = parseIdList();
    }
    {
        syntax_error();
    }


    this->expect(EQUAL);

   Token poly_id = this->expect(ID);
   newAssignment.poly_id = poly_id;

    // either expect (num) or expect (id)

    //x = F(X,W)

    this->expect(LPAREN);
    vector <Token> arguments = argument_list();
    newAssignment.arguments = arguments;   

    Assignments.push_back(newAssignment);

    
   
    std::cout << "Assignment: " << output_id.lexeme << " = " << std::endl;
}



vector <Token> Parser::argument_list()
{
//     argument_list -> argument
// argument_list -> argument COMMA argument_list
// argument ->  ID
// argument -> NUM
// argument -> poly_evaluation

vector<Token> arguments;
    while(lexer.peek(1).token_type != RPAREN) {
        if (lexer.peek(1).token_type == ID) {
            arguments.push_back(this->expect(ID));

        } else if (lexer.peek(1).token_type == NUM) {
            arguments.push_back(this->expect(NUM));
        } 
        else {
            // throw std::runtime_error("Invalid argument on line " + std::to_string(lexer.peek(1).line_no));
            syntax_error();
        }

        if (lexer.peek(1).token_type == COMMA) {
            this->expect(COMMA);
        }
}
return arguments;
}





void Parser::parse_inputs_section()
{
    this->expect(INPUTS);
    // Parse the inputs section
    while (lexer.peek(1).token_type != END_OF_FILE) {
        parseInput();
    }
}

void Parser::parseInput()
{
    while(lexer.peek(1).token_type != END_OF_FILE) {
        parseId();
    }
}

