"""
This script processes the output from the C preprocessor and extracts all
qstr. Each qstr is transformed into a qstr definition of the form 'Q(...)'.

This script works with Python 2.6, 2.7, 3.3 and 3.4.
"""

from __future__ import print_function

import re
import sys
import io
import os


def write_out(fname, output):
    if output:
        for m, r in [("/", "__"), ("\\", "__"), (":", "@"), ("..", "@@")]:
            fname = fname.replace(m, r)
        file = args.output_dir + "/" + fname + ".qstr"
        try:
            with open(file, "w") as f:
                f.write("\n".join(output) + "\n")
        except FileNotFoundError:
            print("File path too long, please modify \\HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\FileSystem\\LongPathsEnabled to 1")
            sys.exit(1)

def process_file(f):
    re_line = re.compile(r"#[line]*\s\d+\s\"([^\"]+)\"")
    re_qstr = re.compile(r'MP_QSTR_[_a-zA-Z0-9]+')
    output = []
    last_fname = None
    for line in f:
        if line.isspace():
            continue
        # match gcc-like output (# n "file") and msvc-like output (#line n "file")
        if line.startswith(('# ', '#line')):
            m = re_line.match(line)
            assert m is not None
            fname = m.group(1)
            if not fname.endswith(".c"):
                continue
            if fname != last_fname:
                write_out(last_fname, output)
                output = []
                last_fname = fname
            continue
        for match in re_qstr.findall(line):
            name = match.replace('MP_QSTR_', '')
            output.append('Q(' + name + ')')

    write_out(last_fname, output)
    return ""


def cat_together():
    import glob
    import hashlib
    hasher = hashlib.md5()
    all_lines = []
    outf = open(args.output_dir + "/out", "wb")
    for fname in glob.glob(args.output_dir + "/*.qstr"):
        with open(fname, "rb") as f:
            lines = f.readlines()
            all_lines += lines
    all_lines.sort()
    all_lines = b"\n".join(all_lines)
    outf.write(all_lines)
    outf.close()
    hasher.update(all_lines)
    new_hash = hasher.hexdigest()
    #print(new_hash)
    old_hash = None
    try:
        with open(args.output_file + ".hash") as f:
            old_hash = f.read()
    except IOError:
        pass
    if old_hash != new_hash:
        print("QSTR updated")
        try:
            # rename below might fail if file exists
            os.remove(args.output_file)
        except:
            pass
        os.rename(args.output_dir + "/out", args.output_file)
        with open(args.output_file + ".hash", "w") as f:
            f.write(new_hash)
    else:
        print("QSTR not updated")


if __name__ == "__main__":
    if len(sys.argv) != 5:
        print('usage: %s command input_filename output_dir output_file' % sys.argv[0])
        sys.exit(2)

    class Args:
        pass
    args = Args()
    args.command = sys.argv[1]
    args.input_filename = sys.argv[2]
    args.output_dir = sys.argv[3]
    args.output_file = sys.argv[4]

    try:
        os.makedirs(args.output_dir)
    except OSError:
        pass

    if args.command == "split":
        with io.open(args.input_filename, encoding='utf-8') as infile:
            process_file(infile)

    if args.command == "cat":
        cat_together()
