#include "SpadeScriptEngine.h"
#include <cmath>
#include <algorithm>
#include <cctype>
using namespace std;

// Evaluate basic integer math and conditions
string SpadeScriptEngine :: evaluate(string str, int skiplevel) {
        
    // Order of Operations: P E M D A S, Inequality, Equality, AND, OR
    //     With Unary checking every iteration

    int s = 0;
    int e = 0;

    if(skiplevel < 1)
    {
        str = trim(str," ");

        // Check for literal strings
        if(str[0] == '\"' && str[str.length()-1]=='\"')
            return trim(str,"\"");
        if(str[0] == '\'' && str[str.length()-1]=='\'')
            return trim(str,"\'");

        while(str.find_first_of(" ")!=string::npos)
            str.erase(str.find(" "),1);
        //str = trim(str," \t");
    }

    // Unary (every recursion)
    size_t cidx;
    while ((cidx = str.find("--")) != string::npos) {
        str.replace(cidx, 2, "+");
    }
    while ((cidx = str.find("+-")) != string::npos) {
        str.replace(cidx, 2, "-");
    }


    // P
    if(skiplevel < 2 && str.find_first_of("(")!=string::npos)
    {
        // TODO: Loop until no more parenthesis
        while(str.find_first_of("(") != string::npos) {
            //cout<<"Eval(P): "<<str<<endl;
            int depth = 0;
            int cdepth = 0;
            s = 0;
            e = 0;
            // find parenthesis with greatest depth
            /*for(unsigned int i=0; i<str.length(); ++i){
                if(str[i]=='('){
                    depth++;
                    if(depth>cdepth){
                        cdepth = depth;
                        s = i;
                    }
                }
                else if(str[i]==')') depth--;
            }*/
            s = str.find_last_of("("); // this'll work :/

            e = str.find(')', s);
            string pexp = str.substr(s+1,e-s-1);
            if(pexp.find_first_not_of("0123456789.") != string::npos || pexp.find_first_not_of("-",1) != string::npos) // contains a non-number
                pexp = evaluate(pexp,1); // recursive call, skipping parenthesis check
            
            str = str.substr(0,s) + pexp + str.substr(e+1);
            // replace solved with numerical value
        }

        str = evaluate(str,2);
        //cout<<"Eval(P)= "<<str<<endl;
    }
    // E
    /*else if(skiplevel < 3 && str.find("^")!=string::npos)
    {
        
    }*/
    // M D
    else if(skiplevel < 4 && str.find_first_of("*/")!=string::npos)
    {
        size_t idx;
        while((idx = str.find_first_of("*/")) != string::npos) {
            //cout<<"Eval(MD): "<<str<<endl;

            s=0;
            e=0;
            string n1,n2;

            //cout<<"string looks like "<<str<<endl;

            for(size_t i = idx-1;;--i) {
                if(i<=0)
                {
                    s = 0;
                    break;
                }
                else if(str[i]=='+')
                {
                    s = i+1;
                    break;
                }
                else if(str[i]=='-')
                {
                    s = i;
                    break;
                }
                else if(str[i]=='*' || str[i]=='/')
                {
                    s = i+1;
                    break;
                }
            }
            for(size_t i = idx+1;;++i) {
                if(i>=str.length()-1)
                {
                    e = str.length()-1;
                    break;
                }
                else if(str[i]=='+')
                {
                    e = i-1;
                    break;
                }
                else if(str[i]=='-' && i!=idx+1)
                {
                    e = i-1;
                    break;
                }
                else if(str[i]=='*' || str[i]=='/')
                {
                    e = i-1;
                    break;
                }
            }

            //e = (e==str.length()-1)?e:e+1;

            n1 = str.substr(s,idx-s);
            n2 = str.substr(idx+1,e-idx);
            //cout<<n1<<" MD "<<n2<<endl;
            //cout<<"fragment 1: "<<str.substr(0,s)<<endl;
            //cout<<"fragment 2: "<<((e==str.length()-1)?"":str.substr(e+1))<<endl;

            string f1 = str.substr(0,s);
            if(f1.length()>0 && f1[f1.length()-1] != '+' && f1[f1.length()-1] != '-')
                f1 += "+";
            string f2 = ((e==str.length()-1)?"":str.substr(e+1));
            
            //if(str[idx]=='*')
            //    str = f1 + itos(atoi(n1.c_str()) * atoi(n2.c_str())) + f2; //e+1
            //else
                str = f1 + itos(atoi(n1.c_str()) / atoi(n2.c_str())) + f2;

            //cout<<str<<endl;
        }
        //cout<<"Eval(MD)= "<<str<<endl;
        str = evaluate(str,4); // recursive ftw
    }
    // A S
    else if(skiplevel < 5 && str.find_first_of("+-")!=string::npos)
    {

        // REDO +- to work with condition-checking

        int total = 0;
        size_t idx = 0;

        if(str.find_first_of("+-")!=string::npos)
        {
            string n = "";
            n += str[0];
            total = 0;
            for(size_t i=1; i<str.length();i++) {
                if(str[i]=='+') {
                    total += atoi(n.c_str());
                    n = "";
                } else if(str[i]=='-') {
                    total += atoi(n.c_str());
                    n = "-";
                }
                else
                    n += str[i];
            }
            if(n.compare("")!=0)
                total += atoi(n.c_str());
        }
        str = itos(total);

        str = evaluate(str,5);
    }
    else if(skiplevel < 6 && str.find_first_of("<>")!=string::npos) {
        size_t idx;
        while((idx = str.find_first_of("<>")) != string::npos) {
            //cout<<"Eval(MD): "<<str<<endl;

            s=0;
            e=0;
            string n1,n2;
            bool allow_eq = (str[idx+1]=='=');

            //cout<<"string looks like "<<str<<endl;

            for(size_t i = idx-1;;--i) {
                if(i<=0)
                {
                    s = 0;
                    break;
                }
                else if(str[i]=='&' || str[i]=='|' || str[i]=='=' || str[i]=='>' || str[i]=='<' || str[i]=='!')
                {
                    s = i+1;
                    break;
                }
            }
            for(size_t i = idx+1;;++i) {
                if(i>=str.length()-1)
                {
                    e = str.length()-1;
                    break;
                }
                else if(str[i]=='&' || str[i]=='|' || str[i]=='>' || str[i]=='<' || str[i]=='!')
                {
                    e = i-1;
                    break;
                }
                else if(str[i]=='=')
                {
                    if(i>idx+1) {
                        e = i-1;
                        break;
                    }
                }
            }

            //e = (e==str.length()-1)?e:e+1;

            n1 = str.substr(s,idx-s);
            if(allow_eq)
                n2 = str.substr(idx+2,e-idx);
            else
                n2 = str.substr(idx+1,e-idx);

            string f1 = str.substr(0,s);
            string f2 = ((e==str.length()-1)?"":str.substr(e+1));
            
            if(str[idx]=='<') {
                if(allow_eq)
                    str = f1 + (( atoi(n1.c_str()) <= atoi(n2.c_str()) )?"1":"0") + f2;
                else
                    str = f1 + (( atoi(n1.c_str()) < atoi(n2.c_str()) )?"1":"0") + f2;
            }
            else if(str[idx]=='>') {
                if(allow_eq)
                    str = f1 + (( atoi(n1.c_str()) >= atoi(n2.c_str()) )?"1":"0") + f2;
                else
                    str = f1 + (( atoi(n1.c_str()) > atoi(n2.c_str()) )?"1":"0") + f2;
            }

            //cout<<str<<endl;
        }
        //cout<<"Eval(MD)= "<<str<<endl;
        str = evaluate(str,6); // recursive ftw
    }
    else if(str.find_first_of("=")!=string::npos) {
        size_t idx;
        while((idx = str.find_first_of("=")) != string::npos) {
            //cout<<"Eval(MD): "<<str<<endl;

            s=0;
            e=0;
            string n1,n2;
            bool is_not = (str[idx-1]=='!');
            if(is_not) idx -= 1;

            //cout<<"string looks like "<<str<<endl;

            for(size_t i = idx-1;;--i) {
                if(i<=0)
                {
                    s = 0;
                    break;
                }
                else if(str[i]=='&' || str[i]=='|' || str[i]=='=')
                {
                    s = i+1;
                    break;
                }
            }
            for(size_t i = idx+1;;++i) {
                if(i>=str.length()-1)
                {
                    e = str.length()-1;
                    break;
                }
                else if(str[i]=='&' || str[i]=='|')
                {
                    e = i-1;
                    break;
                }
                else if(str[i]=='=')
                {
                    if(i>idx+1) {
                        e = i-1;
                        break;
                    }
                }
            }

            n1 = str.substr(s,idx-s);
            n2 = str.substr(idx+2,e-idx); //+1

            string f1 = str.substr(0,s);
            string f2 = ((e==str.length()-1)?"":str.substr(e+1));
            
            bool is_equal = false;
            if(n1.compare(n2)==0) {
                is_equal = true;
            }

            if(!is_not)
                str = f1 + (is_equal?"1":"0") + f2;
            else
                str = f1 + (!is_equal?"1":"0") + f2;

            // old: (without string compare support)
                //str = f1 + (( atoi(n1.c_str()) == atoi(n2.c_str()) )?"1":"0") + f2;
                //str = f1 + (( atoi(n1.c_str()) != atoi(n2.c_str()) )?"1":"0") + f2;

        }
        str = evaluate(str,7);
    }
    else if(str.find_first_of("&&")!=string::npos) {
        // TODO: && not yet implemented
    }
    else if(str.find_first_of("||")!=string::npos) {
        // TODO: || not yet implemented
    }

    return str;
}

