#!/usr/bin/env python3

import argparse

parser = argparse.ArgumentParser(
    # prog='ProgramName',
    description='What the program does',
    epilog='Text at the bottom of help'
)

parser.add_argument('filename')
parser.add_argument('-v', '--verbose', action='store_true')
parser.add_argument('-c', '--count',
    help='very long text '*100
)
parser.add_argument('--very-long-option-name-longer-than-normal',
    help='this is a long option'
)

args = parser.parse_args()
print(args)
