#include<iostream> // Console output header file
#include<fstream> // File stream header file
#include<sstream> // String stream header file
#include<optional> // Optional header file
#include<vector> // Vector header file

enum class TokenType {
    _return,
    _comment,
    _int_lit,
    semicolon
};

std::string TokenTypeToString(TokenType type){
    switch (type){
        case TokenType::_return:
            return "return";
        case TokenType::_comment:
            return "comment";
        case TokenType::_int_lit:
            return "int_lit";
        case TokenType::semicolon:
           return "semicolon";
        default:
            return "Unknown token";
    }
}

struct Token {
    TokenType type;
    std::optional<std::string> value {std::nullopt};
};

std::vector<Token> tokenize(const std::string& input){
    std::vector<Token> tokens;
    std::stringstream current_token;
    for (size_t i = 0; i < input.length(); i++){
        char c = input.at(i);
        if (std::isalpha(c)){
            current_token << c;
            i++;
            while (std::isalnum(input.at(i)) && !isspace(input.at(i))){
                current_token << input.at(i);
                i++;
            }
            i--;
            if (current_token.str() == "return"){
                tokens.push_back({TokenType::_return});
                current_token.str(std::string());
            } else{
                std::cerr << "Unknown token: " << current_token.str() << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else if (std::isdigit(c)){
            current_token << c;
            i++;
            while (std::isdigit(input.at(i))){
                current_token << input.at(i);
                i++;
            }
            if (std::isspace(input.at(i))){
                tokens.push_back({TokenType::_int_lit, current_token.str()});
                current_token.str(std::string());
            }
            else if (input.at(i) == ';'){
                tokens.push_back({TokenType::_int_lit, current_token.str()});
                tokens.push_back({TokenType::semicolon});
                current_token.str(std::string());
            }
            else{
                std::cerr << "Unknown token: " << current_token.str() << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else if (c == '/'){  
            i++;
            if (input.at(i) == '/'){
                i++;
                while (input.at(i) != '\n'){
                    current_token << input.at(i);
                    i++;
                }
                i--;
                tokens.push_back({TokenType::_comment, current_token.str()});
                current_token.str(std::string());
            }
            else{
                std::cerr << "Unknown token: " << current_token.str()  << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else if (std::isspace(c) || c == '\n'){
            current_token.str(std::string());
            continue;
        } 
        else if (c == ';'){
            tokens.push_back({TokenType::semicolon});
        }
        else{
            std::cerr << "Unknown token: " << current_token.str() << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    return tokens;
}

std::string tokens_to_asm(const std::vector<Token>& tokens){
    std::stringstream asm_code;
    asm_code << "global _start\n_start:\n";
    for (int i = 0; i < tokens.size(); i++){
        const Token& token = tokens.at(i);
        if(token.type == TokenType::_return){
            if (i+1 < tokens.size() && tokens.at(i+1).type == TokenType::_int_lit){
                if (i+2 < tokens.size() && tokens.at(i+2).type == TokenType::semicolon){
                    asm_code << "   mov rax, 60\n";
                    asm_code << "   mov rdi, " << tokens.at(i+1).value.value() << "\n";
                    asm_code << "   syscall\n";
                }
            }
        }
    }
    return asm_code.str();
}

int main(int argc, char* argv[]){

    // No input file for compiler
    if (argc != 2){
        std::cerr << "Usage: a++ " << "<name>.cpp" << std::endl;
        return EXIT_FAILURE;
    }

    // Scope file reading for automatic file closing
    // Read file contents into a string
    std::string contents; {
        std::stringstream contents_stream;
        std::fstream input(argv[1], std::ios::in);
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }
    std::vector<Token> res = tokenize(contents + "\n"); 
    for (auto T : res){
        std::cout << TokenTypeToString(T.type) << " " << T.value.value_or("") << std::endl;
    }
    std::cout << std::endl;

    // Print the assembly code
    std::cout << tokens_to_asm(res) << std::endl;

    // Write the assembly code to a file
    {
        std::fstream file("out.asm", std::ios::out);
        file << tokens_to_asm(res);
    }

    // Compile and run the assembly code
    system("nasm -felf64 out.asm && ld -o out out.o");
    system("./out && echo $?");
    return EXIT_SUCCESS;
}