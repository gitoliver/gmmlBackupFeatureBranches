#!/bin/bash

printf "Testing writing original and new residue numbers into a PDB file... "
g++ -std=c++0x -I $GEMSHOME/gmml/ -L$GEMSHOME/gmml/bin/ -Wl,-rpath,$GEMSHOME/gmml/bin/ tests/011.writeResNumbers.cc -lgmml -pthread -o writeResNumbers
./writeResNumbers tests/inputs/pdb2glycam_4YG0.pdb > 011.output_writeResidueNumbers.txt
if [ -f 011.output.newNumbers.pdb ] && [ -f 011.output.original.pdb ]; then
    if ! cmp 011.output.newNumbers.pdb tests/correct_outputs/011.output.newNumbers.pdb > /dev/null 2>&1; then
        printf "Test FAILED!. PDB file with new numbers is different\n"
        return 1;
    elif ! cmp 011.output.original.pdb tests/correct_outputs/011.output.original.pdb > /dev/null 2>&1; then
        printf "Test FAILED!. PDB file with original numbers is different\n"
        return 1;
    else
        printf "Test passed.\n"
        rm 011.output.newNumbers.pdb 011.output.original.pdb writeResNumbers 011.output_writeResidueNumbers.txt
    fi
else
    printf "Test FAILED!.\n"
    return 1;
fi

