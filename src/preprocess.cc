/*
 * Copyright 2024 Jim Haslett
 *
 * This file is part of the 6502 C Compiler implementation.
 *
 * 6502 C Compiler is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * 6502 C Compiler is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * the 6502 C Compiler. If not, see <https:// www.gnu.org/licenses/>.
 */


/**
 * This project is based on the ANSI X3.159-1989 standard, also known as ANSI C or C89
 * published at https://nvlpubs.nist.gov/nistpubs/Legacy/FIPS/fipspub160.pdf
 * 
 * Specifics for this module were also developed from https://en.wikipedia.org/wiki/C_preprocessor
 */

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <stdexcept>
#include <filesystem>

#include "language.h"
#include "helpers.h"

#define DEBUG 0
#define TOKENIZATION_DEBUG 0

std::string preproecess_file(char* filename);  /* forward delcaration */
std::filesystem::path current_path_; /* Global variable for current directory */

void replace_digraphs(std::string &in_buffer) {
    for (int i = 1; i < in_buffer.length() - 1; i++) {
        if (in_buffer[i] == '?' && in_buffer[i-1] == '?') {
            switch (in_buffer[i+1])
            {
            case '=':
                in_buffer[i-1] = '#';
                in_buffer.erase(i, 2);
                i--;
                break;
            case '/':
                in_buffer[i-1] = '\\';
                in_buffer.erase(i, 2);
                i--;
                break;
            case '\'':
                in_buffer[i-1] = '^';
                in_buffer.erase(i, 2);
                i--;
                break;
            case '(':
                in_buffer[i-1] = '[';
                in_buffer.erase(i, 2);
                i--;
                break;
            case ')':
                in_buffer[i-1] = ']';
                in_buffer.erase(i, 2);
                i--;
                break;
            case '!':
                in_buffer[i-1] = '|';
                in_buffer.erase(i, 2);
                i--;
                break;
            case '<':
                in_buffer[i-1] = '{';
                in_buffer.erase(i, 2);
                i--;
                break;
            case '>':
                in_buffer[i-1] = '}';
                in_buffer.erase(i, 2);
                i--;
                break;
            case '-':
                in_buffer[i-1] = '~';
                in_buffer.erase(i, 2);
                i--;
                break;                                                                                                                                           
            default:
                break;
            }
        }
    }
}

void splice_lines(std::string &in_buffer) {
    for (int i = 1; i < in_buffer.length(); i++) {
        if (in_buffer[i] == '\n' && in_buffer[i-1] == '\\') {
            in_buffer.erase(i-1, 2);
            i--;
        }
    }
    
    if (in_buffer.back() != '\n') {
        in_buffer.append("\n");
    }
}

