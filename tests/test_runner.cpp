#include "VectorStore.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <functional>
#include <vector> // needed for TEST mode op splitting & optional reference model
#ifdef USE_STD_REF
// <vector> already included
#endif

// Minimal harness for ArrayList using DSL commands from a case file.
// Allowed headers only (no vector, etc.).

struct TestStats { int passed = 0; int failed = 0; int total() const { return passed + failed; } };
struct CaseStats { int passed = 0; int failed = 0; int total() const { return passed + failed; } };
struct AssertionStats { long long passed = 0; long long failed = 0; long long total() const { return passed + failed; } };

static void report(bool cond, const std::string &msg, TestStats &stats) {
    if (cond) { ++stats.passed; } else { ++stats.failed; std::cout << "FAIL: " << msg << "\n"; }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: test_runner <case_file> [output_file]\n";
        return 1;
    }
    std::ifstream fin(argv[1]);
    if (!fin) { std::cout << "Cannot open cases file: " << argv[1] << "\n"; return 1; }
    std::ofstream fout;
    std::ostream* out = &std::cout;
    if (argc >= 3) {
        fout.open(argv[2]);
        if (!fout) { std::cout << "Cannot open output file: " << argv[2] << "\n"; return 1; }
        out = &fout;
    }

    std::map<std::string, ArrayList<int>*> lists; // store named lists
#ifdef USE_STD_REF
    std::map<std::string, std::vector<int>> refLists; // reference model
