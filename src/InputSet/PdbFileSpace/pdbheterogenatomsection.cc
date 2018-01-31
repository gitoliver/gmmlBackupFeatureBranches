#include "../../../includes/InputSet/PdbFileSpace/pdbheterogenatomsection.hpp"
#include "../../../includes/InputSet/PdbFileSpace/pdbatomcard.hpp"
#include "../../../includes/utils.hpp"
#include "../../../includes/common.hpp"

using namespace std;
using namespace gmml;
using namespace PdbFileSpace;

//////////////////////////////////////////////////////////
//                       CONSTRUCTOR                    //
//////////////////////////////////////////////////////////
PdbHeterogenAtomSection::PdbHeterogenAtomSection() : record_name_("HETATM") {}

PdbHeterogenAtomSection::PdbHeterogenAtomSection(stringstream &stream_block, string index)
{
    heterogen_atom_cards_ = PdbHeterogenAtomCardMap();
    ordered_heterogen_atom_cards_ = PdbHeterogenAtomOrderVector();
    string line;
    bool is_record_name_set = false;
    getline(stream_block, line);
    string temp = line;
    while (!Trim(temp).empty())
    {
        if(!is_record_name_set){
//            record_name_ = line.substr(0,6);
            record_name_ = "HETATM";
            Trim(record_name_);
            is_record_name_set=true;
        }

        PdbAtomCard* atom = new PdbAtomCard(line);
        //int ch = 97 + ConvertString<int>(Split(index,"_")[1]);
        atom->SetAtomCardIndexInResidueSet(index);
        //atom->SetAtomChainId((char)ch);
        heterogen_atom_cards_[atom->GetAtomSerialNumber()] = atom;
        ordered_heterogen_atom_cards_.push_back(atom);

        getline(stream_block, line);
        temp = line;
    }
}

//////////////////////////////////////////////////////////
//                         ACCESSOR                     //
//////////////////////////////////////////////////////////
string PdbHeterogenAtomSection::GetRecordName()
{
    return record_name_;
}

PdbHeterogenAtomSection::PdbHeterogenAtomCardMap PdbHeterogenAtomSection::GetHeterogenAtomCards()
{
    return heterogen_atom_cards_;
}

PdbHeterogenAtomSection::PdbHeterogenAtomOrderVector PdbHeterogenAtomSection::GetOrderedHeterogenAtomCards()
{
    return ordered_heterogen_atom_cards_;
}

//////////////////////////////////////////////////////////
//                          MUTATOR                     //
//////////////////////////////////////////////////////////
void PdbHeterogenAtomSection::SetRecordName(const string record_name)
{
    record_name_ = record_name;
}

void PdbHeterogenAtomSection::SetHeterogenAtoms(PdbHeterogenAtomCardMap heterogen_atoms)
{
    heterogen_atom_cards_.clear();
    for(PdbHeterogenAtomCardMap::iterator it = heterogen_atoms.begin(); it != heterogen_atoms.end(); it++)
    {
        PdbHeterogenAtom* heterogen_atom = (*it).second;
        int serial_number = (*it).first;
        heterogen_atom_cards_[serial_number] = heterogen_atom;
    }
}

void PdbHeterogenAtomSection::SetOrderedHeterogenAtoms(PdbHeterogenAtomOrderVector ordered_heterogen_atoms)
{
    ordered_heterogen_atom_cards_.clear();
    for(PdbHeterogenAtomOrderVector::iterator it = ordered_heterogen_atoms.begin(); it != ordered_heterogen_atoms.end(); it++)
    {
        PdbAtomCard* atom = (*it);
        ordered_heterogen_atom_cards_.push_back(atom);
    }
}
//////////////////////////////////////////////////////////
//                        FUNCTIONS                     //
//////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////
//                      DISPLAY FUNCTION                //
//////////////////////////////////////////////////////////
void PdbHeterogenAtomSection::Print(ostream &out)
{
    out << "Record Name: " << record_name_ << endl <<
           "________________ Heterogen Atoms ___________________" << endl;
    for(PdbHeterogenAtomSection::PdbHeterogenAtomCardMap::iterator it = heterogen_atom_cards_.begin(); it != heterogen_atom_cards_.end(); it++)
    {
        out << "Atom Serial Number: ";
        if((it)->first != iNotSet)
            out << (it)->first << endl;
        else
            out << " " << endl;
        (it)->second->Print();
        out << endl;
    }
}