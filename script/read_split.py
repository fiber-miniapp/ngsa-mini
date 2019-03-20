#!/usr/bin/python
# Copyright (C) 2012-2019 Riken Center for Computational Science (R-CCS).

## \file read-split.py KMR-NGSA Read File Splitter.

import sys
import os
import re
from optparse import OptionParser
import commands

##  Split a file using separator string.
#
#   @param nums    number of reads in a splitted file
#   @param odir    output directory of splitted files.
#   @param opref   output file prefix of splitted files.
#   @param infile  input file path.

def splitfile(nums, odir, opref, infile) :
    total_lines = int(commands.getoutput('wc -l ' + infile).split(' ')[0])
    out_lines = nums * 4
    n_files = total_lines/out_lines
    if not total_lines % out_lines == 0:
        n_files += 1

    print "Splitting file: ",
    try:
        fin = open(infile, "r")
    except IOError:
       print 'Error: could not open "%s".' % infile
       sys.exit()

    for i in range(n_files) :
        # compose output file name.
        # ex: partXXX, where XXX is number of part.
        suffix = '0' * (len(str(n_files-1)) - len(str(i))) + str(i)
        opath = os.path.join(odir, (opref + suffix))

        try:
            fout = open(opath, "w")
        except IOError:
            print 'Error: could not open "%s".' % opath
            sys.exit()
        for j in range(out_lines) :
            text = fin.readline()
            if not text :
                break
            fout.write(text)
        fout.close()
        sys.stdout.write('.')
        sys.stdout.flush()

    fin.close()
    print "done."

## kmrfsplit main routine.
#  It works on Python 2.4 or later.

if __name__ == "__main__":

    usage = "usage: %prog [options] inputfile"
    parser = OptionParser(usage)

    parser.add_option("-n",
                      "--num-reads",
                      dest="nums",
                      type="int",
                      help="number of reads in a splitted file",
                      metavar="number",
                      default=1)

    parser.add_option("-d",
                      "--output-directory",
                      dest="odir",
                      type="string",
                      help="output directory",
                      metavar="'string'",
                      default="./input/")

    parser.add_option("-p",
                      "--output-file-prefix",
                      dest="opref",
                      type="string",
                      help="output filename prefix",
                      metavar="'string'",
                      default="part")

    parser.add_option("-f",
                      "--force",
                      dest="force",
                      action="store_true",
                      help="force option",
                      default=False)

    (options, args) = parser.parse_args()

    # parameter check.
    if len(args) <> 1 :
        parser.error("missing parameter")
        sys.exit()

    inputfile = args[0]

    if not os.path.exists(inputfile) :
        print 'Error: inputfile %s is not exist.' % inputfile
        sys.exit()

    if os.path.exists(options.odir) :
        if not os.path.isdir(options.odir) :
            print 'Error: "%s" is not directory.' % options.odir
            sys.exit()
    else:
        if options.force :
            try:
                os.mkdir(options.odir)
            except IOError:
                print 'Error: could not create "%s".' % options.odir
                sys.exit()
        else:
            print 'Error: directory "%s" is not exist. create it or use -f option.' % options.odir
            sys.exit()

    splitfile(options.nums, options.odir, options.opref, inputfile)

# Copyright (C) 2012-2019 Riken Center for Computational Science (R-CCS).
# This library is distributed WITHOUT ANY WARRANTY.  This library can be
# redistributed and/or modified under the terms of the GNU LGPL-2.1.
