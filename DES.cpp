#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <fstream>
#include <sstream>
#include <time.h>
#include <iostream>

using namespace std;

#define LB32_MASK   0x00000001
#define LB64_MASK   0x0000000000000001
#define L64_MASK    0x00000000ffffffff
#define H64_MASK    0xffffffff00000000

#define EXTRACT(u,b) ((u) >> (8 * (b)))

/* Initial Permutation Table */
static char IP[] = {
    58, 50, 42, 34, 26, 18, 10,  2,
    60, 52, 44, 36, 28, 20, 12,  4,
    62, 54, 46, 38, 30, 22, 14,  6,
    64, 56, 48, 40, 32, 24, 16,  8,
    57, 49, 41, 33, 25, 17,  9,  1,
    59, 51, 43, 35, 27, 19, 11,  3,
    61, 53, 45, 37, 29, 21, 13,  5,
    63, 55, 47, 39, 31, 23, 15,  7
};

/* Inverse Initial Permutation Table */
static char PI[] = {
    40,  8, 48, 16, 56, 24, 64, 32,
    39,  7, 47, 15, 55, 23, 63, 31,
    38,  6, 46, 14, 54, 22, 62, 30,
    37,  5, 45, 13, 53, 21, 61, 29,
    36,  4, 44, 12, 52, 20, 60, 28,
    35,  3, 43, 11, 51, 19, 59, 27,
    34,  2, 42, 10, 50, 18, 58, 26,
    33,  1, 41,  9, 49, 17, 57, 25
};

/*Expansion table */
static char E[] = {
    32,  1,  2,  3,  4,  5,
     4,  5,  6,  7,  8,  9,
     8,  9, 10, 11, 12, 13,
    12, 13, 14, 15, 16, 17,
    16, 17, 18, 19, 20, 21,
    20, 21, 22, 23, 24, 25,
    24, 25, 26, 27, 28, 29,
    28, 29, 30, 31, 32,  1
};

/* Post S-Box permutation */
static char P[] = {
    16,  7, 20, 21,
    29, 12, 28, 17,
     1, 15, 23, 26,
     5, 18, 31, 10,
     2,  8, 24, 14,
    32, 27,  3,  9,
    19, 13, 30,  6,
    22, 11,  4, 25
};

/* The S-Box tables */
static char S[8][64] = { {
        /* S1 */
        14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
         0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
         4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
        15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13
    },{
        /* S2 */
        15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
         3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
         0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
        13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9
    },{
        /* S3 */
        10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
        13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
        13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
         1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12
    },{
        /* S4 */
         7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
        13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
        10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
         3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14
    },{
        /* S5 */
         2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
        14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
         4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
        11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3
    },{
        /* S6 */
        12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
        10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
         9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
         4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13
    },{
        /* S7 */
         4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
        13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
         1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
         6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12
    },{
        /* S8 */
        13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
         1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
         7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
         2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11
    } };

/* Permuted Choice 1 Table */
static char PC1[] = {
    57, 49, 41, 33, 25, 17,  9,
     1, 58, 50, 42, 34, 26, 18,
    10,  2, 59, 51, 43, 35, 27,
    19, 11,  3, 60, 52, 44, 36,

    63, 55, 47, 39, 31, 23, 15,
     7, 62, 54, 46, 38, 30, 22,
    14,  6, 61, 53, 45, 37, 29,
    21, 13,  5, 28, 20, 12,  4
};

/* Permuted Choice 2 Table */
static char PC2[] = {
    14, 17, 11, 24,  1,  5,
     3, 28, 15,  6, 21, 10,
    23, 19, 12,  4, 26,  8,
    16,  7, 27, 20, 13,  2,
    41, 52, 31, 37, 47, 55,
    30, 40, 51, 45, 33, 48,
    44, 49, 39, 56, 34, 53,
    46, 42, 50, 36, 29, 32
};

