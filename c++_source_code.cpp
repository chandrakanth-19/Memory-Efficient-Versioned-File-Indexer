#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <stdexcept>

using namespace std;


// function that prints all key value pairs from unordered map. Used to display word frequency mappings (word -> count)
template<typename K, typename V>

void printm(const unordered_map<K,V> &m){
    for (auto &p : m){ 
        cout<< p.first << " -> " <<p.second << endl;
    }
}


// template function that prints the first K elements from a vector of pairs. This is used to display the top K most frequent words after sorting

template<typename K, typename V>
void printm(const vector<pair<K,V>> &v, int k){
    int count = 0;
    for(auto &p : v){

        if(count >= k) break;
        cout <<p.first << " -> " <<p.second << endl;
        count++;
    }
}

// class for reading a file incrementally using fixed size buffer, this ensures that the program does not load the entire file into memory

class bufferread{

private:
    ifstream file;
    size_t buf_size;
    vector<char> buf;

public:

    
    // constructor that opens the file and initializes the buffer with the given size, throws an exception if the file cannot be opened
    
    bufferread(const string &path, size_t size)
        : buf_size(size), buf(size){
        file.open(path);
        if(!file) throw runtime_error("Unable to open file: " + path);
    }

    
    // reads the next chunk of the file into the buffer and returns false when the file has no more data, the read data is copied into the output string
    
    bool read_chnk(string &out){

        if(!file.good()) return false;

        file.read(buf.data(), buf_size);
        size_t byte_read = file.gcount();

        if(byte_read == 0) return false;

        out.assign(buf.data(), byte_read);
        return true;
    }
};


// Tokenizer class for extracting words from text chunks  where words are defined as contiguous alphanumeric characters and are converted to lowercase for case insensitive indexing

class Tokenizer{
public:

    
   //  helper function that checks whether a character is a valid word character (alphanumeric)
    
    static bool is_wchar(char c){
        return isalnum(static_cast<unsigned char>(c));
    }

    
   // splits the input data chunk into words, uses the carry string to handle partial words that may be split across buffer boundaries
    
    vector<string> tokenize(const string &data, string &carry){

        vector<string> tokens;
        string word = carry;

        for(char c : data){
            if(is_wchar(c)) word += tolower(c);
            else {
                if(!word.empty()){
                    tokens.push_back(word);
                    word.clear();
                }
            }
        }

        carry = word;
        return tokens;
    }
};


// class that maintains the word index for multiple versions of files, each version has its own map of word frequencies

class versionedindex{
private:
    unordered_map<string, unordered_map<string,int>> versns;

public:

    
     // adds a word occurrence to a specific versions index if the word already exists, its count is incremented
    
    void add_word(const string &version, const string &word){
        versns[version][word]++;
    }

    
    // returns the frequency of a given word in a specific version if the version or word does not exist, returns 0
    
    int get_wcount(const string &version, const string &word){

        auto vit = versns.find(version);
        if(vit == versns.end()) return 0;
        auto wit = vit->second.find(word);
        if(wit == vit->second.end()) return 0;
        return wit->second;
    }

    
    // computes the difference in frequency of a word between two versions of indexed files
    
    int diff(const string &v1, const string &v2, const string &word){
        return get_wcount(v2, word)-get_wcount(v1, word);
    }

    
   // returns the top K most frequent words for a given version, the words are sorted in descending order of frequency
    
    vector<pair<string,int>> topK(const string &version, int k){

        auto vit = versns.find(version);
        if(vit == versns.end()) return {};

        vector<pair<string,int>> vec(vit->second.begin(),vit->second.end());

        sort(vec.begin(), vec.end(), [](auto &a, auto &b) { return a.second > b.second; });
        if((int)vec.size() > k) vec.resize(k);

        return vec;
    }
};


// abstract base class representing a generic query, derived classes implement specific query types

class query{

public:
    virtual void execute() = 0;
    virtual ~query(){}
};


// derived query class that returns the frequency of a specific word in a given version

class wordquery : public query{

private:
    versionedindex &idx;
    string version;
    string word;

public:
    wordquery(versionedindex &i,string v,string w)
        : idx(i),version(v),word(w){}

    
    // executes the word count query and prints the result
    
    void execute() override{

        cout<<"Version: "<< version<<endl;
        cout<<"Word: "<<word<< endl;
        cout<<"Count: " << idx.get_wcount(version,word)<<endl;
    }
};


// derived query class that computes the difference in word frequency between two indexed versions

