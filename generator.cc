#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <random>

#include "json11.hpp"

// Seed with a real random value, if available
std::random_device rrr;

/*
Rule:
    name
    [phrase]

name: string
phrase: string* Rule string*
string: [a-zA-Z\-]
[]: array
*/
struct Phrase;

struct Rule{
    std::string Name{""};
    std::vector<Phrase> Phrases{};
    Rule(const std::string& name):Name(name){
    }
};

struct Node{
    std::string Words{""};
    std::shared_ptr<Rule> pRule;
};

struct Phrase{
    std::vector<Node> Nodes;
    
    std::string parse_rule_name(const std::string& t, int & j){
        //parse the rule name from 1st char after # to the next #
        int i=j;
        while('#'!= t[j] && j < t.size())
            ++j;
        bool found = (t[j]=='#');
        if(!found){
            throw std::runtime_error(std::string("illformed rulename, no delimiting # :>")+t);
        }

        ++j; //skip '#'
        return t.substr(i, j-i-1);
    }

    void parse(const std::string& text, std::map<std::string, std::shared_ptr<Rule>>& rules){
        int i=0,j=0;
        while(j < text.size()){
          while('#'!= text[j] && j< text.size()){
            ++j;
          }
          
          {//insert the text node
              Node text_node;
              text_node.Words = text.substr(i,j-i);
              std::cout << "normal-text=["<< text_node.Words <<"]\n";
              Nodes.push_back(text_node);
          }

          bool found = (text[j]=='#');
          if(found){//insert the rule node
            auto rule_name = parse_rule_name(text,++j);
            std::cout << "rule_name=[" <<rule_name <<"]\n"; 
            Node rule_node;
            if(rules.find(rule_name)==rules.end()){ //not found in the rules dictionary
                rule_node.pRule = std::make_shared<Rule>(rule_name);
                rules[rule_name] = rule_node.pRule;
            }else{//found in the rules dictionary
                rule_node.pRule = rules[rule_name];
            }
            Nodes.push_back(rule_node);
          }
          
          i = j;
        }
    }
};

void unfold_random_phrase(std::shared_ptr<Rule> p_rule){
    if (p_rule->Phrases.size()==0){
        std::cout << "no phrases. returning;";
        return;
    }
    //-- select random phrase
    std::default_random_engine e1(rrr());
    std::uniform_int_distribution<int> uniform_dist(0, p_rule->Phrases.size()-1);
    int phi = uniform_dist(e1);
    // std::cout << "\nUnfolding phrase = "<<phi<< "\n";
    //-- unfold that phrase
    for(const auto& n : p_rule->Phrases[phi].Nodes){
        if(n.pRule==nullptr){
            std::cout << n.Words;
        }else{
            // std::cout << "\nUnfolding rule = "<<n.pRule->Name << "\n";
            unfold_random_phrase(n.pRule);
        }
    }
}

int main(int argc, char* argv[]){
    /*
    Parse:
    Load Json
    for each key
        for each element
        parse Phrase
    */
    // if(argc<2)
    //     std::cout << "cmd line params: "<< argv[0] << " rule.json" << std::endl;

    // auto rules_file = std::string(argv[1]);
    auto rules_file = std::string("./spoken-time-ops-dsl.json");
    std::string line;
    std::stringstream ss;
    std::ifstream file(rules_file);
    while(std::getline(file,line)){
        auto k=line.find("//");
        if(k==std::string::npos){
            ss << line;
            // std::cout << line << "\n";
        }else if(k!=0){
            auto ll = (line.substr(0,k));
            // std::cout << ll << "\n";
            ss << ll;
        }
    }
    // std::cout << ss.str() << "\n\n";
    //parsed content would be stored here
    std::map<std::string,std::shared_ptr<Rule>> rule_set;

    std::string err;
    auto rules_set_json = json11::Json::parse(ss.str(), err);
    if(!err.empty()){
        std::cout << "error parsing json" << std::endl;
    }else{
        for(auto const& k : rules_set_json.object_items()){
            std::cout << "rule-name-json="<<k.first << "\n";
            std::shared_ptr<Rule> r = nullptr;
            if(rule_set.find(k.first)==rule_set.end()){
                r = std::make_shared<Rule>(k.first);
                rule_set[k.first] = r;
            }else{
                r = rule_set[k.first];            
            }
            //-- set the phrasess
            for(auto const& v : k.second.array_items()){
                Phrase ph;
                // std::cout << v.string_value() << std::endl;
                ph.parse(v.string_value(),rule_set);
                r->Phrases.push_back(ph);
            }
            std::cout << "****** setting new rule:["<<k.first <<"]\n";
            std::cout << "****** phrase_ count="<< r->Phrases.size()<<"\n";
        }
        std::cout << "\n";
    }

    /*
    Production:
    find starting rule,
    randomly select a phrase
    unfold phrase recursively
    */    
    std::cout << "-----------------------------------------------------------\n";
    std::cout << "rule-set key count="<< rule_set.size()<<"\n";

    if(rule_set.find("origin")!=rule_set.end()){
        // std::cout << "found starting rule\n";
        const auto& rule = rule_set["origin"];
        // std::cout << "ptr=" << rule << "\n";
        while(true){
        unfold_random_phrase(rule);
        std::cout << std::endl;
        }
    }
}