int SpadeScriptEngine :: doBlock(const string& function,
            const string& cmd,
            SpadeScript* ss,
            SpadeScriptContext* context,
            const string& inst,
            string& result,
            const vector<pair<unsigned int,string>>& blockStarts)
{
    //cout<<"block..."<<endl;
    int r = -999;
    if(cmd.compare("if")==0)
    {
        //cout<<"if..."<<endl;
        // Create a control structure on the context's call stack
        context->stack.push(SpadeScriptStructure("","if"));

        // Evaluate initial expression as condition
        bool initial_cond = evaluate(blockStarts[0].second).compare("0")!=0;
        // bool cond = condition( blockStarts[0].second );

        if(initial_cond)
            r = ss->call(function,context,result,inst,"",blockStarts[0].first+1);
        else for(unsigned int i=1; i<blockStarts.size(); i++) {
            // go through trailing blocks (elseif and else's until end or true condition

            bool current_cond = true;

            if(blockStarts[0].second.length()==0) {
                current_cond = true;
            }
            else current_cond = evaluate(blockStarts[i].second).compare("0")!=0;

            if(current_cond) {
                r = ss->call(function,context,result,inst,"",blockStarts[i].first+1);
                break; // done with block
            }
        }
        
        context->stack.pop();
    } else if(cmd.compare("loop")==0) {
        if(blockStarts[0].second.length()==0) {
            // warning: infinite loop
            while(r!=5 && r!=0) {
                r = ss->call(function,context,result,inst,"",blockStarts[0].first+1);
            }
        }
        else
        {
            int e = atoi(blockStarts[0].second.c_str());
            for(int i=0; i<e && r!=5 && r!=0; i++) {
                // store i as a context variable: _iterator?
                context->stack.top().vars["_i"] = itos(i);
                r = ss->call(function,context,result,inst,"",blockStarts[0].first+1);
            }
                
        }
        r = 5;
    }
    //cout<<"... end block"<<endl;
    return r; // exit from block, notify parent function/block
}