#endif
    TestStats stats;
    std::string line;
    int lineNo = 0;
    CaseStats testCases; // For single-line TEST mode
    AssertionStats assertionStats; // per-assert inside TEST mode
    bool verbose = false;
    // Detect --verbose flag (3rd or later arg)
    for (int ai = 1; ai < argc; ++ai) {
        if (std::string(argv[ai]) == "--verbose") { verbose = true; }
    }

    auto getList = [&](const std::string &name)->ArrayList<int>& {
        if (!lists.count(name)) throw std::runtime_error("List not found: " + name);
        return *lists[name];
    };

    while (std::getline(fin, line)) {
        ++lineNo;
        if (line.empty()) continue;
        if (line[0] == '#') continue;
        std::istringstream iss(line);
        // Detect single-line TEST mode (isolated array per test)
    if (line.rfind("TEST ",0)==0) {
            // Format: TEST <id> <operations separated by ';'>
            // Operation grammar (tokens separated by space, ops by ';'):
            // CAP <capacity>
            // ADD <value>
            // ADD_AT <index> <value>
            // SET <index> <value>
            // REMOVE_AT <index> = <expected>
            // GET <index> = <expected>
            // SIZE = <expected>
            // EMPTY = true|false
            // INDEX_OF <value> = <expectedIndex>
            // CONTAINS <value> = true|false
            // TO_STRING = <literalList>
            // CLEAR
            // Ex: TEST 1 ADD 1; ADD 2; SIZE = 2; GET 0 = 1; TO_STRING = [1, 2]
            std::istringstream lss(line);
            std::string word; lss >> word; // TEST
            int tid=-1; lss >> tid;
            std::string rest; std::getline(lss, rest);
            if(!rest.empty() && (rest[0]==' '||rest[0]=='\t')) rest.erase(0,1);
            // Split by ';'
            std::vector<std::string> ops; size_t start=0; while(start<rest.size()) { size_t pos = rest.find(';', start); std::string seg = rest.substr(start, pos==std::string::npos? std::string::npos : pos-start); // trim
                size_t b=seg.find_first_not_of(" \t"); if(b==std::string::npos) { seg.clear(); } else { size_t e=seg.find_last_not_of(" \t"); seg=seg.substr(b,e-b+1);} if(!seg.empty()) ops.push_back(seg); if(pos==std::string::npos) break; start=pos+1; }
            ArrayList<int> arr; ArrayList<int>* arr2=nullptr; bool hadCap=false; bool testFail=false; std::string failMsg; auto listStr=[&](){return arr.toString();}; auto listStr2=[&](){ return arr2? arr2->toString(): std::string("<null>"); };
            // For per-assert reporting accumulate failures (but still finish test) unless fatal parse/exception.
            std::vector<std::string> localFailures;
            auto recordAssert = [&](bool ok, const std::string &msg){ if(ok) ++assertionStats.passed; else { ++assertionStats.failed; localFailures.push_back(msg); } };
            for(size_t i=0;i<ops.size();i++) {
                std::istringstream oss(ops[i]); std::string cmdOp; oss >> cmdOp;
                if(cmdOp=="CAP") { int c; if(!(oss>>c)) { testFail=true; failMsg="CAP missing value"; break;} ArrayList<int> tmp(c); arr=tmp; hadCap=true; }
                else if(cmdOp=="CAP2") { int c; if(!(oss>>c)){ testFail=true; failMsg="CAP2 missing value"; break;} if(arr2){ delete arr2; arr2=nullptr;} arr2=new ArrayList<int>(c); }
                else if(cmdOp=="COPY_NEW") { if(arr2){ delete arr2; arr2=nullptr;} try { arr2 = new ArrayList<int>(arr); } catch(const std::exception &ex){ testFail=true; failMsg=std::string("COPY_NEW exception: ")+ex.what(); break; } }
                else if(cmdOp=="ASSIGN_TO2") { if(!arr2){ testFail=true; failMsg="ASSIGN_TO2 with no arr2"; break;} try { *arr2 = arr; } catch(const std::exception &ex){ testFail=true; failMsg=std::string("ASSIGN_TO2 exception: ")+ex.what(); break; } }
                else if(cmdOp=="ASSIGN_FROM2") { if(!arr2){ testFail=true; failMsg="ASSIGN_FROM2 with no arr2"; break;} try { arr = *arr2; } catch(const std::exception &ex){ testFail=true; failMsg=std::string("ASSIGN_FROM2 exception: ")+ex.what(); break; } }
                else if(cmdOp=="SELF_ASSIGN") { try { arr = arr; } catch(const std::exception &ex){ testFail=true; failMsg=std::string("SELF_ASSIGN exception: ")+ex.what(); break; } }
                else if(cmdOp=="ADD") { int v; if(!(oss>>v)){ testFail=true; failMsg="ADD missing value"; break;} arr.add(v); }
                else if(cmdOp=="ADD_AT") { int idx,v; if(!(oss>>idx>>v)){ testFail=true; failMsg="ADD_AT missing args"; break;} bool threw=false; try{ arr.add(idx,v);}catch(const std::exception &ex){ threw=true; recordAssert(false,std::string("ADD_AT exception: ")+ex.what()); } if(threw) continue; }
                else if(cmdOp=="SET") { int idx,v; if(!(oss>>idx>>v)){ testFail=true; failMsg="SET missing args"; break;} try{ arr.set(idx,v);}catch(const std::exception &ex){ testFail=true; failMsg=std::string("SET exception: ")+ex.what(); break; } }
                else if(cmdOp=="SET2") { int idx,v; if(!(oss>>idx>>v)){ testFail=true; failMsg="SET2 missing args"; break;} if(!arr2){ testFail=true; failMsg="SET2 no arr2"; break;} try{ arr2->set(idx,v);}catch(const std::exception &ex){ testFail=true; failMsg=std::string("SET2 exception: ")+ex.what(); break; } }
                else if(cmdOp=="REMOVE_AT") { int idx; if(!(oss>>idx)){ testFail=true; failMsg="REMOVE_AT missing index"; break;} std::string eq; oss>>eq; int exp; if(eq!="="||!(oss>>exp)){ testFail=true; failMsg="REMOVE_AT expected '= <val>'"; break;} int got; try { got=arr.removeAt(idx);} catch(const std::exception &ex){ recordAssert(false,std::string("REMOVE_AT exception: ")+ex.what()); continue;} recordAssert(got==exp, "REMOVE_AT got="+std::to_string(got)+" exp="+std::to_string(exp)); }
                else if(cmdOp=="REMOVE_AT2") { int idx; if(!(oss>>idx)){ testFail=true; failMsg="REMOVE_AT2 missing index"; break;} if(!arr2){ testFail=true; failMsg="REMOVE_AT2 no arr2"; break;} std::string eq; oss>>eq; int exp; if(eq!="="||!(oss>>exp)){ testFail=true; failMsg="REMOVE_AT2 expected '= <val>'"; break;} int got; try { got=arr2->removeAt(idx);} catch(const std::exception &ex){ recordAssert(false,std::string("REMOVE_AT2 exception: ")+ex.what()); continue;} recordAssert(got==exp, "REMOVE_AT2 got="+std::to_string(got)+" exp="+std::to_string(exp)); }
                else if(cmdOp=="GET") { int idx; if(!(oss>>idx)){ testFail=true; failMsg="GET missing index"; break;} std::string eq; int exp; oss>>eq>>exp; if(eq!="="){ testFail=true; failMsg="GET expected '= <val>'"; break;} int got; try { got=arr.get(idx);} catch(const std::exception &ex){ recordAssert(false,std::string("GET exception: ")+ex.what()); continue;} recordAssert(got==exp, "GET got="+std::to_string(got)+" exp="+std::to_string(exp)); }
                else if(cmdOp=="GET2") { int idx; if(!(oss>>idx)){ testFail=true; failMsg="GET2 missing index"; break;} if(!arr2){ testFail=true; failMsg="GET2 no arr2"; break;} std::string eq; int exp; oss>>eq>>exp; if(eq!="="){ testFail=true; failMsg="GET2 expected '= <val>'"; break;} int got; try { got=arr2->get(idx);} catch(const std::exception &ex){ recordAssert(false,std::string("GET2 exception: ")+ex.what()); continue;} recordAssert(got==exp, "GET2 got="+std::to_string(got)+" exp="+std::to_string(exp)); }
                else if(cmdOp=="SIZE") { std::string eq; int exp; if(!(oss>>eq>>exp)||eq!="="){ testFail=true; failMsg="SIZE expected '= <val>'"; break;} recordAssert(arr.size()==exp, "SIZE got="+std::to_string(arr.size())+" exp="+std::to_string(exp)); }
                else if(cmdOp=="SIZE2") { if(!arr2){ testFail=true; failMsg="SIZE2 no arr2"; break;} std::string eq; int exp; if(!(oss>>eq>>exp)||eq!="="){ testFail=true; failMsg="SIZE2 expected '= <val>'"; break;} recordAssert(arr2->size()==exp, "SIZE2 got="+std::to_string(arr2->size())+" exp="+std::to_string(exp)); }
                else if(cmdOp=="EMPTY") { std::string eq,val; if(!(oss>>eq>>val)||eq!="="){ testFail=true; failMsg="EMPTY expected '= <bool>'"; break;} bool exp=(val=="true"); recordAssert(arr.empty()==exp, std::string("EMPTY got=")+(arr.empty()?"true":"false")+" exp="+val); }
                else if(cmdOp=="INDEX_OF") { int v; if(!(oss>>v)){ testFail=true; failMsg="INDEX_OF missing value"; break;} std::string eq; int exp; if(!(oss>>eq>>exp)||eq!="="){ testFail=true; failMsg="INDEX_OF expected '= <val>'"; break;} int got=arr.indexOf(v); recordAssert(got==exp, "INDEX_OF got="+std::to_string(got)+" exp="+std::to_string(exp)); }
                else if(cmdOp=="CONTAINS") { int v; if(!(oss>>v)){ testFail=true; failMsg="CONTAINS missing value"; break;} std::string eq,val; if(!(oss>>eq>>val)||eq!="="){ testFail=true; failMsg="CONTAINS expected '= <bool>'"; break;} bool exp=(val=="true"); bool got=arr.contains(v); recordAssert(got==exp, std::string("CONTAINS got=")+(got?"true":"false")+" exp="+val); }
                else if(cmdOp=="TO_STRING") { std::string eq; if(!(oss>>eq)||eq!="="){ testFail=true; failMsg="TO_STRING expected '= <literal>'"; break;} std::string expect; std::getline(oss, expect); if(!expect.empty()&&expect[0]==' ') expect.erase(0,1); recordAssert(listStr()==expect, "TO_STRING got="+listStr()+" exp="+expect); }
                else if(cmdOp=="TO_STRING2") { if(!arr2){ testFail=true; failMsg="TO_STRING2 no arr2"; break;} std::string eq; if(!(oss>>eq)||eq!="="){ testFail=true; failMsg="TO_STRING2 expected '= <literal>'"; break;} std::string expect; std::getline(oss, expect); if(!expect.empty()&&expect[0]==' ') expect.erase(0,1); recordAssert(listStr2()==expect, "TO_STRING2 got="+listStr2()+" exp="+expect); }
                else if(cmdOp=="CLEAR") { arr.clear(); }
                else if(cmdOp=="CLEAR2") { if(!arr2){ testFail=true; failMsg="CLEAR2 no arr2"; break;} arr2->clear(); }
                else if(cmdOp=="ITER_SUM") { // ITER_SUM = <expectedSum>
                    std::string eq; int exp; if(!(oss>>eq>>exp) || eq!="="){ testFail=true; failMsg="ITER_SUM expected '= <val>'"; break;} long long sum=0; try { for(auto it=arr.begin(); it!=arr.end(); ++it) sum += *it; } catch(const std::exception &ex){ recordAssert(false, std::string("ITER_SUM exception: ")+ex.what()); continue; } recordAssert(sum==exp, "ITER_SUM got="+std::to_string(sum)+" exp="+std::to_string(exp)); }
                else if(cmdOp=="ITER_SEQ") { // ITER_SEQ = [a, b, c]
                    std::string eq; if(!(oss>>eq) || eq!="="){ testFail=true; failMsg="ITER_SEQ expected '= [..]'"; break;} std::string expect; std::getline(oss, expect); if(!expect.empty()&&expect[0]==' ') expect.erase(0,1); // reuse toString format
                    std::string got="["; bool first=true; try { for(auto it=arr.begin(); it!=arr.end(); ++it){ if(!first) got += ", "; first=false; got += std::to_string(*it);} } catch(const std::exception &ex){ recordAssert(false, std::string("ITER_SEQ exception: ")+ex.what()); continue; } got += "]"; recordAssert(got==expect, "ITER_SEQ got="+got+" exp="+expect); }
                else if(cmdOp=="IT_AT") { // IT_AT <index> = <expectedValue>
                    int idx; if(!(oss>>idx)){ testFail=true; failMsg="IT_AT missing index"; break;} std::string eq; int exp; if(!(oss>>eq>>exp) || eq!="="){ testFail=true; failMsg="IT_AT expected '= <val>'"; break;} try { auto it=arr.begin(); for(int i=0;i<idx;++i) ++it; int v=*it; recordAssert(v==exp, "IT_AT got="+std::to_string(v)+" exp="+std::to_string(exp)); } catch(const std::exception &ex){ recordAssert(false, std::string("IT_AT exception: ")+ex.what()); }
                }
                else if(cmdOp=="IT_PRE_INC") { // IT_PRE_INC <steps> = <finalValue>
                    int steps; if(!(oss>>steps)){ testFail=true; failMsg="IT_PRE_INC missing steps"; break;} std::string eq; int exp; if(!(oss>>eq>>exp) || eq!="="){ testFail=true; failMsg="IT_PRE_INC expected '= <val>'"; break;} try { auto it=arr.begin(); for(int i=0;i<steps;++i) ++it; int v=*it; recordAssert(v==exp, "IT_PRE_INC got="+std::to_string(v)+" exp="+std::to_string(exp)); } catch(const std::exception &ex){ recordAssert(false,std::string("IT_PRE_INC exception: ")+ex.what()); }
                }
                else if(cmdOp=="IT_POST_INC") { // IT_POST_INC <steps> = <finalValue>
                    int steps; if(!(oss>>steps)){ testFail=true; failMsg="IT_POST_INC missing steps"; break;} std::string eq; int exp; if(!(oss>>eq>>exp)||eq!="="){ testFail=true; failMsg="IT_POST_INC expected '= <val>'"; break;} try { auto it=arr.begin(); for(int i=0;i<steps;++i) it++; int v=*it; recordAssert(v==exp, "IT_POST_INC got="+std::to_string(v)+" exp="+std::to_string(exp)); } catch(const std::exception &ex){ recordAssert(false,std::string("IT_POST_INC exception: ")+ex.what()); }
                }
                else if(cmdOp=="IT_PRE_DEC") { // IT_PRE_DEC <startIndex> <decSteps> = <finalValue>
                    int startIdx, decSteps; if(!(oss>>startIdx>>decSteps)){ testFail=true; failMsg="IT_PRE_DEC missing args"; break;} std::string eq; int exp; if(!(oss>>eq>>exp)||eq!="="){ testFail=true; failMsg="IT_PRE_DEC expected '= <val>'"; break;} try { auto it=arr.begin(); for(int i=0;i<startIdx;++i) ++it; for(int d=0; d<decSteps; ++d) --it; int v=*it; recordAssert(v==exp, "IT_PRE_DEC got="+std::to_string(v)+" exp="+std::to_string(exp)); } catch(const std::exception &ex){ recordAssert(false,std::string("IT_PRE_DEC exception: ")+ex.what()); }
                }
                else if(cmdOp=="IT_POST_DEC") { // IT_POST_DEC <startIndex> <decSteps> = <finalValue>
                    int startIdx, decSteps; if(!(oss>>startIdx>>decSteps)){ testFail=true; failMsg="IT_POST_DEC missing args"; break;} std::string eq; int exp; if(!(oss>>eq>>exp)||eq!="="){ testFail=true; failMsg="IT_POST_DEC expected '= <val>'"; break;} try { auto it=arr.begin(); for(int i=0;i<startIdx;++i) ++it; for(int d=0; d<decSteps; ++d) it--; int v=*it; recordAssert(v==exp, "IT_POST_DEC got="+std::to_string(v)+" exp="+std::to_string(exp)); } catch(const std::exception &ex){ recordAssert(false,std::string("IT_POST_DEC exception: ")+ex.what()); }
                }
                else if(cmdOp=="THROW_IT_DEREF") { // expect throw dereferencing invalid position
                    int idx; if(!(oss>>idx)){ testFail=true; failMsg="THROW_IT_DEREF missing index"; break;} bool threw=false; try { auto it=arr.begin(); for(int i=0;i<idx;++i) ++it; (void)*it; } catch(...){ threw=true; } recordAssert(threw, "THROW_IT_DEREF expected exception idx="+std::to_string(idx)); }
                else if(cmdOp=="THROW_IT_INC") { // increment past end
                    bool threw=false; try { auto it=arr.end(); ++it; } catch(...){ threw=true; } recordAssert(threw, "THROW_IT_INC expected exception"); }
                else if(cmdOp=="THROW_IT_DEC") { // decrement before begin
                    bool threw=false; try { auto it=arr.begin(); --it; } catch(...){ threw=true; } recordAssert(threw, "THROW_IT_DEC expected exception"); }
                else if(cmdOp=="THROW_GET") { int idx; if(!(oss>>idx)){ testFail=true; failMsg="THROW_GET missing index"; break;} bool threw=false; try{ (void)arr.get(idx);}catch(...){ threw=true;} recordAssert(threw, "THROW_GET expected exception idx="+std::to_string(idx)); }
                else if(cmdOp=="THROW_SET") { int idx,v; if(!(oss>>idx>>v)){ testFail=true; failMsg="THROW_SET missing args"; break;} bool threw=false; try{ arr.set(idx,v);}catch(...){ threw=true;} recordAssert(threw, "THROW_SET expected exception idx="+std::to_string(idx)); }
                else if(cmdOp=="THROW_REMOVE") { int idx; if(!(oss>>idx)){ testFail=true; failMsg="THROW_REMOVE missing index"; break;} bool threw=false; try{ (void)arr.removeAt(idx);}catch(...){ threw=true;} recordAssert(threw, "THROW_REMOVE expected exception idx="+std::to_string(idx)); }
                else if(cmdOp=="THROW_ADD_AT") { int idx,v; if(!(oss>>idx>>v)){ testFail=true; failMsg="THROW_ADD_AT missing args"; break;} bool threw=false; try{ arr.add(idx,v);}catch(...){ threw=true;} recordAssert(threw, "THROW_ADD_AT expected exception idx="+std::to_string(idx)); }
                else { testFail=true; failMsg="Unknown op '"+cmdOp+"'"; }
            }
            if(arr2){ delete arr2; arr2=nullptr; }
            bool assertionsFailed = !localFailures.empty();
            if(testFail){ ++testCases.failed; *out << "TEST "<<tid<<" PARSE/EXEC FAIL: "<<failMsg<<" state="<<listStr()<<" line="<<lineNo<<"\n"; }
            else if(assertionsFailed) { ++testCases.failed; for(auto &m: localFailures) *out << "TEST "<<tid<<" ASSERT FAIL: "<<m<<" final="<<listStr()<<"\n"; }
            else { ++testCases.passed; if (verbose) *out << "TEST "<<tid<<" PASS final="<<listStr()<<" asserts="<<assertionStats.passed<<"/"<<assertionStats.total()<<"\n"; }
            continue; // proceed to next line
        }

        // Optional leading case number (old batch mode)
        std::string first; iss >> first;
        std::string cmd;
        int caseId = -1;
        if (!first.empty() && std::isdigit(static_cast<unsigned char>(first[0]))) {
            // treat as case number
            caseId = std::stoi(first);
            iss >> cmd;
        } else {
            cmd = first; // first token is command
        }
        if (cmd.empty()) continue;

        try {
            if (cmd == "NEW") {
                std::string name; int cap; iss >> name >> cap; if (!iss) cap = 10;
                if (lists.count(name)) { delete lists[name]; }
                lists[name] = new ArrayList<int>(cap);
#ifdef USE_STD_REF
                refLists[name].clear();
                refLists[name].shrink_to_fit();
#endif
            } else if (cmd == "ADD") {
                std::string name; int v; iss >> name >> v; getList(name).add(v);
#ifdef USE_STD_REF
                refLists[name].push_back(v);
#endif
            } else if (cmd == "RANGE_ADD") { // RANGE_ADD name start endInclusive
                std::string name; int s,e; iss >> name >> s >> e; for (int x=s; x<=e; ++x) getList(name).add(x);
#ifdef USE_STD_REF
                for (int x=s; x<=e; ++x) refLists[name].push_back(x);
#endif
            } else if (cmd == "ADD_AT") {
                std::string name; int idx,v; iss >> name >> idx >> v; getList(name).add(idx,v);
#ifdef USE_STD_REF
                if (idx >=0 && idx <= (int)refLists[name].size()) refLists[name].insert(refLists[name].begin()+idx,v);
#endif
            } else if (cmd == "SET") {
                std::string name; int idx,v; iss >> name >> idx >> v; getList(name).set(idx,v);
#ifdef USE_STD_REF
                if (idx >=0 && idx < (int)refLists[name].size()) refLists[name][idx]=v;
#endif
            } else if (cmd == "GET") {
                std::string name; int idx,exp; iss >> name >> idx >> exp; int got = getList(name).get(idx); report(got==exp, (caseId!=-1?"[#"+std::to_string(caseId)+"] ":"")+"GET mismatch line " + std::to_string(lineNo), stats);
            } else if (cmd == "REMOVE_AT") {
                std::string name; int idx,exp; iss >> name >> idx >> exp; int got = getList(name).removeAt(idx); report(got==exp, (caseId!=-1?"[#"+std::to_string(caseId)+"] ":"")+"REMOVE_AT mismatch line " + std::to_string(lineNo), stats);
#ifdef USE_STD_REF
                if (idx >=0 && idx < (int)refLists[name].size()) refLists[name].erase(refLists[name].begin()+idx);
#endif
            } else if (cmd == "SIZE") {
                std::string name; int exp; iss >> name >> exp; report(getList(name).size()==exp, (caseId!=-1?"[#"+std::to_string(caseId)+"] ":"")+"SIZE mismatch line " + std::to_string(lineNo), stats);
            } else if (cmd == "EMPTY") {
                std::string name; std::string exp; iss >> name >> exp; bool b = (exp=="true"); report(getList(name).empty()==b, (caseId!=-1?"[#"+std::to_string(caseId)+"] ":"")+"EMPTY mismatch line " + std::to_string(lineNo), stats);
            } else if (cmd == "INDEX_OF") {
                std::string name; int v,exp; iss >> name >> v >> exp; report(getList(name).indexOf(v)==exp, (caseId!=-1?"[#"+std::to_string(caseId)+"] ":"")+"INDEX_OF mismatch line " + std::to_string(lineNo), stats);
            } else if (cmd == "CONTAINS") {
                std::string name; int v; std::string exp; iss >> name >> v >> exp; bool b=(exp=="true"); report(getList(name).contains(v)==b, (caseId!=-1?"[#"+std::to_string(caseId)+"] ":"")+"CONTAINS mismatch line " + std::to_string(lineNo), stats);
            } else if (cmd == "CLEAR") {
                std::string name; iss >> name; getList(name).clear();
#ifdef USE_STD_REF
                refLists[name].clear();
#endif
            } else if (cmd == "ITER_SUM") {
                std::string name; int exp; iss >> name >> exp; int sum=0; for (auto it=getList(name).begin(); it != getList(name).end(); ++it) sum += *it; report(sum==exp, (caseId!=-1?"[#"+std::to_string(caseId)+"] ":"")+"ITER_SUM mismatch line " + std::to_string(lineNo), stats);
            } else if (cmd == "TO_STRING") {
                std::string name; std::string rest; iss >> name; std::getline(iss, rest); // rest starts with space then expected literal
                if (!rest.empty() && rest[0]==' ') rest.erase(0,1);
                std::string got = getList(name).toString();
                report(got==rest, (caseId!=-1?"[#"+std::to_string(caseId)+"] ":"")+"TO_STRING mismatch line " + std::to_string(lineNo) + " got=" + got + " expected=" + rest, stats);
            } else if (cmd == "ASSIGN") { // ASSIGN dst src (supports self)
                std::string dst, src; iss >> dst >> src; ArrayList<int> &d = getList(dst); ArrayList<int> &s = getList(src); d = s; report(d.size()==s.size(), "ASSIGN size mismatch line " + std::to_string(lineNo), stats);
#ifdef USE_STD_REF
                if (refLists.count(src)) refLists[dst] = refLists[src];
#endif
            } else if (cmd == "EXPECT_THROW") {
                std::string sub; iss >> sub;
                bool thrown = false;
                try {
                    if (sub == "ADD_AT") { std::string name; int idx,v; iss >> name >> idx >> v; getList(name).add(idx,v); }
                    else if (sub == "GET") { std::string name; int idx; iss >> name >> idx; getList(name).get(idx); }
                    else if (sub == "SET") { std::string name; int idx,v; iss >> name >> idx >> v; getList(name).set(idx,v); }
                    else if (sub == "REMOVE_AT") { std::string name; int idx; iss >> name >> idx; getList(name).removeAt(idx); }
                    else { throw std::runtime_error("Unknown EXPECT_THROW op '"+sub+"'"); }
                } catch (const std::out_of_range&) { thrown = true; }
                report(thrown, (caseId!=-1?"[#"+std::to_string(caseId)+"] ":"")+"EXPECT_THROW failed line " + std::to_string(lineNo), stats);
            } else if (cmd == "ITER_COMPARE") { // ITER_COMPARE name
                std::string name; iss >> name; int sumIter=0; int sumIdx=0; auto &lst = getList(name); for (auto it=lst.begin(); it!=lst.end(); ++it) sumIter += *it; for (int i=0;i<lst.size();++i) sumIdx += lst.get(i); report(sumIter==sumIdx, (caseId!=-1?"[#"+std::to_string(caseId)+"] ":"")+"ITER_COMPARE mismatch line " + std::to_string(lineNo), stats);
            } else if (cmd == "GROW") { // GROW name targetCount startVal
                std::string name; int target,start; iss >> name >> target >> start; auto &lst = getList(name); for (int v=start; lst.size()<target; ++v) lst.add(v);
                report(lst.size()==target, (caseId!=-1?"[#"+std::to_string(caseId)+"] ":"")+"GROW size mismatch line " + std::to_string(lineNo), stats);
            } else if (cmd == "CLEAR_REFILL") { // CLEAR_REFILL name cycles batch
                std::string name; int cycles,batch; iss >> name >> cycles >> batch; auto &lst = getList(name); bool ok=true; for (int c=0;c<cycles && ok;++c){ lst.clear(); if(!lst.empty()) ok=false; for(int i=0;i<batch;++i) lst.add(i); if(lst.size()!=batch) ok=false;} report(ok,(caseId!=-1?"[#"+std::to_string(caseId)+"] ":"")+"CLEAR_REFILL failure line "+std::to_string(lineNo),stats);
            } else if (cmd == "EXPECT_THROW_INDEX") { // EXPECT_THROW_INDEX op name largeIndex
                std::string op,name; long long idx; iss >> op >> name >> idx; bool thrown=false; try { if (op=="GET") getList(name).get((int)idx); else if (op=="SET") { getList(name).set((int)idx,0);} else if (op=="REMOVE_AT") { getList(name).removeAt((int)idx);} else if (op=="ADD_AT") { getList(name).add((int)idx,0);} else throw std::runtime_error("Bad op"); } catch (const std::out_of_range&) { thrown=true;} report(thrown,(caseId!=-1?"[#"+std::to_string(caseId)+"] ":"")+"EXPECT_THROW_INDEX failed line "+std::to_string(lineNo),stats);
            } else if (cmd == "STRESS") { // STRESS name ops seed (conditional RNG operations)
                std::string name; int ops; unsigned seed; iss >> name >> ops >> seed; auto &lst = getList(name); unsigned long long state = seed ? seed : 1ULL; auto rnd=[&](){ state = (state*6364136223846793005ULL + 1ULL); return (unsigned)(state>>32); }; bool ok=true; for(int i=0;i<ops && ok;++i){ unsigned r=rnd()%6; int sz=lst.size(); if(r==0){ lst.add((int)(rnd()%1000)); } else if(r==1 && sz>0){ int pos=(int)(rnd()% (sz+1)); if(pos>sz) pos=sz; lst.add(pos,(int)(rnd()%1000)); } else if(r==2 && sz>0){ int pos=(int)(rnd()%sz); lst.removeAt(pos);} else if(r==3 && sz>0){ int pos=(int)(rnd()%sz); lst.set(pos,(int)(rnd()%5000)); } else if(r==4){ if(rnd()%50==0) lst.clear(); } else if(r==5){ // iterate sum as light verification
                        int s=0; for(auto it=lst.begin(); it!=lst.end(); ++it) s += *it; (void)s; }
                }
                report(ok, (caseId!=-1?"[#"+std::to_string(caseId)+"] ":"")+"STRESS run failed line "+std::to_string(lineNo), stats);
            } else {
                *out << "Unknown command '" << cmd << "' at line " << lineNo << "\n";
                ++stats.failed;
            }
        } catch (const std::exception &ex) {
            *out << "EXCEPTION line " << lineNo << ": " << ex.what() << " (command: " << cmd << ")\n";
            ++stats.failed;
        }
    }

    // cleanup
    for (auto &p : lists) delete p.second;

    *out << "\nBATCH_SUMMARY: passed=" << stats.passed << " failed=" << stats.failed << " total=" << stats.total() << "\n";
    *out << "TEST_SUMMARY: passed=" << testCases.passed << " failed=" << testCases.failed << " total=" << testCases.total() << "\n";
    *out << "ASSERT_SUMMARY: passed=" << assertionStats.passed << " failed=" << assertionStats.failed << " total=" << assertionStats.total() << "\n";
    return (stats.failed==0)?0:1;
}
