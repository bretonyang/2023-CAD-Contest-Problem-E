#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <queue>
#include <vector>

using std::cout, std::endl;
using std::ifstream, std::ios, std::istreambuf_iterator, std::ofstream;
using std::string;
using std::unordered_map, std::priority_queue, std::vector;

using namespace std;


class BinaryIn {
private:

};


/*************************************
 * Below are decompression functions *
 *************************************/

int main(int argc, char **argv)
{
    if (argc != 2) {
        cout << "Usage: ./compress testcase1.bin" << endl;
        return 1;
    }

    // get the bytes in the file as 8-bit chars and store them in a string
    ifstream iFile(argv[1], ios::binary);
    ofstream oFile("../decompressedFile.bin", ios::binary);
    BinaryOut out(oFile);

    if (iFile && oFile) {
        string bytes((istreambuf_iterator<char>(iFile)), istreambuf_iterator<char>());
        compress(bytes, out);
    } else {
        cout << "Failed to open file." << endl;
        return 1;
    }

    return 0;
}
