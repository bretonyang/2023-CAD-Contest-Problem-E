/**
 * References
 * - https://algs4.cs.princeton.edu/55compression/Huffman.java.html
 * - https://algs4.cs.princeton.edu/code/edu/princeton/cs/algs4/BinaryOut.java.html
 * - https://algs4.cs.princeton.edu/code/edu/princeton/cs/algs4/BinaryIn.java.html
 *
 * To compile this program on linux, use: g++ -std=c++11 -o decompress decompress.cpp
 */

#include <iostream>
#include <stdexcept>
#include <fstream>
#include <string>

using std::runtime_error;
using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::istreambuf_iterator;
using std::ofstream;
using std::string;


/**
 * Utility class for writing bits to ofstream.
 * Basically it writes 8 bits from buffer to ofstream when 8 bits are accumulated.
 */
class BinaryOut {
    using byte = unsigned char;
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
                // NOTE: this works since byte pads 0 from left
                bool bit = ((x >> (8 - i - 1)) & 1) == 1;
                writeBitHelper(bit);
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


/**
 * Utility class for reading bits from ifstream.
 * Basically it reads 8 bits from ifstream and fills up the buffer
 */
class BinaryIn {
    using byte = unsigned char;
    using bit = bool;

private:
    byte buffer;
    int n;
    ifstream &in;

    // returns true if input stream is used up
    bool isEmpty() {
        return in.eof();
    }

    void fillBuffer() {
        if (isEmpty()) {
            buffer = 0;
            n = -1;
        }
        else {
            in.read(reinterpret_cast<char*>(&buffer), sizeof(byte));
            n = 8;
        }
    }

public:
    BinaryIn(ifstream &in) : in(in), n(0), buffer(0) {
        fillBuffer();
    }

    // read 1 bit and return a bool (NOTE it's not 1 byte bool)
    bool readOneBitBool() {
        if (isEmpty()) throw runtime_error("File reached EOF already!");

        n--;
        bool x = ((buffer >> n) & 1) == 1;
        if (n == 0) fillBuffer();
        return x;
    }

    // read 1 byte and return a char
    char readChar() {
        if (isEmpty()) throw runtime_error("File reached EOF already!");

        // When there's exactly 1 byte in buffer, just return all bits in buffer
        if (n == 8) {
            char x = buffer;
            fillBuffer();
            return x;
        }
        // combine K bits in current buffer with first 8 - K bits in new buffer
        else {
            byte x = buffer;
            int oldN = n;
            x <<= (8 - n); // record first K bits

            fillBuffer();
            if (isEmpty()) throw runtime_error("File reached EOF already!");

            n = oldN;
            x |= (buffer >> n); // record 8 - K bits from new buffer
            return (char) x;
        }
    }

    // read 4 bytes and return an int
    int readInt() {
        int x = 0;
        for (int i = 0; i < 4; i++) {
            char c = readChar();
            x <<= 8;
            x |= (c & 0xff); // to avoid padding left with 1 when char is promoted to int implicitly!!!
        }
        return x;
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


/*************************************
 * Below are decompression functions *
 *************************************/

Node* readTrie(BinaryIn &in) {
    bool isLeaf = in.readOneBitBool();
    if (isLeaf)
        return new Node(in.readChar(), -1, nullptr, nullptr); // -1 is just dummy value for freq
    else
        return new Node(0, -1, readTrie(in), readTrie(in));
}

void decompress(BinaryIn &in, BinaryOut &out) {
    // read from input and construct Trie
    Node* root = readTrie(in);

    // get number of bytes of the uncompressed file
    int length = in.readInt();

    // Write decoded binary to output
    for (int i = 0; i < length; i++) {
        Node* n = root;
        // Traverse to decoded char corresponding to code
        while (!n->isLeaf()) {
            bool isRightChild = in.readOneBitBool();
            if (isRightChild)
                n = n->right;
            else
                n = n->left;
        }
        out.writeByte(n->ch);
    }
    out.close();
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        cout << "Usage: decompress.exe filenameCompressed.bin" << endl;
        return 1;
    }

    // open the files
    string inputPath = argv[1];
    ifstream iFile(inputPath, ios::binary);
    BinaryIn in(iFile);

    string removeExtension = inputPath.substr(0, inputPath.length() - 14);
    ofstream oFile(removeExtension + "Decompressed.bin", ios::binary);
    BinaryOut out(oFile);

    if (iFile && oFile) {
        decompress(in, out);
    } else {
        cout << "Failed to open file." << endl;
        return 1;
    }
}