/* Iteration Shift Array */
static char iteration_shift[] = {
    /* 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16 */
       1,  1,  2,  2,  2,  2,  2,  2,  1,  2,  2,  2,  2,  2,  2,  1
};

//------------------------------------** Functions **-------------------------------------

bool validateName(string operation, string inName, string outName) {
    bool validate = false;
    string inExt = inName.substr(inName.find("."));
    string outExt = outName.substr(outName.find("."));


    if (operation == "encrypt") {
        if ((inExt != ".txt") && (outExt != ".dat"))
            validate = false;
        else if ((inExt == ".txt") && (outExt == ".dat"))
            validate = true;
    }

    else if (operation == "decrypt") {
        if ((inExt != ".dat") && (outExt != ".txt"))
            validate = false;
        else if ((inExt == ".dat") && (outExt == ".txt"))
            validate = true;
    }

    // always valid
    return 1;
}

void printUsage() {
    cout << "\n----------------Please Enter Correct Command.----------------\n\n";

    cout << "Usage:\n";
    cout << "To Encrypt the File use the Command: ProgramName encrypt {inFile}\t {keyFile} {EncryptedOutFile}\n";
    cout << "To Decrypt the File use the Command: ProgramName decrypt {EncryptedFile} {keyFile} {DecryptedOutFile}\n";
    cout << "-------------------------------------------------------------\n";
}

string readFileString(string fileName) {
    string str, output;
    fstream file;
    long long int size;
    //long long int newsize;



    file.open(fileName, ios::in);
    file.seekg(0, ios::end);
    size = file.tellg();
    //cout << size << "\n\n";  // comment//-------------------------------------------------------------
    file.close();

    //ifstream file(fileName);
    file.open(fileName, ios::in);
    if (!file) {
        cout << "Failed to Open";
        return 0;
    }

    while (getline(file, str)) {
        if (str.size() > 0)
            output += str;
    }

    return output;
}

uint64_t stringToHexa(string block) {
    uint64_t res = 0;

    std::stringstream ss;
    ss << std::hex << block;
    ss >> res;

    return res;
}

string hexaToString(uint64_t hexa) {
    string result;
    uint8_t b[8] = {
        EXTRACT(hexa,0),
        EXTRACT(hexa,1),
        EXTRACT(hexa,2),
        EXTRACT(hexa,3),
        EXTRACT(hexa,4),
        EXTRACT(hexa,5),
        EXTRACT(hexa,6),
        EXTRACT(hexa,7),
    };

    char left_hex, right_hex;
    for (int i = 7; i >= 0; i--) {
        left_hex = ((b[i] >> 4) & 0x0F), right_hex = (b[i] & 0x0F);
        (left_hex < 10) ? left_hex += 48 : left_hex += 55;
        (right_hex < 10) ? right_hex += 48 : right_hex += 55;
        result += (left_hex);
        result += (right_hex);
    }

    return result;
}

inline uint64_t ASCII2uint64(string b){
    return ((uint64_t)(uint8_t)b[0] << 56) | ((uint64_t)(uint8_t)b[1] << 48) | 
        ((uint64_t)(uint8_t)b[2] << 40) | ((uint64_t)(uint8_t)b[3] << 32) | 
        ((uint64_t)(uint8_t)b[4] << 24) | ((uint64_t)(uint8_t)b[5] << 16) | 
        ((uint64_t)(uint8_t)b[6] << 8) | ((uint64_t)(uint8_t)b[7]);
}

inline string uint64ToASCII(uint64_t a){
    string result = "";
    for (unsigned int i = 0; i < 8; ++i){
        result += (unsigned char)((a >> (56 - (i << 3))) & 0xFF);
    }
    return result;
}

string read(string fileName){

    string str, output = "";
    int size = 0, modular = 0;

    ifstream file(fileName, ios::binary);
    if (!file){
        cout << "the file failed to open, please enter the correct FileName " << endl;
        return 0;
    }

    while (getline(file, str)){
        output += str + "\n";
    }

    size = output.length();
    modular = size % 8;
    for (int i = 0; i < 8 - modular; i++) {
        output += '\0';
    }

    return output;
}


