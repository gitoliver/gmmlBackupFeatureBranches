#!/bin/bash
printf "Testing 016.test.DrawGlycan.cc...\n"
g++ -std=c++17 -I $GEMSHOME/gmml/ -I $GEMSHOME/gmml/ -L$GEMSHOME/gmml/bin/ -Wl,-rpath,$GEMSHOME/gmml/bin/ tests/016.test.DrawGlycan.cc -lgmml -pthread -o drawGlycan
./drawGlycan > 016.output_drawGlycan.txt

for dotFile in `ls *.dot`
do
	dotFileName="${dotFile%.*}"
    dot -Tsvg:cairo:cairo $dotFile -o $dotFileName.svg > /dev/null 2>&1
    rm $dotFile
done

for svgFile in `ls *.svg`
do
	cmp $svgFile tests/correct_outputs/016.output_SVGs/$svgFile
	if ! cmp $svgFile tests/correct_outputs/016.output_SVGs/$svgFile > /dev/null 2>&1; then
		echo "Test FAILED!. Output file %s different to tests/correct_outputs/016.output_SVGs/%s\n" $svgFile $svgFile
	  	return 1;
	fi
	rm $svgFile
done

if ! cmp  016.output_drawGlycan.txt tests/correct_outputs/016.output_drawGlycan.txt > /dev/null 2>&1; then
    printf "Test FAILED!. Output file different\n"
    return 1;
else
    printf "Test passed.\n"
    rm drawGlycan 016.output_drawGlycan.txt
    return 0;
fi