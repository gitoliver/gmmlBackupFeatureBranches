#include <string>
#include <algorithm> // Reverse function.

#include "includes/InputSet/CondensedSequence/sequenceParser.hpp"

using CondensedSequence::SequenceParser;
using CondensedSequence::ParsedResidue;

SequenceParser::SequenceParser (std::string inputSequence)
{
    
	if (inputSequence.find(';') != std::string::npos)
	{
		std::cout << "Found labels in input\n";
		this->ParseLabelledInput(inputSequence);
	}
	else
	{
		std::cout << "Parsing unlabelled input sequence:\n" << inputSequence << "\n";
        if (this->CheckSequenceSanity(inputSequence))
        {
            std::cout << "Sequence is sane\n";
            this->ParseCondensedSequence(inputSequence);
        }
    }
    return;
}


std::string SequenceParser::Print()
{
    std::string output = "";
    for (auto &residue : this->GetParsedResidues())
    {
        output += residue->Print();
    }
    return output;
}

std::vector<ParsedResidue*> SequenceParser::GetParsedResidues()
{
    std::vector<ParsedResidue*> rawResidues;
    for(auto &residue : parsedResidues_)
    {
        rawResidues.push_back(residue.get());
    }
    return rawResidues;
}

void SequenceParser::ParseLabelledInput(std::string inString)
{
	// char delimiter = ';';
	// std::vector<std::string> tokens = gmml::splitStringByDelimiter(inString, delimiter);
	// delimiter = '&';
	// for (auto &element : tokens)
	// {
	// 	// std::vector<std::string> labelAndLabelee = gmml::splitStringByDelimiter(element, delimiter);
	// 	// for (auto &subtoken : labelAndLabelee)
	// 	// {
	// 		std::cout << " " << subtoken << " \n";
	// 	}
	// }
    throw "Error: We can't handle labeled stuff yet: " + inString + "\n";
}

bool SequenceParser::ParseCondensedSequence(const std::string sequence)
{
    // Reading from the end of the string.
	size_t i = (sequence.find_last_of('-') + 1);
    if ( isdigit(sequence[i]) )
    { // e.g. DGlcpa1-2DFrufb or 
        ++i; // ano-ano
        if (sequence[i] == ']') // e.g. DGlcpa1-2[LFucpa1-1]DFrufb
        {
            ++i;
        }
        parsedResidues_.push_back(std::make_unique<ParsedResidue>(sequence.substr(i), ParsedResidue::Type::Sugar));
    }
    else
    { // e.g. DGlcpa1-OH
        parsedResidues_.push_back(std::make_unique<ParsedResidue>(sequence.substr(i), ParsedResidue::Type::Aglycone));
    }
    auto terminal = parsedResidues_.back().get();
    std::cout << "That was a terminal save with i=" << i << "\n";
    this->RecurveParseAlt(i, sequence, terminal); 
    return true;
}

void SequenceParser::RecurveParseAlt(size_t &i, const std::string sequence, ParsedResidue* parent)
{
    //std::cout << "Started RecurveParseAlt with i=" << i << " and sequence[i] is " << sequence[i] << "\n";
    size_t windowEnd = i;
    while (i > 0)
    {
        i--;
        //std::cout << "i=" << i << " sequence[i]=" << sequence[i] << "\n";
        if ( ( sequence[i] == '-' ) && ((windowEnd - i) > 5 ) )
        { // dash and have read enough that there is a residue to save.
          //  std::cout << "- with i=" << i << "\n";
            parent = this->SaveResidue(i + 2, windowEnd, sequence, parent);
            windowEnd = i + 2; // Get to the right side of the number e.g. 1-4
        }
        else if ( sequence[i] == ']' )
        { // Start of branch: recurve. Maybe save if have read enough.
          //  std::cout << "Starting recurve down a branch as have found a ] with i=" << i << "\n";
            if ((windowEnd - i) > 5)
            {   // if not a derivative start and have read enough, save.
                parent = this->SaveResidue(i + 1, windowEnd, sequence, parent);
           //     std::cout << "Recurving with branch\n";
                this->RecurveParseAlt(i, sequence, parent);
           //     std::cout << "Returned from branch\n";
                windowEnd = i;
            }
            else if ((windowEnd - i) > 1) // and not > 5
            { // Derivative. Note that windowEnd does not move.
            //    std::cout << "Recurving with derivative\n";
                this->RecurveParseAlt(i, sequence, parent);
            //    std::cout << "Returned from derivative recurve\n";
            }
            else 
            {  // Return from branch and find new one. e.g. [Gal][Gal]
            //    std::cout << "Recurving with ][ branch\n";
                this->RecurveParseAlt(i, sequence, parent);
            //    std::cout << "Returned from ][ branch\n";
                windowEnd = i;
            }
        }
        else if ( sequence[i] == '[' )
        { // End of branch
            //std::cout << "Saving and returning from recurve due to [ with i=" << i << "\n"; 
            this->SaveResidue(i + 1, windowEnd, sequence, parent);
            return;
        }
        else if ( sequence[i] == ',' )
        { // , in derivative list
            //std::cout << ", so saving derivative with i=" << i << "\n";
            parent = this->SaveResidue(i + 1, windowEnd, sequence, parent);
            windowEnd = i;
        }
        else if ( i == 0)
        { // Fin.
            //std::cout << "i == 0 so need to save and then I'm finished!" << "\n";
            parent = this->SaveResidue(i, windowEnd, sequence, parent);
        }
    }
    return; 
}