std::string tokenize(std::string &in_buffer) {
    std::stringstream out_buffer;
    std::stringstream token;

    bool preprocessor_directive = false;
    bool first_token_this_line = true;

    std::string debug_token_type = "";

    /* generic messaging variables */
    std::string message;
    int start_position;


    /* Parse in_buffer one character at a time */
    for (int i = 0; i < in_buffer.length(); i++) {


        /* Process Comments */
        if (in_buffer[i] == '/' && i + 1 < in_buffer.length() && ( 
                in_buffer[i+1] == '*' || in_buffer[i+1] == '/')) { 
            if (in_buffer[i+1] == '*') {  
                /* we found / * sequence and we are not already in a comment
                    nor are we in a quote block */
                start_position = i;
                i += 2;
                while (i < in_buffer.length() && !(in_buffer[i-1]=='*' && in_buffer[i]=='/')){
                    i++;
                }
                if (in_buffer[i-1]=='*' && in_buffer[i]=='/') {
                    /* We did find the end of the comment block */
                    out_buffer << " ";  // replace comment with single space
                    continue; // skip rest of loop and start over
                }
                message = "Comment block not terminated before end of buffer, started at ";
                message.append(std::to_string(start_position));
                throw std::invalid_argument(message);            
            }
            else if (in_buffer[i+1] == '/') {
                /* we found // sequence and we are not already in a comment
                    nor are we in a quote block */
                while (i < in_buffer.length() && in_buffer[i]!='\n'){
                    i++;
                }
                if (in_buffer[i] == '\n') {
                    /* we did find the end of line, thus ending the comment */
                    out_buffer << " \n"; // replace comment with single space, maintain newline
                    continue;
                }
                message = "Inline comment not terminated before end of buffer, started at ";
                message.append(std::to_string(start_position));
                throw std::invalid_argument(message);                   
            }
        }

        /* Newline Character */
        else if (in_buffer[i] == '\n') {
            out_buffer << "\n";
            preprocessor_directive = false;
            first_token_this_line = true;
        }

        /* Process String Literal */
        else if (in_buffer[i] == '"') {
            debug_token_type = "String Literal";
            first_token_this_line = false;
            token << "\"";
            start_position = i;
            i++;
            while(i < in_buffer.length() && (in_buffer[i] != '"' || in_buffer[i-1] == '\\') ) {
                token << in_buffer[i];
                i++;
            }
            token << "\"";
            if (!is_valid_string_literal(token.str())) {
                std::string message;
                message = "Invalid string literal token ";
                message.append(token.str());
                message.append(" found at ");
                message.append(std::to_string(start_position));
                throw std::invalid_argument(message);
            }

        }

       /* Process Header Name */
        else if (preprocessor_directive and in_buffer[i] == '<') {
            debug_token_type = "Header Name";
            first_token_this_line = false;
            token << "<";
            start_position = i;
            i++;
            while(i < in_buffer.length() && (in_buffer[i] != '>' || in_buffer[i-1] == '\\') ) {
                token << in_buffer[i];
                i++;
            }
            token << ">";
            if (!is_valid_header_name(token.str())) {
                std::string message;
                message = "Invalid header name token ";
                message.append(token.str());
                message.append(" found at ");
                message.append(std::to_string(start_position));
                throw std::invalid_argument(message);
            }

        }


        /* Process Character Literal */
        else if (in_buffer[i] == '\'') {
            debug_token_type = "Character Literal";
            first_token_this_line = false;
            token << "'";
            start_position = i;
            i++;
            while(i < in_buffer.length() && (in_buffer[i] != '\'' || in_buffer[i-1] == '\\') ) {
                token << in_buffer[i];
                i++;
            }
            token << "'";
            if (!is_valid_character_constant(token.str())) {
                std::string message;
                message = "Invalid character literal token ";
                message.append(token.str());
                message.append(" found at ");
                message.append(std::to_string(start_position));
                throw std::invalid_argument(message);
            }
        }   

        /* Process Identifier */
        else if (is_char_a_non_digit(in_buffer[i])){
            debug_token_type = "Identifier";
            first_token_this_line = false;
            token << in_buffer[i];
            i++;
            while(i < in_buffer.length() && (is_char_a_non_digit(in_buffer[i]) || is_char_a_digit(in_buffer[i]))){
                token << in_buffer[i];
                i++;
            }
            if(i < in_buffer.length()){
                i--;
            }
        }

        /* Process Preprocessing Numbers */
        else if (is_char_a_digit(in_buffer[i]) ||
                    i+1 < in_buffer.length() && in_buffer[i] == '.' && is_char_a_digit(in_buffer[i+1])){
            debug_token_type = "PP-Number";
            first_token_this_line = false;
            token << in_buffer[i];
            i++;
            while(i < in_buffer.length() && (is_char_a_digit(in_buffer[i]) || 
                                                is_char_a_non_digit(in_buffer[i]) ||
                                                in_buffer[i] == '.')){
                if((in_buffer[i] == 'e' || in_buffer[i] == 'E') && i + 1 < in_buffer.length()) {
                    if(in_buffer[i+1] == '+' || in_buffer[i+1] == '-') { //only time '+' or '-' permitted is after an 'e' or 'E'
                        token << in_buffer[i];
                        i++;
                    }
                }
                token << in_buffer[i];
                i++;                
            }
            i--;
        }

        /* Process Operators / Punctuators / Non-whitespace-characters */
        else if (not is_char_whitepsace(in_buffer[i])) {
            int last_valid_token_character = -1;
            int first_character = i;            
            while(i < in_buffer.length() && not is_char_whitepsace(in_buffer[i])){
                token << in_buffer[i];
                if(is_token_an_operator(token.str().c_str()) || is_token_a_punctuator(token.str().c_str())) {
                    last_valid_token_character = i;
                }                
                i++;    
            }
            if (last_valid_token_character > -1) {
                token.str(in_buffer.substr(first_character, last_valid_token_character+1-first_character));
                debug_token_type = "Operator/Punctuator";
                i = last_valid_token_character;  //set this back to the end of the token!
            }
            else {
                token.str(in_buffer.substr(first_character, 1));
                debug_token_type = "Other";
                i = first_character; //set this back as the token is only one character!
            }
            if(first_token_this_line && token.str() == "#"){
                preprocessor_directive = true;
            }
            first_token_this_line = false;
        }

        if (token.str().length() > 0) {
            if (TOKENIZATION_DEBUG) {
                if (preprocessor_directive) std::cout << "PPD ";
                std::cout << debug_token_type;
                std::cout << " : " << token.str() << std::endl;
            }
            out_buffer << " " << token.str();
            token.str(std::string());
        }
    }
    return out_buffer.str();
}


