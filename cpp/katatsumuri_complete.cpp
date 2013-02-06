#include <memory>
#include <vector>
#include <utility>
#include <string>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

int main(int argc, char *argv[]){
    const int argm = 6;
    if(argc < argm){ return 0; }
    const std::string
        clang_path  = argv[1],
        target_path = argv[2],
        str_line    = argv[3],
        str_col     = argv[4];
    int clang_output_line_length = std::atoi(argv[5]);

    if(clang_output_line_length < 0x1000){ clang_output_line_length = 0x1000; }
    std::unique_ptr<char[]> buffer(new char[clang_output_line_length]);

    std::string other_options;
    for(int i = argm; i < argc; ++i){
        other_options += " ";
        other_options += argv[i];
    }

#ifdef _MSC_VER
    const char *drop = "2> nul";
#   define popen _popen
#   define pclose _pclose
#else
    const char *drop = "2> /dev/null";
#endif

    std::string command = clang_path + " -cc1 -std=c++11  -fsyntax-only -code-completion-at=" + target_path + ":" + str_line + ":" + str_col + " " + target_path + " " + other_options + " " + drop;

    auto pipe_deleter = [](std::FILE *pipe){ pclose(pipe); };
    std::unique_ptr<std::remove_pointer<std::FILE*>::type, decltype(pipe_deleter)> pipe(popen(command.c_str(), "r"));
    if(!&*pipe){ return 0; }

    std::string log_line;
    while(!std::feof(&*pipe)){
        if(std::fgets(&buffer[0], clang_output_line_length, &*pipe)){
            log_line = &buffer[0];
        }else{
            continue;
        }

        std::size_t i = log_line.find(" : ");
        if(i == log_line.npos){ continue; }
        if(log_line.find("COMPLETION: ") == 0){
            log_line = log_line.substr(12, log_line.size() - 12);
            i -= 12;
        }else{ continue; }

        auto del = [](std::string &str, const std::string &target, const int space){
            std::size_t off = 0;
            for(; ; ){
                std::size_t i = str.find(target, off);
                if(i == str.npos){ break; }
                str.erase(str.begin() + i, str.begin() + i + target.size());
                if(space && str[i - 1] != '*' && str[i - 1] != '&'){
                    str.insert(i, " ");
                    ++i;
                }
                off = i;
            }
            return std::ref(str);
        };

        auto rep = [](std::string &str, const std::string &p, const std::string &q, const int alnum){
            std::size_t off = 0;
            for(; ; ){
                std::size_t i = str.find(p, off);
                if(i == str.npos){ break; }
                if(alnum){
                    if((i > 0 && !std::isalnum(str[i - 1])) || (i < str.size() - 1 && !std::isalnum(str[i + 1]))){
                        str.replace(i, p.size(), q);
                    }
                }else{
                    str.replace(i, p.size(), q);
                }
                off = i + q.size();
            }
            return std::ref(str);
        };

        auto def = [](std::string &str){
            std::size_t off = 0;
            for(; ; ){
                std::size_t i = str.find("{#", off);
                if(i == str.npos){ break; }
                str.insert(str.find("#>", i), " = def");
                off = i + 2;
            }
            return std::ref(str);
        };

        std::string word = log_line.substr(0, i);
        if(word == "Pattern"){
            continue;
        }
        if(word.size() > 8 && word.find("(Hidden)", word.size() - 8) == word.size() - 8){
            continue;
        }
        std::string menu = log_line.substr(i + 3, log_line.size() - i - 3);
        del(del(del(del(del(del(def(rep(menu, "typename", "class", 1)), "[#", 0), "#]", 1), "<#", 0), "#>", 0), "{#", 0), "#}", 0);
        std::cout << "<katatsumuri>" << word << "#" << menu;
    }

    return 0;
}
