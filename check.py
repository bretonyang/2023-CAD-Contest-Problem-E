
# Usage: python3 test.py InputFileName CompressedFileName DecompressedFileName

import sys


def error(msg):
    sys.stderr.write(f'{msg}\n')


def main(argv):
    # Check valid number of arguments
    if len(argv) != 4:
        error('Usage: python3 check.py OriginalFileName CompressedFileName DecompressedFileName')
        sys.exit(1)

    # Read in the .bin files
    f1, f2, f3 = argv[1:4]
    with open(f1, 'rb') as f1_fh, open(f2, 'rb') as f2_fh, open(f3, 'rb') as f3_fh:
        input_bytes = f1_fh.read()  # bytes
        comp_bytes = f2_fh.read()  # bytes
        decomp_bytes = f3_fh.read()  # bytes

    # Output result
    print()
    ratio = len(comp_bytes) / len(input_bytes)
    print(f'Input binary file size: {len(input_bytes)} bytes')
    print(f'Compressed binary file size: {len(comp_bytes)} bytes')
    print(f'Decompressed binary file size: {len(decomp_bytes)} bytes')
    print()
    print(f'==> Compression Ratio: {ratio:.4f}')
    if input_bytes == decomp_bytes:
        print('==> Compression result: lossless')
    elif len(input_bytes) > len(decomp_bytes):
        print('==> Compression result: some data is lost')
    else:
        print('==> Compression result: some unnecessary data is added')


if __name__ == '__main__':
    main(sys.argv)
