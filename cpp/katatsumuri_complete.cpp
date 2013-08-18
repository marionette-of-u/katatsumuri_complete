#include <memory>
#include <map>
#include <utility>
#include <string>
#include <sstream>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

struct buffer_element{
    buffer_element() : menu(), num(0){}

    // menu.
    std::string menu;
    
    // 他のメンバの数.
    std::size_t num;
};

// 1.   clang 実行ファイルの path.
// 2.   補完対象のファイルの path.
// 3.   補完するキャレットの列.
// 4.   補完するキャレットの行.
// 5.   1 行の最大文字列数.
// 6.   split each signature flag = 0 or 1.
// 7... clang 実行ファイルに渡すその他のオプション.

int main(int argc, char *argv[]){
    // プログラム起動時, 引数を最低 6 つ設定しなければいけない.
    const int argm = 7;
    if(argc < argm){
        return 0;
    }

    const std::string
        // clang 実行ファイルの path.
        clang_path  = argv[1],

        // 補完対象のファイルの path.
        target_path = argv[2],

        // 補完するキャレットの列.
        str_line    = argv[3],

        // 補完するキャレットの行.
        str_col     = argv[4],

        // <katatsumuri>
        signature_katatsumuri = "<katatsumuri>";

    // 補完表示最大数.
    int clang_output_line_length = std::atoi(argv[5]);
    if(clang_output_line_length < 0x1000){
        clang_output_line_length = 0x1000;
    }
    std::unique_ptr<char[]> buffer(new char[clang_output_line_length]);

    // clang 実行ファイルに渡すその他のオプション.
    std::string other_options;
    for(int i = argm; i < argc; ++i){
        other_options += " ";
        other_options += argv[i];
    }

    // ドロップするための設定.
    // msvc と posix 準拠の処理系とでは微妙に挙動が違うので
    // 簡単なマクロで統一する.
#ifdef _MSC_VER
    const char *drop = "2> nul";

    // msvc の場合は _popen, _pclose の名前も置き換えておく.
#   define popen _popen
#   define pclose _pclose
#else
    const char *drop = "2> /dev/null";
#endif

    // clang を呼び出すための設定.
    std::string command = clang_path + " -cc1 -std=c++11 -fsyntax-only -code-completion-at=" + target_path + ":" + str_line + ":" + str_col + " " + target_path + " " + other_options + " " + drop;

    // pipe の設定.
    auto pipe_deleter = [](std::FILE *pipe){
        pclose(pipe);
    };
    std::unique_ptr<std::remove_pointer<std::FILE*>::type, decltype(pipe_deleter)> pipe(popen(command.c_str(), "r"), pipe_deleter);
    if(!&*pipe){
        return 0;
    }

    std::string log_line;
    std::map<std::string, buffer_element> buffer_map;

    // 同名の補完ワードをシグネチャごとに分けて表示する場合は true.
    // そうでない場合は false.
    const bool split_each_signature = std::atoi(argv[6]) > 0;

    // 補完結果をひとつひとつ読み込む.
    while(!std::feof(&*pipe)){
        if(std::fgets(&buffer[0], clang_output_line_length, &*pipe)){
            log_line = &buffer[0];
        }else{
            continue;
        }

        // 書式に沿って出力から結果を編集する.
        std::size_t i = log_line.find(" : ");
        if(i == log_line.npos){
            continue;
        }
        if(log_line.find("COMPLETION: ") == 0){
            log_line = log_line.substr(12, log_line.size() - 12);
            i -= 12;
        }else{
            continue;
        }

        // str から target を削除する.
        // space の数だけ代わりに空白を入れる.
        auto del = [](std::string &str, const std::string &target, const int space){
            std::size_t off = 0;
            for(; ; ){
                std::size_t i = str.find(target, off);
                if(i == str.npos){
                    break;
                }
                str.erase(str.begin() + i, str.begin() + i + target.size());
                // TypeName ∧ !pointer ∧ !reference で space の分だけ空白を入れる.
                if(space && str[i - 1] != '*' && str[i - 1] != '&'){
                    str.insert(i, " ");
                    ++i;
                }
                off = i;
            }
            return std::ref(str);
        };

        // str 内部の p を q に置き換える.
        auto rep = [](std::string &str, const std::string &p, const std::string &q, const int alnum){
            std::size_t off = 0;
            for(; ; ){
                std::size_t i = str.find(p, off);
                if(i == str.npos){
                    break;
                }
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

        // デフォルト引数を処理する.
        auto def = [](std::string &str){
            std::size_t off = 0;
            for(; ; ){
                std::size_t i = str.find("{#", off);
                if(i == str.npos){
                    break;
                }
                str.insert(str.find("#>", i), " = def");
                off = i + 2;
            }
            return std::ref(str);
        };
        
        std::string word;
        
        // "Pattern : ..." の場合は何もしない.
        word = log_line.substr(0, i);
        if(word == "Pattern"){
            continue;
        }

        // "(Hidden)..." の場合も何もしない.
        if(word.size() > 8 && word.find("(Hidden)", word.size() - 8) == word.size() - 8){
            continue;
        }

        // 一連の処理を適用する.
        std::string menu = log_line.substr(i + 3, log_line.size() - i - 3);
        del(del(del(del(del(del(del(def(rep(menu, "typename", "class", 1)), "[#", 0), "#]", 1), "<#", 0), "#>", 0), "{#", 0), "#}", 0), "\n", 0);

        if(split_each_signature){
            // 標準出力に結果を出力する.
            // 特に "<katatsumuri>" である必要はない.
            std::cout << signature_katatsumuri << word << "#" << menu << "\n";
        }else{
            // まだ出力しない.
            buffer_element element;
            element.menu = menu;
            if(!buffer_map.insert(std::make_pair(word, element)).second){
                ++buffer_map[word].num;
            }
        }
    }

    // buffer_map の内容を出力する.
    if(!split_each_signature){
        for(auto iter = buffer_map.begin(); iter != buffer_map.end(); ++iter){
            std::cout << signature_katatsumuri << iter->first << "#" << iter->second.menu;
            if(iter->second.num > 0){
                std::ostringstream oss;
                oss << ", other " << iter->second.num << " member(s).";
                std::cout << oss.str();
            }
            std::cout << "\n";
        }
    }

    return 0;
}