/* 48 bits */
uint64_t sub_key[16] = { 0 };
void generateKeys(uint64_t key) {
    int i, j;

    /* 28 bits */
    uint32_t C = 0;
    uint32_t D = 0;

    /* 48 bits */
    //uint64_t sub_key[16] = { 0 };

    /* 56 bits */
    uint64_t permuted_choice_1 = 0;
    uint64_t permuted_choice_2 = 0;

    /* initial key schedule calculation */
    for (i = 0; i < 56; i++) {

        permuted_choice_1 <<= 1;
        permuted_choice_1 |= (key >> (64 - PC1[i])) & LB64_MASK;

    }

    C = (uint32_t)((permuted_choice_1 >> 28) & 0x000000000fffffff);
    D = (uint32_t)(permuted_choice_1 & 0x000000000fffffff);

    /* Calculation of the 16 keys */
    for (i = 0; i < 16; i++) {

        /* key schedule */
        // shifting Ci and Di
        for (j = 0; j < iteration_shift[i]; j++) {

            C = 0x0fffffff & (C << 1) | 0x00000001 & (C >> 27);
            D = 0x0fffffff & (D << 1) | 0x00000001 & (D >> 27);

        }

        permuted_choice_2 = 0;
        permuted_choice_2 = (((uint64_t)C) << 28) | (uint64_t)D;

        sub_key[i] = 0;

        for (j = 0; j < 48; j++) {

            sub_key[i] <<= 1;
            sub_key[i] |= (permuted_choice_2 >> (56 - PC2[j])) & LB64_MASK;

        }

    }

}

/*
 * The DES function
 * input: 64 bit message
 * key: 64 bit key for encryption/decryption
 * mode: 'e' = encryption; 'd' = decryption
 */

