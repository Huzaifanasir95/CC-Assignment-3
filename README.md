LL(1) Parser for Compiler Construction
Overview
This repository contains the implementation of a stack-based LL(1) parser for the CS-4031 Compiler Construction course at the National University of Computer and Emerging Sciences, Spring 2025. The parser, written in C, processes space-separated terminal symbols from an input file based on a given context-free grammar (CFG). It generates detailed parsing steps, handles errors gracefully, and outputs results to a file. The program is modular, reusable, and well-documented, designed to parse any LL(1)-compliant grammar.
Features

Reads a CFG from a text file (grammar.txt) and processes it to ensure LL(1) compatibility (left factoring, left recursion removal).
Computes First and Follow sets and builds an LL(1) parsing table.
Parses space-separated terminal strings from an input file (input.txt) using a stack-based LL(1) parsing algorithm.
Displays parsing steps (stack contents, input, actions) in a tabular format.
Handles syntax errors by skipping erroneous tokens and continuing parsing, with descriptive error messages.
Outputs grammar details, parsing tables, and parsing results to output.txt.

Repository Structure

file.c: The main C source file containing the parser implementation.
grammar.txt: Sample grammar file defining the CFG for parsing declarations, assignments, and if-statements.
input.txt: Sample input file with space-separated terminal strings to parse.
output.txt: Generated output file containing grammar details, parsing tables, and parsing logs.
README.md: This file, providing an overview and instructions.
report.pdf: A brief report detailing the approach, challenges, and verification process (included as per assignment requirements).

Prerequisites

A C compiler (e.g., gcc).
Standard C libraries (included with most compilers).

Installation and Usage

Clone the Repository:
git clone https://github.com/your-username/ll1-parser.git
cd ll1-parser


Compile the Program:
gcc temp.c -o parser


Run the Program:
./parser

The program reads grammar.txt and input.txt, processes the input, and generates output.txt.

Input/Output Files:

Modify grammar.txt to use a different LL(1)-compliant grammar.
Update input.txt with new space-separated terminal strings to parse.
Check output.txt for the parsing results, including grammar details, parsing tables, and step-by-step logs.



Example
Input (input.txt):
k i ;
i = i + n ;
f ( i > n ) { i = i + n ; }
f ( i = n ) { i = n ; } e { i = i ; }
i = ( i + n ) * i ;

Output (output.txt excerpt):
Line 1: k i ;
Step Stack                Input                     Action
1    $S                   k i ; $                   use S->D
2    $D                   k i ; $                   use D->ki;
3    $;ik                 k i ; $                   match 'k'
...
6    $                    $                         ACCEPT
Line 1: Parsed successfully.
...
Parsing completed with 0 total errors.

Implementation Details

Grammar Processing: The program reads the grammar, applies left factoring and left recursion removal, computes First/Follow sets, and builds the LL(1) parsing table.
Parsing: A stack-based LL(1) parser processes input tokens, matching terminals or expanding non-terminals based on the parsing table.
Error Handling: Errors (e.g., mismatched tokens) are detected, reported with clear messages, and handled by skipping tokens to continue parsing.
Output: Detailed logs include grammar transformations, parsing tables, and step-by-step parsing actions.

Challenges and Solutions

Error Recovery: Ensured robust error handling by skipping only erroneous tokens, preserving parser synchronization.
Stack Management: Debugged stack operations to handle complex productions correctly, using reverse-order pushing.
Output Formatting: Used helper functions to create consistent, aligned tables for parsing steps.

Verification

Tested with valid inputs from input.txt, confirming successful parsing.
Validated error handling with invalid inputs (e.g., missing symbols).
Checked grammar processing by manually verifying First/Follow sets and parsing tables.
Tested genericity with alternative LL(1) grammars.

License
This project is for educational purposes and is not licensed for commercial use. Feel free to use it as a reference for learning compiler construction concepts.
Contact
For questions or feedback, please open an issue on this repository or contact the contributors.