std::string execute_preprocessing_directives(std::string &in_buffer){
    int i_start, i_end;
    std::string line, token;
    std::stringstream out_buffer;

    i_start = 0;
    i_end = in_buffer.find('\n', i_start);
    while( i_end != in_buffer.npos) {  // got through, line by line
        /* get line contents into line buffer */
        line = in_buffer.substr(i_start, i_end - i_start);
        // std::cout << "EPD:" << line << std::endl; /* debug */

        int i = 0;
        token = get_next_token(line, i);
        if(token == "#") {
            token = get_next_token(line, i);
            // std::cout << "EPD::" << line[i] << std::endl; /* debug */
            std::cout << "EPD: DIRECTIVE:" << token << std::endl; /* debug */

            if(token == "include") {
                token = get_next_token(line, i);
                std::string filename;
                if(token[0] == '<'){
                    /* sandard include */
                }
                else if(token[0] == '"') {
                    /* local include */
                    filename = token.substr(1,token.length()-2);
                }
                else {
                    /* macro replacement */
                }
                std::filesystem::path file_path = current_path_ / filename;
                std::cout << "filename: " << file_path << std::endl;
                out_buffer << preproecess_file((char *)file_path.string().c_str());
            }
            else if(token == "define") {

            }
            else if(token == "ifndef") {

            }
            else if(token == "endif") {

            }
            else {
                std::string message;
                message = "Invalid preprocessing directive ";
                message.append(token);
                message.append("\n");
                message.append(line);
                throw std::invalid_argument(message);                
            }
        }
        else {
            /* not a preprocessor directive */
            out_buffer << line << "\n";
        }

        /* Get next line */
        i_start = i_end + 1;
        i_end = in_buffer.find('\n', i_start);
    }
    return out_buffer.str();
}

void preprocess(std::string &buffer) {
    /* Translation Phase 1 */
    replace_digraphs(buffer);

    /* Translation Phase 2 */
    splice_lines(buffer);

    /* Translation Phase 3 */
    buffer = tokenize(buffer);

    /* Translation Phase 4 */
    buffer = execute_preprocessing_directives(buffer);
}

std::string preproecess_file(char* filename){
    std::string buffer = get_file_contents(filename);
    preprocess(buffer);
    return buffer;
}

int main(int argc, char* argv[]) {
    char* filename = argv[argc - 1];
    std::cout << "Pre-Processing file " << filename << std::endl;

    current_path_ = std::filesystem::path(filename);
    current_path_.remove_filename();
    // std::filesystem::path newpath;
    // newpath = current_path_ / "bob.txt";

    std::string buffer = preproecess_file(filename);    

    std::cout << buffer << std::endl;

}