uint64_t des(uint64_t input, string mode) {

    int i, j;

    /* 8 bits */
    char row, column;

    /* 28 bits */
    //uint32_t C = 0;
    //uint32_t D = 0;

    /* 32 bits */
    uint32_t L = 0;
    uint32_t R = 0;
    uint32_t s_output = 0;
    uint32_t f_function_res = 0;
    uint32_t temp = 0;

    /* 48 bits */
    //uint64_t sub_key[16] = { 0 };
    uint64_t s_input = 0;

    /* 56 bits */
    //uint64_t permuted_choice_1 = 0;
    //uint64_t permuted_choice_2 = 0;

    /* 64 bits */
    uint64_t init_perm_res = 0;
    uint64_t inv_init_perm_res = 0;
    uint64_t pre_output = 0;

    /* initial permutation */
    for (i = 0; i < 64; i++) {

        init_perm_res <<= 1;
        init_perm_res |= (input >> (64 - IP[i])) & LB64_MASK;

    }

    L = (uint32_t)(init_perm_res >> 32) & L64_MASK;
    R = (uint32_t)init_perm_res & L64_MASK;

    ///* initial key schedule calculation */
    //for (i = 0; i < 56; i++) {

    //    permuted_choice_1 <<= 1;
    //    permuted_choice_1 |= (key >> (64 - PC1[i])) & LB64_MASK;

    //}

    //C = (uint32_t)((permuted_choice_1 >> 28) & 0x000000000fffffff);
    //D = (uint32_t)(permuted_choice_1 & 0x000000000fffffff);

    ///* Calculation of the 16 keys */
    //for (i = 0; i < 16; i++) {

    //    /* key schedule */
    //    // shifting Ci and Di
    //    for (j = 0; j < iteration_shift[i]; j++) {

    //        C = 0x0fffffff & (C << 1) | 0x00000001 & (C >> 27);
    //        D = 0x0fffffff & (D << 1) | 0x00000001 & (D >> 27);

    //    }

    //    permuted_choice_2 = 0;
    //    permuted_choice_2 = (((uint64_t)C) << 28) | (uint64_t)D;

    //    sub_key[i] = 0;

    //    for (j = 0; j < 48; j++) {

    //        sub_key[i] <<= 1;
    //        sub_key[i] |= (permuted_choice_2 >> (56 - PC2[j])) & LB64_MASK;

    //    }

    //}

    for (i = 0; i < 16; i++) {

        /* f(R,k) function */
        s_input = 0;

        for (j = 0; j < 48; j++) {

            s_input <<= 1;
            s_input |= (uint64_t)((R >> (32 - E[j])) & LB32_MASK);

        }

        /*
         * Encryption/Decryption
         * XORing expanded Ri with Ki
         */
        if (mode == "decrypt") {
            // decryption
            s_input = s_input ^ sub_key[15 - i];

        }
        else if(mode == "encrypt"){
            // encryption
            s_input = s_input ^ sub_key[i];

        }

        /* S-Box Tables */
        for (j = 0; j < 8; j++) {
            // 00 00 RCCC CR00 00 00 00 00 00 s_input
            // 00 00 1000 0100 00 00 00 00 00 row mask
            // 00 00 0111 1000 00 00 00 00 00 column mask

            row = (char)((s_input & (0x0000840000000000 >> 6 * j)) >> 42 - 6 * j);
            row = (row >> 4) | row & 0x01;

            column = (char)((s_input & (0x0000780000000000 >> 6 * j)) >> 43 - 6 * j);

            s_output <<= 4;
            s_output |= (uint32_t)(S[j][16 * row + column] & 0x0f);

        }

        f_function_res = 0;

        for (j = 0; j < 32; j++) {

            f_function_res <<= 1;
            f_function_res |= (s_output >> (32 - P[j])) & LB32_MASK;

        }

        temp = R;
        R = L ^ f_function_res;
        L = temp;

    }

    pre_output = (((uint64_t)R) << 32) | (uint64_t)L;

    /* inverse initial permutation */
    for (i = 0; i < 64; i++) {

        inv_init_perm_res <<= 1;
        inv_init_perm_res |= (pre_output >> (64 - PI[i])) & LB64_MASK;

    }

    return inv_init_perm_res;

}


