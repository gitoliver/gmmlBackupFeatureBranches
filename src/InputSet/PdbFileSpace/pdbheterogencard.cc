// Author: Alireza Khatamian

#include "../../../includes/InputSet/PdbFileSpace/pdbheterogencard.hpp"
#include "../../../includes/common.hpp"
#include "../../../includes/utils.hpp"
#include <iostream>

using namespace std;
using namespace PdbFileSpace;
using namespace gmml;

//////////////////////////////////////////////////////////
//                       CONSTRUCTOR                    //
//////////////////////////////////////////////////////////
PdbHeterogenCard::PdbHeterogenCard() : heterogen_id_(""), chain_identifier_(' '), sequence_number_(dNotSet), insertion_code_(' '),
    number_of_heterogen_atoms_(dNotSet), dscr_("") {}

PdbHeterogenCard::PdbHeterogenCard(const string &heterogen_id, char chain_identifier, int sequence_number,
                           char insertion_code, int number_of_heterogen_atoms, const string &dscr)
    : heterogen_id_(heterogen_id), chain_identifier_(chain_identifier), sequence_number_(sequence_number), insertion_code_(insertion_code),
      number_of_heterogen_atoms_(number_of_heterogen_atoms), dscr_(dscr) {}

PdbHeterogenCard::PdbHeterogenCard(stringstream& stream_block)
{
    string line;
    getline(stream_block, line);
    string temp = line;
    while (!Trim(temp).empty())
    {
        heterogen_id_ = line.substr(7,3);
        Trim(heterogen_id_);
        if(line.substr(12,1) == " ")
        {
            chain_identifier_ = ' ';
        }
        else
        {
            chain_identifier_ = ConvertString<char>(line.substr(12,1));
        }
        if(line.substr(13, 4) == "    ")
            sequence_number_ = iNotSet;
        else
            sequence_number_ = ConvertString<int>(line.substr(13,4));
        if(line.substr(17,1) == " ")
        {
            insertion_code_ = ' ';
        }
        else
        {
            insertion_code_ = ConvertString<char>(line.substr(17,1));
        }
        if(line.substr(20, 5) == "     ")
            number_of_heterogen_atoms_ = iNotSet;
        else
            number_of_heterogen_atoms_ = ConvertString<int>(line.substr(20,5));
        dscr_ = line.substr(30,40);

        getline(stream_block, line);
        temp = line;
    }
}
//////////////////////////////////////////////////////////
//                         ACCESSOR                     //
//////////////////////////////////////////////////////////
string PdbHeterogenCard::GetHeterogenId()
{
    return heterogen_id_;
}

char PdbHeterogenCard::GetChainId()
{
    return chain_identifier_;
}

int PdbHeterogenCard::GetSequenceNumber()
{
    return sequence_number_;
}

char PdbHeterogenCard::GetInsertionCode()
{
    return insertion_code_;
}

int PdbHeterogenCard::GetNumberOfHeterogenAtoms()
{
    return number_of_heterogen_atoms_;
}

string PdbHeterogenCard::GetDscr()
{
    return dscr_;
}

//////////////////////////////////////////////////////////
//                          MUTATOR                     //
//////////////////////////////////////////////////////////
void PdbHeterogenCard::SetHeterogenId(const string heterogen_id)
{
    heterogen_id_ = heterogen_id;
}

void PdbHeterogenCard::SetChainId(char chain_identifier)
{
    chain_identifier_ = chain_identifier;
}

void PdbHeterogenCard::SetSequenceNumber(int sequence_number)
{
    sequence_number_ = sequence_number;
}

void PdbHeterogenCard::SetInsertionCode(char insertion_code)
{
    insertion_code_ = insertion_code;
}

void PdbHeterogenCard::SetNumberOfHeterogenAtoms(int number_of_heterogen_atoms)
{
    number_of_heterogen_atoms_ = number_of_heterogen_atoms;
}

void PdbHeterogenCard::SetDscr(const string dscr)
{
    dscr_ = dscr;
}

//////////////////////////////////////////////////////////
//                        FUNCTIONS                     //
//////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////
//                      DISPLAY FUNCTION                //
//////////////////////////////////////////////////////////
void PdbHeterogenCard::Print(ostream &out)
{
    out << "Heterogen ID: " << heterogen_id_
        << ", Chain Identifier: " << chain_identifier_
        << ", Sequence Number: ";
    if(sequence_number_ != iNotSet)
        out << sequence_number_;
    else
        out << " ";
    out << ", Insertion Code: " << insertion_code_
        << ", Number of Heterogen Atoms: ";
    if(number_of_heterogen_atoms_ != iNotSet)
        out << number_of_heterogen_atoms_;
    else
        out << " ";
    out << ", Description: " << dscr_ << endl;
}