int SpadeScriptEngine :: explode(char delim, const string& list, vector<string>& result, bool append)
{
    istringstream ss(list);
    string element;

    if(!append)
        result.clear();

    while(getline(ss, element, delim)) {
        result.push_back(trim(element," \t"));
    }

    return result.size();
}

string SpadeScriptEngine :: safe_lookup(const vector<string>& v, unsigned int idx, string default_val)
{
    if(idx>=v.size())
        return default_val;
    return v[idx];
}


string SpadeScriptEngine :: trim(const string &s, const string &chars) {

    int last = s.find_last_not_of(chars);
    if (last == string::npos) return "";
    int first = s.find_first_not_of(chars);
    if (first == string::npos) first = 0;
    return s.substr(first, (last-first)+1);
}

bool SpadeScriptEngine :: vectorHas(const vector<string>& v, string element, bool ignoreCase)
{
    if(ignoreCase)
    {
        // transform to lower
        //for(unsigned int i=0 i<element.size(); i++)
        //    element[i] = tolower(element[i]);

        transform(element.begin(), element.end(), element.begin(), ::tolower);
    }

    for(vector<string>::const_iterator itr = v.begin();
        itr != v.end();
        ++itr)
    {
        if(itr->compare(element)==0)
            return true;
    }
    return false;
}

void SpadeScriptEngine :: clear()
{
    // Calls respective clear() method on each SpadeScript object
    for(map<string, SpadeScript*>::iterator itr = m_Scripts.begin();
        itr != m_Scripts.end();
        ++itr)
    {
        itr->second->clear();
    }
    m_Scripts.clear();

    for(map<string, SpadeScript*>::iterator itr = m_Memory.begin();
        itr != m_Memory.end();
        ++itr)
    {
        delete itr->second;
    }
    m_Memory.clear();
}

bool SpadeScriptEngine :: allocate(string fn, string override_name)
{
    // Get name from filename
    string name = "";
    if(override_name.length())
    {
        name = override_name;
    }
    else
    {
        size_t idx = fn.find('.');
        name = fn.substr(0,idx);
    }

    if(m_Scripts.find(name) != m_Scripts.end())
        return true;

    auto_ptr<SpadeScript> ss(new SpadeScript());
    m_Scripts[name] = ss.get();
    m_Memory[name] = ss.release();
    if(m_Memory[name]->load(fn,this,true,override_name))
        return true;
    return false;
}

bool SpadeScriptEngine :: runScript(string script, string inst, string func, string args, SpadeScriptContext* context) 
{
    string result = "";
    context->restart();
    if(m_Scripts.find(script)!=m_Scripts.end())
        getScript(script)->call(func,context,result,inst,args);
    else
        return false;
    return true;
}

bool SpadeScriptEngine :: callFunction(string script, string func)
{
    if(isScript(script)) {
        SpadeScriptContext ssc;
        ssc.start();
        string result = "";
        getScript(script)->call(func,&ssc,result);
        ssc.stop();
        return true;
    }
    return false;
}