int main(int argc, char** argv) {


    if (argc != 5) {

        printUsage();
        return 0;
    }

    else {

        string op = argv[1];
        string inFileName = argv[2];
        string KeyFile = argv[3];
        string outFileName = argv[4];

        uint64_t input = 0;
        uint64_t result = 0;

        uint64_t key = 0;
        key = stringToHexa(readFileString(KeyFile));
        
        long long int size;
        string block;
        string type;
        //string type = "-a";
     

        if (op == "encrypt") {
            if (validateName(op, inFileName, outFileName)) {

                //ifstream fin(inFileName, ios::binary);
                size_t buffer_size = 8;
                char* buffer = new char[buffer_size];

                cout << "\nPlease specify the output File Type of the Encryption process: \n";
                cout << "Note that you should enter: \n For ASCI type: -a\n For HEXA type: -h\n\n";

                do {
                    cin >> type;
                    if ((type != "-a") && (type != "-h")){ // && (type != "-b")) {
                        cout << "\t------------------------------------------------\n";
                        cout << "\nError choosing File Type.\n";
                        cout << "Please follow the instructions:\n";
                        cout << "Note that you should enter:\n For ASCI type: -a\n For HEXA type: -h\n\n";
                    }
                } while ((type != "-a") && (type != "-h")); // && (type != "-b"));


                // Generate HEXA Encrypted File
                if (type == "-h") {

                    ifstream fin(inFileName); // , ios::binary);
                    ofstream outFile;
                    string out;
                    string outHexa = outFileName.replace(outFileName.find("."), 4, "-HEXA.dat");
                    outFile.open(outHexa); // , ios::binary);

                    clock_t start, end;

                    cout << "\nGenerating Encrypted HEXA File.....\nPlease wait.....\n";

                    start = clock();
                    generateKeys(key);

                    while (fin) {
                        // number of Characters = buffer_size
                        fin.read(buffer, buffer_size);


                        // Get the number of bytes actually read
                        size_t count = fin.gcount();

                        if (!count)  // end of file
                            break;


                        while (count < buffer_size) {
                            buffer[count] = '\0';
                            count++;
                        }

                        for (int i = 0; i < buffer_size; i++) {
                            block += buffer[i];
                        }

                        //DESEncrypt()

                        input = ASCII2uint64(block);
                        result = des(input, op);
                        out = hexaToString(result);


                        outFile << out;

                        out.erase();
                        block.erase();
                    }

                    outFile.close();
                    delete[] buffer;
                    cout << "Encrypted Successfully" << "\n\n";


                    end = clock();

                    double time_taken = double(end - start) / double(CLOCKS_PER_SEC);
                    long milli = time_taken * 1000;
                    //long milli = (t2 - t1);
                    //3600000 milliseconds in an hour
                    long hr = milli / 3600000;
                    milli = milli - 3600000 * hr;
                    //60000 milliseconds in a minute
                    long min = milli / 60000;
                    milli = milli - 60000 * min;

                    //1000 milliseconds in a second
                    long sec = milli / 1000;
                    milli = milli - 1000 * sec;


                    cout << "Time taken: " << hr << " hours and " << min << " minutes and " << sec << " seconds and " << milli << " milliseconds." << endl;

                }

                // Generate ASCII Encrypted File
                else if (type == "-a") {
                    ifstream fin(inFileName, ios::binary);
                    ofstream outFile;
                    string out;
                    string outAscii = outFileName.replace(outFileName.find("."), 4, "-ASCII.dat");
                    outFile.open(outAscii, ios::binary);

                    clock_t start, end;

                    cout << "\nGenerating Encrypted ASCII File.....\nPlease wait.....\n";

                    start = clock();
                    generateKeys(key);

                    while (fin) {
                        // number of Characters = buffer_size
                        fin.read(buffer, buffer_size);


                        // Get the number of bytes actually read
                        size_t count = fin.gcount();

                        if (!count)  // end of file
                            break;


                        while (count < buffer_size) {
                            buffer[count] = '\0';
                            count++;
                        }

                        for (int i = 0; i < buffer_size; i++) {
                            block += buffer[i];
                        }

                        //DESEncrypt() 
                        input = ASCII2uint64(block);
                        result = des(input, op);
                        out = uint64ToASCII(result);


                        outFile << out;

                        out.erase();
                        block.erase();
                    }

                    outFile.close();
                    delete[] buffer;
                    cout << "Encrypted Successfully" << "\n\n";


                    end = clock();

                    double time_taken = double(end - start) / double(CLOCKS_PER_SEC);
                    long milli = time_taken * 1000;
                    //long milli = (t2 - t1);
                    //3600000 milliseconds in an hour
                    long hr = milli / 3600000;
                    milli = milli - 3600000 * hr;
                    //60000 milliseconds in a minute
                    long min = milli / 60000;
                    milli = milli - 60000 * min;

                    //1000 milliseconds in a second
                    long sec = milli / 1000;
                    milli = milli - 1000 * sec;


                    cout << "Time taken: " << hr << " hours and " << min << " minutes and " << sec << " seconds and " << milli << " milliseconds." << endl;

                }


            }


            else
                printUsage();

            return 0;
        }


        else if (op == "decrypt") {  

            if (validateName(op, inFileName, outFileName)) {


                cout << "\nPlease specify the input File Type of the Deccryption process: \n";
                cout << "Note that you should enter: \n For ASCI type: -a\n For HEXA type: -h\n\n";

                do {
                    cin >> type;
                    if ((type != "-a") && (type != "-h")) {
                        cout << "\t------------------------------------------------\n";
                        cout << "\nError choosing File Type.\n";
                        cout << "Please follow the instructions:\n";
                        cout << "Note that you should enter:\n For ASCI type: -a\n For HEXA type: -h\n For both type: -b\n\n";
                    }
                } while ((type != "-a") && (type != "-h"));


                // Decrypt from ASCII
                if(type == "-a") {
                    
                    ifstream fin(inFileName, ios::binary);

                    ofstream outFile;
                    string out;

                    outFile.open(outFileName);
                    size_t buffer_size = 8;
                    char* buffer = new char[buffer_size];


                    clock_t start, end;

                    cout << "\nDecrypting The ASCII File.....\nPlease wait.....\n";

                    start = clock();
                    generateKeys(key);


                    while (fin) {
                        // Try to read next chunk of data, read 16 chars by 16 chars
                        fin.read(buffer, buffer_size);

                        // Get the number of bytes actually read
                        size_t count = fin.gcount();

                        if (!count)  // end of file
                            break;

                        while (count < buffer_size) {
                            buffer[count] = '\0';
                            count++;
                        }

                        for (int i = 0; i < buffer_size; i++) {
                            block += buffer[i];
                        }


                        input = ASCII2uint64(block);
                        result = des(input, op);
                        out = uint64ToASCII(result);


                        outFile << out;

                        out.erase();
                        block.erase();
                    }

                    outFile.close();
                    delete[] buffer;
                    cout << "Decrypted Successfully" << "\n\n";


                    end = clock();

                    double time_taken = double(end - start) / double(CLOCKS_PER_SEC);
                    long milli = time_taken * 1000;

                    // long milli = (t2 - t1);
                    //3600000 milliseconds in an hour
                    long hr = milli / 3600000;
                    milli = milli - 3600000 * hr;
                    //60000 milliseconds in a minute
                    long min = milli / 60000;
                    milli = milli - 60000 * min;

                    //1000 milliseconds in a second
                    long sec = milli / 1000;
                    milli = milli - 1000 * sec;


                    cout << "Time taken: " << hr << " hours and " << min << " minutes and " << sec << " seconds and " << milli << " milliseconds." << endl;

                }

                // Decrypt from HEXA
                else if (type == "-h") {

                ifstream fin(inFileName); // , ios::binary);

                ofstream outFile;
                string out;

                outFile.open(outFileName);
                size_t buffer_size = 16;
                char* buffer = new char[buffer_size];


                clock_t start, end;

                cout << "\nDecrypting The HEXA File.....\nPlease wait.....\n";

                start = clock();
                generateKeys(key);

                while (fin) {
                    // Try to read next chunk of data, read 16 chars by 16 chars
                    fin.read(buffer, buffer_size);

                    // Get the number of bytes actually read
                    size_t count = fin.gcount();

                    if (!count)  // end of file
                        break;


                    while (count < buffer_size) {
                        buffer[count] = '\0';
                        count++;
                    }


                    for (int i = 0; i < buffer_size; i++) {
                        block += buffer[i];
                    }


                    //DESDecrypt() function takes block(16 hexadecimal characters) as input
                    input = stringToHexa(block);
                    result = des(input, op);
                    out = uint64ToASCII(result);


                    outFile << out;

                    out.erase();
                    block.erase();
                }

                outFile.close();
                delete[] buffer;
                cout << "Decrypted Successfully" << "\n\n";


                end = clock();

                double time_taken = double(end - start) / double(CLOCKS_PER_SEC);
                long milli = time_taken * 1000;

                // long milli = (t2 - t1);
                //3600000 milliseconds in an hour
                long hr = milli / 3600000;
                milli = milli - 3600000 * hr;
                //60000 milliseconds in a minute
                long min = milli / 60000;
                milli = milli - 60000 * min;

                //1000 milliseconds in a second
                long sec = milli / 1000;
                milli = milli - 1000 * sec;


                cout << "Time taken: " << hr << " hours and " << min << " minutes and " << sec << " seconds and " << milli << " milliseconds." << endl;
                }

            }

            else
                printUsage();

            return 0;

                
        }

    }

    return 0;
}