class diffquery : public query {

private:
    versionedindex &idx;
    string v1,v2,word;

public:
    diffquery(versionedindex &i,string a,string b,string w)
        : idx(i),v1(a),v2(b),word(w){}

    
    // executes the difference query and prints frequency comparison between two versions
    
    void execute() override {

        cout <<"Word: "<<word<<endl;
        cout <<"Version1 ("<<v1<<"): "<< idx.get_wcount(v1,word)<< endl;
        cout <<"Version2 ("<<v2<<"): "<< idx.get_wcount(v2,word)<< endl;
        cout <<"Difference: " << idx.diff(v1,v2,word) <<endl;
    }
};


// derived query class that displays the top K most frequent words in a specified version

class topquery : public query{

private:
    versionedindex &idx;
    string version;
    int k;

public:
    topquery(versionedindex &i,string v,int t)
        : idx(i),version(v),k(t){}

    
   //  executes the topK query and prints the highest frequency words
    
    void execute() override{

        cout<<"Version: "<< version<<endl;
        cout<<"Top "<< k << " words:"<<endl;

        auto result = idx.topK(version,k);
        for(auto &p : result) cout<<p.first<<" -> "<<p.second<<endl;
    }
};


// function that builds the word index for a file version, reads the file using buffered chunks, tokenizes the text, and stores word frequencies in the index

void build_idx( const string& file, const string& version, int bufKB, versionedindex &idx){

    if(bufKB < 256 || bufKB > 1024) throw invalid_argument("buffer size must be between 256 and 1024 KB");

    size_t buf_size = bufKB * 1024;
    bufferread reader(file,buf_size);
    Tokenizer tokenizer;

    string carry;
    string chunk;

    while(reader.read_chnk(chunk)){
        auto tokens = tokenizer.tokenize(chunk,carry);
        for(auto &w : tokens) idx.add_word(version,w);
    }

    if(!carry.empty()) idx.add_word(version,carry);
}


// Main function
// parses command line arguments
// builds indexes for files
// executes the requested query
// measures and prints execution time

int main(int argc,char* argv[]){
    try{
        auto start = chrono::high_resolution_clock::now();

        string file,version,qtype,word;
        string file1,file2,version1,version2;

        int topk=0;
        int buf=512;

        for(int i=1;i<argc;i++){

            if(strcmp(argv[i],"--file") == 0) file=argv[++i];
            else if(strcmp(argv[i],"--file1") ==0) file1=argv[++i];
            else if(strcmp(argv[i],"--file2") == 0) file2=argv[++i];
            else if(strcmp(argv[i],"--version")== 0) version=argv[++i];
            else if(strcmp(argv[i],"--version1")== 0) version1=argv[++i];
            else if(strcmp(argv[i],"--version2") == 0) version2=argv[++i];
            else if(strcmp(argv[i],"--query") ==0) qtype=argv[++i];
            else if(strcmp(argv[i],"--word") == 0) word=argv[++i];
            else if(strcmp(argv[i],"--top") == 0) topk=stoi(argv[++i]);
            else if(strcmp(argv[i],"--buffer")== 0) buf=stoi(argv[++i]);

        }

        if (!word.empty()) {
            for (char &c : word) {
                c = tolower(c);
            }
        }

        versionedindex idx;

        if(qtype == "diff"){
            if(file1.empty() || file2.empty() || version1.empty() || version2.empty())
                throw invalid_argument("Diff query needs --file1 --file2 --version1 --version2");

            build_idx(file1, version1,buf ,idx);
            build_idx(file2,version2, buf,idx);
        }
        else{
            if(file.empty() || version.empty()) throw invalid_argument("query needs --file and --version");

            build_idx(file,version,buf,idx);
        }

        query* q=nullptr;

        if(qtype=="word")    q=new wordquery(idx,version,word);
        else if(qtype=="top") q=new topquery(idx,version,topk);
        else if(qtype=="diff")   q=new diffquery(idx,version1,version2,word);
        else throw invalid_argument("Unknown query type: " + qtype);

        if(q){
            q->execute();
            delete q;
        }

        auto end = chrono::high_resolution_clock::now();
        double time =chrono::duration<double>(end-start).count();

        cout << "buffer : "<< buf<<" KB"<<endl;
        cout << "Execution time: "<< time<<" sec"<<endl;

    }
    catch(exception &e){
        cerr <<"Error: "<< e.what() << endl;
    }

    return 0;
}
