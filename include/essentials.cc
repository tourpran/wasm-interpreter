#include <iostream> 
#include <vector> 
#include <fstream> 
#include <cstring>
#include <iomanip>

#define ull unsigned long long

namespace wappMainArena{
    using std::string;
    using std::cout;
    using std::vector;

    enum valuesToken{
        Integer,
        FloatingPoint,
        Strings,
        Names,      // Assuming only printable characters are names. 
        Identifiers
    };

    class wapp{
    public:
        wapp(int argc, char** argv) : astPrint(false), bytecodePrint(false), tokenPrint(false) {
            if(argc < 2){
                error("Usage: wapp filename -flag");
                /* TODO: Implement a shell feature. */
                error("Shell feature yet to be implemented.");
            }
            if(argc >= 3 && argv[2][0] == '-'){
                if(!strcmp(argv[2], "-bytecode")){
                    bytecodePrint = true;  
                }
                else if(!strcmp(argv[2], "-ast")){
                    astPrint = true;  
                }
                else if(!strcmp(argv[2], "-tokens")){
                    tokenPrint = true;  
                }
            }

            /* Read the file and put it entirely in memory for faster read writes. */
            std::ifstream fileStream(argv[1]); 
            string rawFileContents((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>()); 

            /* Start the parsing process for the given WAT here:
                - tokenize  - get all the tokens separated with their proper representation.
                - AST       - Create a Abstract Syntac Tree accordingly.
                - ByteCode  - Generate the bytecode from the AST and Send it to the byteCodeHandler.
            */
            vector<std::pair<string, int>> watTokenizedStrings = tokenize(rawFileContents);
            if(tokenPrint){
                cout << "Tokens: \n";
                for(auto iter: watTokenizedStrings){
                    cout << "[-] " << iter.first << " => " << iter.second << std::endl;
                }
            }
        }

        /* Tokenisation of the Given Raw String */
        vector<std::pair<string, int>> tokenize(string rawString){
            vector<std::pair<string, int>> tokens;
            bool openSingleQuote = false;
            bool openDoubleQuote = false;

            string tmp = "";
            rawString = removeCommentsInRawString(rawString); 
            ull rawStringLen = rawString.length();
            for(ull it=0;it<rawStringLen;it++){
                if(!openDoubleQuote && !openSingleQuote && isTokenDelimiter(rawString[it])){
                    if(tmp.length()!=0){
                        tokens.push_back(std::make_pair(tmp, lexicalAnalysis(tmp)));
                        tmp = "";
                    }
                }
                /* TODO: Bug here when intermixed with hexadecimal bytes codes in strings. Format \u where u are hex digits. */
                else if(rawString[it] == '\\'){
                    tmp += rawString[it+1];
                    it++;
                }
                /* TODO: In lexical Analysis stage convert the hexadecimal bytes in the form \u, Right now it would be written as normal string literal. */
                else if(isTokenSingleQuote(rawString[it])){
                    if(openSingleQuote){
                        tokens.push_back(std::make_pair(tmp, Strings));
                        tmp = "";
                        openSingleQuote = false;
                    }
                    else{
                        openSingleQuote = true;
                    }
                }
                else if(isTokenDoubleQuote(rawString[it])){
                    if(openDoubleQuote){
                        tokens.push_back(std::make_pair(tmp, Strings));
                        tmp = "";
                        openDoubleQuote = false;
                    }
                    else{
                        openDoubleQuote = true;
                    }
                }
                else{
                    tmp += rawString[it];
                }
            }
            if(tmp.length() != 0){
                tokens.push_back(std::make_pair(tmp, lexicalAnalysis(tmp)));
            }
            return tokens;
        }

        int lexicalAnalysis(string& src){
            clearFlags();
            if(src[0] == '$'){
                if(isTokenValidIdentifier(src)){
                    return Identifiers;
                }
                else{
                    error("Lexing: Invalid Identifier.");
                }
            }    
            /* TODO: Does'nt handle cases where there are multiple signs, why would someone do it? */
            else if(isTokenSign(src[0])){
                return isFloatOrInteger(src, 1);
            }
            else if(isTokenNumber(src[0])){
                return isFloatOrInteger(src, 0);
            }
            else{
                if(isTokenName(src)){
                    return Names;
                }
                error("lexing: Not a valid name.");
            }
            error("TODO: Missed some case ? Or messing with me.");
            return 69;
        }

        /* Assumption: Keywords/ names will only contain lowercase alphabets and dot. */
        bool isTokenName(string& src){
            for(auto chr: src){
                if(!((int)chr >= 0x61 && (int)chr <= 0x7a) && !(chr == '.')){
                    return false;
                }
            }
            return true;
        }
        uint isFloatOrInteger(string& src, int indx){
            for(ull it=indx;it<src.length();it++){
                char chr = src[it];
                if(isTokenNumber(chr)){
                    if(!isFloat){
                        isInteger = true;
                    }
                }
                else if(chr == '.'){
                    if(isFloat){
                        error("lexing: Invalid Number Type.");
                    }
                    isInteger = false;
                    isFloat = true;
                }
                else{
                    /* TODO: Make Signs compatible with hex numbers. */
                    error("lexing: Invalid Number Type.");
                }
            }
            if(isInteger){
                return Integer;
            }
            else if(isFloat){
                return FloatingPoint;
            }
            error("Lexing: Invalid Number Type.");
            return 69;
        }
        bool isTokenValidIdentifier(string& src){
            for(auto chr: src){
                if(!((int)chr >= 0x21 && (int)chr<=0x7e) || isTokenBannedIdentifier(chr)){
                    return false;
                }
            }
            return true;
        }

        bool isTokenBannedIdentifier(char chr){
            if((int)chr == 0x20 || (int)chr == 0x22 || (int)chr == 0x27 || (int)chr == 0x2c || (int)chr == 0x28 || (int)chr == 0x29 || (int)chr == 0x7b || (int)chr == 0x7d || (int)chr == 0x5b || (int)chr == 0x5d){
                return true;
            }
            return false;
        }
        // bool isTokenCapitalAlphabet(char chr){
        //     return ((int)chr >= 0x41 && (int)chr<=0x5a);
        // }
        // bool isTokenSmallAlphabet(char chr){
        //     return ((int)chr >= 0x61 && (int)chr <= 0x7a);
        // }
        bool isTokenSign(char chr){
            return (chr == '+' || chr == '-');
        }
        bool isTokenNumber(char chr){
            return ((int)chr >= 0x30 && (int)chr<=0x39 );
        }
        bool isTokenDelimiter(char charToken){
            if(charToken == '(' || charToken == ')' || charToken == '\n' || charToken == ' '){
                return true;
            }
            return false;
        }
        bool isTokenSingleQuote(char charToken){
            return (charToken == '\'');
        }
        bool isTokenDoubleQuote(char charToken){
            return (charToken == '\"');
        }
        /* Take care of line comments and block comments. */
        string removeCommentsInRawString(string rawString){
            ull cnt = 0;
            ull rawStringLen = rawString.length();
            char* tmpString = (char *)malloc(rawStringLen);
            bool inMultilineComment = false;
            bool inSinglelineComment = false;

            /* TODO: Actually analyse if it goes OOB. We can add an Extra Null byte at the end to reduce security risk but compromising memory/ performance. */
            for(ull it=0;it<rawStringLen;it++){
                if(rawString[it] == ';' && rawString[it+1] == ')' && inMultilineComment){
                    inMultilineComment = false;
                    it++;
                }
                else if(rawString[it] == '(' && rawString[it+1] == ';'){
                    inMultilineComment = true;
                    it++;
                }
                else if(rawString[it] == ';' && rawString[it+1] == ';' && !inSinglelineComment){
                    inSinglelineComment = true;
                }
                else{
                    if(rawString[it] == '\n' && inSinglelineComment){
                        inSinglelineComment = false;
                    }
                    else if(!inSinglelineComment && !inMultilineComment){
                        tmpString[cnt] = rawString[it];
                        cnt++;
                    }
                }
            }
            return tmpString;
        }

        void clearFlags(){
            isInteger = false;
            isFloat = false;
            isString = false;
            isName = false;
        }
        /* Error */
        void error(string msg){
            cout << "[-] " << msg << std::endl;
            exit(-1);
        }
    private:
        /* Essentials for wapp. */
        string rawFileContents;
        string byteCodeStream;  
        unsigned long long byteCodeStreamLen;

        /* Flags */
        bool bytecodePrint;
        bool tokenPrint;
        bool astPrint;

        /* Helper Flags */
        bool isInteger = false;
        bool isFloat = false;
        bool isString = false;
        bool isName = false;
    };
}