#include <fstream>
#include <assert.h>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <iostream>

/// SIMPLE ASSEMBLER
using namespace std;

ifstream in("prog.in");
ofstream out("prog.out");

inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

inline void panic(string s){
    cerr << s << endl;
    exit(122);
}

enum Instruction : uint8_t{
    NOOP, TOA, TOB, SUM, SUB, FROMA, FROMB, JMP, CHNGA, CHNGB, AND,    JMPC=0xE, JMPZ
};
map<string, Instruction> inst_mnemonics = {

    { "TOA", TOA },
    { "TOB", TOB },
    { "SUM", SUM },
    { "SUB", SUB },
    { "FROMA", FROMA },
    { "FROMB", FROMB },
    { "JMP", JMP },
    { "CHNGA", CHNGA },
    { "CHNGB", CHNGB },
    { "AND",   AND },
    { "JMPC", JMPC },
    { "JMPZ", JMPZ },

};

int curr_addr = 0;


inline void insert_instruction(Instruction inst, int immediate){
    assert(immediate < (1 << 4));
    out << "0x" << hex <<  +curr_addr << ": ";
    out << "0x" << hex <<  +((inst << 4) | immediate) << '\n';
    curr_addr++;
}

inline void insert_data(int data){
    out << "0x" << hex <<  +curr_addr << ": ";
    out << "0x" << hex << +data << '\n';
    curr_addr++;
}
/// NOTE: This is facky AF
/// For example i don't think you can't have the letter 'x' in labels or instruction mnemonics because we use it to identify numbers, also all numbers must be written in hex
/// You also can't use '[',  ']',  '{',  '}' in names, because they get 'ignored'
int main(){
map<string, int> lblmap;
map<string, int> defmap;

vector<string> file;
string line;
int lineno = 0;

while(getline(in, line ,'\n')){
    lineno++;
    file.push_back(line);

    string buf;
    bool is_hex = false;
    bool is_def = false;
    int dv = -1;
    for(char c : line){
        if(c == ';') break;
        if(buf == "#define "){
                is_def = true;
                buf = "";
        }
        if(c == 'x'){ is_hex = true; buf = ""; }
        if(c == ':'){
            trim(buf);
            if(is_hex){
                int a = -1;
                stringstream ss;
                ss << hex << buf;
                ss >> a;
                if(curr_addr < a) curr_addr = a;
                is_hex = false;
            }else lblmap[buf] = curr_addr;
            buf = "";
        }
        if(is_def){
            if(c == ' '){
                stringstream ss;
                ss << hex << buf;
                ss >> dv;
                buf = "";
            }
        }
        if(c != ':' && c != 'x') buf += c;
    }

    if(is_def){
            if(dv == -1){
                cerr << "Define on line: " << lineno << ", without number!" << endl;
                panic("");
            }
            trim(buf);
            defmap[buf] = dv;
            continue;
    }
    trim(buf);
    if(buf == "") continue;
    curr_addr++;
}

curr_addr = 0;
lineno = 0;
for(string line : file){
    lineno++;
    string buf;
    string backup;
    /// Parse numbers and addrs
    bool is_hex = false;
    bool contains_lead = false;
    for(char c : line){
        if(c == '#') break;
        if(c == ';') break;
        if(c == 'x'){ is_hex = true; backup += buf +"x"; buf = ""; }
        if(c == ':'){
            if(is_hex){
                int a = -1;
                stringstream ss;
                ss <<  hex << buf;
                ss >> a;
                if(curr_addr < a) curr_addr = a;
                is_hex = false;
                buf = "";
            } else{
                 buf = "";
            }
            contains_lead = true;
        }

        if(c != ':' && c != 'x') buf += c;
    }
    if(!contains_lead && is_hex){
        trim(backup);
        buf = backup + buf;
        if(backup != "0x") is_hex = false; // If the backup only contains the preceeding of a number, don't disable is_hex
    }
    trim(buf);
    if(buf == "") continue;
    /// Parse rest
    // Now parse actual thing knowing a
    if(is_hex){
        // Insert data
        stringstream ss;
        ss << hex << buf;
        int data;
        ss >> data;
        insert_data(data);
    }else{
        // Insert instruction
        trim(buf);
        Instruction ins;
        string part = "";
        bool is_hex_val = false;
        bool is_def_val = false;
        bool is_lbl_val = false;
        bool is_ins = false;
        for(char c : buf){
            if(c == '[') is_def_val = true;
            if(c == '{') is_lbl_val = true;
            if(c == 'x'){ is_hex_val = true; part = ""; }
            if(c == ' '){
                if(is_ins) continue;
                auto vall = inst_mnemonics.find(part);
                if(vall == inst_mnemonics.end()){
                    cerr << "Mnemonic: \"" << part << "\", on line: " << lineno << ", not found!" << endl;
                    panic("");
                }
                ins = (*vall).second;
                part = "";
                is_ins = true;
            }
            if(c != ' ' && c != '[' && c != ']' && c != 'x' && c != '{' && c != '}') part += c;
        }
        trim(part);

        if(!is_ins && is_lbl_val){
            auto vall = lblmap.find(part);
            if(vall == lblmap.end()){
                    cerr << "Could not find label: \"" << part << "\", on line: " << lineno << "!";
                    panic("");
            }
            insert_data((*vall).second);
            continue;
        }

        if(!is_ins && part != ""){ // Naked instructions with no arguments
            auto vall = inst_mnemonics.find(part);
            if(vall == inst_mnemonics.end()){
                cerr << "Mnemonic: \"" << part << "\", on line: " << lineno << ", not found!" << endl;
                panic("");
            }
            ins = (*vall).second;
            part = "";
            is_ins = true;
        }
        if(is_ins && part == ""){
            insert_instruction(ins, 0);
            continue;
        }
        if(is_ins && is_def_val){
            auto vall = defmap.find(part);
            if(vall == defmap.end()){
                    cerr << "Could not find define: \"" <<  part <<  "\", used on line: " << lineno << "!" << endl;
                    panic("");
            }
            insert_instruction(ins, (*vall).second);
            continue;
        }
        if(is_ins && is_hex_val){
            stringstream ss2;
            ss2 << part;
            int ival;
            ss2 >> ival;
            insert_instruction(ins, ival);
            continue;
        }

        if(is_ins && is_lbl_val){
            auto vall = lblmap.find(part);
            if(vall == lblmap.end()){
                cerr << "Could not find label: \"" << part << "\", used on line: " << lineno << "!" << endl;
                panic("");
            }
            insert_instruction(ins, (*vall).second);
            continue;
        }
        panic("Could not parse line!");
    }
}

}