ParsedResidue* SequenceParser::SaveResidue(const size_t windowStart, const size_t windowEnd, const std::string sequence, ParsedResidue* parent)
{
    std::string residueString = sequence.substr(windowStart, (windowEnd - windowStart));
    //std::cout << "At start of save: " << residueString << std::endl;
    // Splice out anything within [ and ].
    if (residueString.find('[') != std::string::npos) 
    {
        size_t branch_start = residueString.find_first_of('[');
        size_t branch_finish = residueString.find_last_of(']') + 1;
        std::string firstPart = residueString.substr(0, branch_start);
        std::string lastPart = residueString.substr(branch_finish);
        residueString = firstPart + lastPart;
        //std::cout << firstPart << " + " << lastPart << std::endl;
    }
    if (residueString.find('-') != std::string::npos)
    {
        std::cout << "Saving " << residueString << " with parent " << parent->GetName() <<  std::endl;
        parsedResidues_.push_back(std::make_unique<ParsedResidue>(residueString, parent));
        auto newRes = parsedResidues_.back().get();
        if(this->DerivativesExist())
        {
            for(auto &derivative : this->ExtractDerivatives())
            {
                std::cout << "Saving derivative: " << derivative << " with parent " << newRes->GetName() <<  std::endl;
                parsedResidues_.push_back(std::make_unique<ParsedResidue>(derivative, newRes));
            }
        }
        return newRes;
    }
    else // A derivatve. The parent residue doesn't exist yet, so save it.
    {
        //std::cout << "Temporarily holding derivative: " << residueString << "\n";
        this->SaveDerivative(residueString);
        return parent;
    }
}

std::vector<std::string> SequenceParser::ExtractDerivatives()
{
    auto derivativesCopy = savedDerivatives_;
    savedDerivatives_.clear();
    return derivativesCopy;
}

bool SequenceParser::CheckSequenceSanity(std::string sequence)
{
    if ( sequence.empty() )
    {
        throw "Error: sequence is empty:\n>>>" + sequence + "<<<\n" ;
    }
    if ( sequence.find("cake") != std::string::npos )
    {
        throw "Error: the cake is a lie:\n>>>" + sequence + "<<<\n" ;
    }
    if ( sequence.find(" ") != std::string::npos )
    {
        throw "Error: sequence contains a space:\n>>>" + sequence + "<<<\n" ;
    }
    std::vector<char> badChars = {'\'', '_', '+', '"', '`'};
    for (auto &badChar : badChars)
    {
        if (sequence.find(badChar) != std::string::npos)
        {
            std::string s(1, badChar); // convert to string     
            throw "Error: sequence cannot contain this:\'" + s + "\':\n>>>" + sequence + "<<<\n" ; 
        }
    }
    size_t a = std::count(sequence.begin(), sequence.end(), '[');
    size_t b = std::count(sequence.begin(), sequence.end(), ']');
    if (a != b)
    {
        throw "Error: the number of [ doesn't match the number of ]. Bad branch in :\n>>>" + sequence + "<<<\n" ;
    }
    return true;
}


