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
    
    // ���̃����o�̐�.
    std::size_t num;
};

// 1.   clang ���s�t�@�C���� path.
// 2.   �⊮�Ώۂ̃t�@�C���� path.
// 3.   �⊮����L�����b�g�̗�.
// 4.   �⊮����L�����b�g�̍s.
// 5.   1 �s�̍ő啶����.
// 6.   split each signature flag = 0 or 1.
// 7... clang ���s�t�@�C���ɓn�����̑��̃I�v�V����.

int main(int argc, char *argv[]){
    // �v���O�����N����, �������Œ� 6 �ݒ肵�Ȃ���΂����Ȃ�.
    const int argm = 7;
    if(argc < argm){
        return 0;
    }

    const std::string
        // clang ���s�t�@�C���� path.
        clang_path  = argv[1],

        // �⊮�Ώۂ̃t�@�C���� path.
        target_path = argv[2],

        // �⊮����L�����b�g�̗�.
        str_line    = argv[3],

        // �⊮����L�����b�g�̍s.
        str_col     = argv[4],

        // <katatsumuri>
        signature_katatsumuri = "<katatsumuri>";

    // �⊮�\���ő吔.
    int clang_output_line_length = std::atoi(argv[5]);
    if(clang_output_line_length < 0x1000){
        clang_output_line_length = 0x1000;
    }
    std::unique_ptr<char[]> buffer(new char[clang_output_line_length]);

    // clang ���s�t�@�C���ɓn�����̑��̃I�v�V����.
    std::string other_options;
    for(int i = argm; i < argc; ++i){
        other_options += " ";
        other_options += argv[i];
    }

    // �h���b�v���邽�߂̐ݒ�.
    // msvc �� posix �����̏����n�Ƃł͔����ɋ������Ⴄ�̂�
    // �ȒP�ȃ}�N���œ��ꂷ��.
#ifdef _MSC_VER
    const char *drop = "2> nul";

    // msvc �̏ꍇ�� _popen, _pclose �̖��O���u�������Ă���.
#   define popen _popen
#   define pclose _pclose
#else
    const char *drop = "2> /dev/null";
#endif

    // clang ���Ăяo�����߂̐ݒ�.
    std::string command = clang_path + " -cc1 -std=c++11 -fsyntax-only -code-completion-at=" + target_path + ":" + str_line + ":" + str_col + " " + target_path + " " + other_options + " " + drop;

    // pipe �̐ݒ�.
    auto pipe_deleter = [](std::FILE *pipe){
        pclose(pipe);
    };
    std::unique_ptr<std::remove_pointer<std::FILE*>::type, decltype(pipe_deleter)> pipe(popen(command.c_str(), "r"), pipe_deleter);
    if(!&*pipe){
        return 0;
    }

    std::string log_line;
    std::map<std::string, buffer_element> buffer_map;

    // �����̕⊮���[�h���V�O�l�`�����Ƃɕ����ĕ\������ꍇ�� true.
    // �����łȂ��ꍇ�� false.
    const bool split_each_signature = std::atoi(argv[6]) > 0;

    // �⊮���ʂ��ЂƂЂƂǂݍ���.
    while(!std::feof(&*pipe)){
        if(std::fgets(&buffer[0], clang_output_line_length, &*pipe)){
            log_line = &buffer[0];
        }else{
            continue;
        }

        // �����ɉ����ďo�͂��猋�ʂ�ҏW����.
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

        // str ���� target ���폜����.
        // space �̐���������ɋ󔒂�����.
        auto del = [](std::string &str, const std::string &target, const int space){
            std::size_t off = 0;
            for(; ; ){
                std::size_t i = str.find(target, off);
                if(i == str.npos){
                    break;
                }
                str.erase(str.begin() + i, str.begin() + i + target.size());
                // TypeName �� !pointer �� !reference �� space �̕������󔒂�����.
                if(space && str[i - 1] != '*' && str[i - 1] != '&'){
                    str.insert(i, " ");
                    ++i;
                }
                off = i;
            }
            return std::ref(str);
        };

        // str ������ p �� q �ɒu��������.
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

        // �f�t�H���g��������������.
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
        
        // "Pattern : ..." �̏ꍇ�͉������Ȃ�.
        word = log_line.substr(0, i);
        if(word == "Pattern"){
            continue;
        }

        // "(Hidden)..." �̏ꍇ���������Ȃ�.
        if(word.size() > 8 && word.find("(Hidden)", word.size() - 8) == word.size() - 8){
            continue;
        }

        // ��A�̏�����K�p����.
        std::string menu = log_line.substr(i + 3, log_line.size() - i - 3);
        del(del(del(del(del(del(del(def(rep(menu, "typename", "class", 1)), "[#", 0), "#]", 1), "<#", 0), "#>", 0), "{#", 0), "#}", 0), "\n", 0);

        if(split_each_signature){
            // �W���o�͂Ɍ��ʂ��o�͂���.
            // ���� "<katatsumuri>" �ł���K�v�͂Ȃ�.
            std::cout << signature_katatsumuri << word << "#" << menu << "\n";
        }else{
            // �܂��o�͂��Ȃ�.
            buffer_element element;
            element.menu = menu;
            if(!buffer_map.insert(std::make_pair(word, element)).second){
                ++buffer_map[word].num;
            }
        }
    }

    // buffer_map �̓��e���o�͂���.
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
