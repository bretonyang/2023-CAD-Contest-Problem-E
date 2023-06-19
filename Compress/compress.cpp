/**
 * References:
 * - https://algs4.cs.princeton.edu/55compression/Huffman.java.html
 * - https://algs4.cs.princeton.edu/code/edu/princeton/cs/algs4/BinaryOut.java.html
 *
 * To compile this program on linux, use: g++ -std=c++11 -o compress compress.cpp
 */

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <queue>
#include <vector>

using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::istreambuf_iterator;
using std::ofstream;
using std::string;
using std::unordered_map;
using std::priority_queue;
using std::vector;


/**
 * Utility class for writing bits to ofstream.
 * Basically it writes 8 bits from buffer to ofstream when 8 bits are accumulated.
 */
class BinaryOut {
    // Use unsigned char to represent 8 bits, s.t. >> pads in 0
    using byte = unsigned char;

    // technically bool is still 8-bit, but let's just use it like 1 bit
    // where true is 1, false is 0
    using bit = bool;

private:
    byte buffer; // 8-bit buffer
    int n; // buffer size
    ofstream &out; // reference to output file stream

    void writeBitHelper(bit x) {
        // write bit to buffer LSB
        buffer <<= 1;
        if (x) buffer |= 1;

        n++;
        if (n == 8) clearBuffer(); // write 8-bit if buffer is full
    }

    void writeByteHelper(byte x) {
        // When buffer is empty, we can directly write a byte to file
        if (n == 0) {
            out.write(reinterpret_cast<const char*>(&x), sizeof(byte));
        }
        // write one bit at a time to the buffer, from MSB to LSB
        else {
            for (int i = 0; i < 8; i++) {
                bool b = ((x >> (8 - i - 1)) & 1) == 1;
                writeBitHelper(b);
            }
        }
    }

    void clearBuffer() {
        if (n == 0) return;

        // write out bits in buffer, padding with 0 from right
        if (n > 0) buffer <<= (8 - n);
        out.write(reinterpret_cast<const char*>(&buffer), sizeof(byte));

        n = 0;
        buffer = 0;
    }

public:
    BinaryOut(ofstream &out) : out(out), n(0), buffer(0) {}

    void writeBit(bit x) {
        writeBitHelper(x);
    }

    void writeByte(byte x) {
        writeByteHelper(x);
    }

    void writeUnsignedInt(unsigned int x) {
        // use unsigned to make sure >> pads in 0 from left
        writeByteHelper((x >> 24) & 0xff); // MSB
        writeByteHelper((x >> 16) & 0xff);
        writeByteHelper((x >> 8) & 0xff);
        writeByteHelper((x >> 0) & 0xff); // LSB
    }

    void close() {
        clearBuffer();
    }
};


/** Node class for the Trie */
class Node {
public:
    char ch; // NOTE: char range is -128 ~ 127
    int freq;
    Node* left;
    Node* right;

    Node(char ch, int freq, Node* left, Node* right) :
        ch(ch), freq(freq), left(left), right(right) {}

    bool isLeaf() {
        return (left == nullptr) && (right == nullptr);
    }
};


/** Node Pointer comparator for the priority_queue of Node pointers */
struct NodePtrComparator {
    bool operator()(const Node* lhs, const Node* rhs) const {
        return lhs->freq > rhs->freq;
    }
};


/***********************************
 * Below are compression functions *
 ***********************************/

Node* buildTrie(unordered_map<char, int> &freq) {
    // push trees with only one node into the Min PQ
    priority_queue<Node*, vector<Node*>, NodePtrComparator> pq;
    for (auto it : freq)
        pq.push(new Node(it.first, it.second, nullptr, nullptr));

    // Merge 2 smallest trees into a larger tree until we have only 1 tree
    while (pq.size() > 1) {
        Node* left = pq.top();
        pq.pop();
        Node* right = pq.top();
        pq.pop();
        pq.push(new Node(0, left->freq + right->freq, left, right));
    }
    Node* root = pq.top();
    pq.pop();
    return root;
}

void buildTable(unordered_map<char, string> &table, Node* n, string code) {
    if (n->isLeaf()) {
        table[n->ch] = code;
    }
    else {
        buildTable(table, n->left, code + "0");
        buildTable(table, n->right, code + "1");
    }
}

/** write a prefix traversal of the Trie */
void writeTrie(Node* n, BinaryOut &out) {
    if (n->isLeaf()) {
        out.writeBit(1);
        out.writeByte(n->ch); // note that char to unsigned char won't change underlying bit pattern
    }
    else {
        out.writeBit(0);
        writeTrie(n->left, out);
        writeTrie(n->right, out);
    }
}

void compress(string &bytes, BinaryOut &out) {
    // Record the frequency of each char
    unordered_map<char, int> freq;
    for (char c : bytes)
        freq[c]++;

    // construct Trie based on the frequency table
    Node* root = buildTrie(freq);

    // construct table with code for each char s.t. the most frequent char
    // gets the shortest prefix-free binary code
    unordered_map<char, string> table;
    buildTable(table, root, "");

    // write Trie data for decompression
    writeTrie(root, out);

    // Write number of bytes in the original binary file
    unsigned int length = (unsigned int) bytes.length(); // this cast is legal only because size is guaranteed to be < 1MB
    out.writeUnsignedInt(length);

    // write Huffman code to the compressed binary file
    for (char byte : bytes) {
        string code = table[byte];
        for (char bit : code) {
            if (bit == '0')
                out.writeBit(0);
            else
                out.writeBit(1);
        }
    }

    out.close();
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        cout << "Usage: compress.exe filename.bin" << endl;
        return 1;
    }

    // open the files
    string inputPath = argv[1];
    ifstream iFile(inputPath, ios::binary);

    string removeExtension = inputPath.substr(0, inputPath.length() - 4); // assume extension is 4 chars long
    ofstream oFile(removeExtension + "Compressed.bin", ios::binary);
    BinaryOut out(oFile);

    if (iFile && oFile) {
        // get the bytes in the file as 8-bit chars and store them in a string
        string bytes((istreambuf_iterator<char>(iFile)), istreambuf_iterator<char>());
        compress(bytes, out);
    } else {
        cout << "Failed to open file." << endl;
        return 1;
    }

    return 0;
}
