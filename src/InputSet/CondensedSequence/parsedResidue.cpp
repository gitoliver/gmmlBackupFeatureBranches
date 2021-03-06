#include <sstream>
#include "includes/InputSet/CondensedSequence/parsedResidue.hpp"

using CondensedSequence::ParsedResidue;

ParsedResidue::ParsedResidue(std::string residueString, ParsedResidue::Type specifiedType) 
: Node(this, residueString), fullResidueString_ (residueString)  
{
    this->ParseResidueStringIntoComponents(residueString, specifiedType);
}

ParsedResidue::ParsedResidue(std::string residueString, ParsedResidue* neighbor, ParsedResidue::Type specifiedType) 
: Node(this, residueString), fullResidueString_ (residueString) 
{
    this->ParseResidueStringIntoComponents(residueString, specifiedType);
	this->AddLinkage(neighbor);
}

void ParsedResidue::AddLinkage(ParsedResidue* otherRes) 
{
    //std::string label = this->GetName() + "->" + otherRes->GetName();
    //std::cout << "Adding Edge: " << label << std::endl;
    if ( this->GetType() == Type::Sugar ) 
    {
        this->AddEdge(otherRes, this->GetConfiguration() + this->GetLinkage());
    }
    else
    {
        this->AddEdge(otherRes, this->GetLinkage());
    }
    return;
}

std::string ParsedResidue::GetLink()
{
    std::string linkage = this->GetLinkage();
    switch (this->GetType())
    {
        case (Type::Sugar):
            return linkage.substr(linkage.size() - 1, 1);
        case (Type::Derivative):
            return linkage.substr(linkage.size() - 1, 1);
        case (Type::Deoxy):
            return linkage.substr(0, 1);
        default:
            return "0";
    }
}

std::vector<ParsedResidue*> ParsedResidue::GetChildren()
{
    return this->GetIncomingNeighborObjects();
}

std::vector<ParsedResidue*> ParsedResidue::GetParents()
{
    return this->GetOutgoingNeighborObjects();
}

std::string ParsedResidue::GetChildLinkagesForGlycamResidueNaming()
{
    std::string linkages;
    for (auto &child : this->GetChildren())
    {   // For glycam residue name, e.g. 2YB, do not want deoxy linkages to impact the residue name.
        if (child->GetType() != Type::Deoxy)
        {
            linkages += (child->GetLink() + ",");
        }
    }
    if (linkages.empty())
    {
        linkages = "Terminal";
    }
    else     // Erase the last ","
    {
        linkages.erase(std::prev(linkages.end()));
    }
    return linkages;
}

std::string ParsedResidue::GetName(const bool withLabels)
{
    if (withLabels)
    {
        return FindLabelContaining("&Label=");
    }
    return this->GetIsomer() + this->GetResidueName() + this->GetRingType() + this->GetResidueModifier() + this->GetRingShape();
}

std::string ParsedResidue::GetLinkageName(const bool withLabels)
{   // Should only ever be zero or one outEdges in my current design.
    for (auto &linkage : this->GetOutEdges())
    {
        if (withLabels)
        {
            return linkage->FindLabelContaining("&Label=");
        }
        else
        {
            return linkage->GetLabel();
        }
    }    
    return ""; // aglycone/reducing terminal will not have linkage.
}

void ParsedResidue::ParseResidueStringIntoComponents(std::string residueString, ParsedResidue::Type specifiedType)
{
	//std::cout << "PARSING RESIDUE: " << residueString << std::endl;
    // Set defaults:
    this->SetIsomer("");
    this->SetResidueName("");
    this->SetRingType("");
    this->SetRingShape("");
    this->SetResidueModifier("");
    this->SetConfiguration("");
    this->SetLinkage("");
    this->SetType(specifiedType);
	if ( (residueString.find('-') != std::string::npos) || (specifiedType == Type::Sugar) )
    { // E.g. DManpNAca1-4 . Isomer (D or L), residueName (ManNAc), ring type (f or p), configuration (a or b), linkage (1-4)
    	// Reading from front.
        this->SetType(Type::Sugar);
        // Assumptions
        size_t residueStart = 1; // e.g. Gal, Glc, Ido
        size_t modifierStart = 5; // E.g. NAc, A, A(1C4)
        // Checks
        std::string isomer = residueString.substr(0, 1);
        if ((isomer == "D") || (isomer == "L")) 
        {
            this->SetIsomer(isomer);
        }
        else
        {
            residueStart--;
            modifierStart--;
        }
        this->SetResidueName(residueString.substr(residueStart, 3));
        size_t ringPosition = (residueStart + 3);
        std::string ringType = residueString.substr(ringPosition, 1);
        if (( ringType == "p") || (ringType == "f"))
        {
            this->SetRingType(ringType);
        }
        else
        {
            modifierStart--;
        }
        // Find the dash, read around it.
        size_t dashPosition = residueString.find('-');
        if (dashPosition == std::string::npos) // There is no -. e.g. Fru in DGlcpa1-2DFrufb
        {
            dashPosition = residueString.size() + 1;
        }
        else
        {
            this->SetLinkage(residueString.substr((dashPosition - 1), 3 ));
        }
        std::string configuration = residueString.substr(dashPosition - 2, 1);
        if (( configuration == "a" || configuration == "b"))
        {
            this->SetConfiguration(residueString.substr(dashPosition - 2, 1));
        }
    	// Find any special modifiers e.g. NAc, Gc, A in IdoA
    	size_t modifierLength = (dashPosition - modifierStart - 2); // They are 2 apart if no modifier
        //std::cout << "modifierLength is " << modifierLength << ", dashPosition was " << dashPosition << ", ringPosition was " << ringPosition << std::endl;
        if (modifierLength > 100)
        {
            std::string message = "Non standard glycam residue string: " + residueString;
            //throw message;
            std::cout << message << std::endl;
        }
    	if (modifierLength > 0 && modifierLength < 100)
        {
    		this->SetResidueModifier(residueString.substr(modifierStart, modifierLength));
            this->ExciseRingShapeFromModifier();
            //std::cout << "Modifier is " << this->GetResidueModifier() << std::endl;
        }
    	else
        {
    		this->SetResidueModifier("");
        }
    }
    else if ( isdigit(residueString[0]) )
    { // A derivative e.g. 3S, 6Me. Linkage followed by residue name. No configuration.
        this->SetType(Type::Derivative);
    	this->SetLinkage(residueString.substr(0, 1));
    	this->SetResidueName(residueString.substr(1)); // From position 1 to the end.
        if ( (this->GetResidueName() == "D") || (this->GetResidueName() == "H") ) // Supporting both for now.
        {
            this->SetType(Type::Deoxy);
        }
    }
    else if (specifiedType == Type::Aglycone)
    { // A terminal
    	this->SetResidueName(residueString);
    }
    else
    { // Dunno.
        std::string message = "Error: we can't parse this residue: \"" + residueString + "\""; 
        throw message;
    }
    std::cout << this->Print();
}

std::string ParsedResidue::Print()
{
	std::stringstream ss;
    ss << this->GetIsomer() << "_" 
				<< this->GetResidueName() << "_"
				<< this->GetRingType() << "_"
                << this->GetResidueModifier() << "_"
                << this->GetRingShape() << "_"
				<< this->GetConfiguration() << "_"
				<< this->GetLinkage() << ".\n";
    return ss.str();
}

std::string ParsedResidue::GetMonosaccharideName()
{
    return this->GetIsomer() + this->GetResidueName() + this->GetResidueModifier() + this->GetRingShape();
}

void ParsedResidue::ExciseRingShapeFromModifier()
{ // E.g. LIdopA(4C1)a1-4 with modifier "A(4C1)", which here gets broken into ring shape "4C1" and modifier "A".
    std::string modifier = this->GetResidueModifier();
    size_t leftParenthesisPosition = modifier.find('(');
    size_t rightParenthesisPosition = modifier.find(')');
    
    if ( (leftParenthesisPosition == std::string::npos) || (rightParenthesisPosition == std::string::npos) )
    { // If there isn't a ring shape declared.
        return;
    }
    else
    { // Assumes it's always at end of modifiers
        this->SetRingShape(modifier.substr(leftParenthesisPosition));
        this->SetResidueModifier(modifier.substr(0,leftParenthesisPosition));
    }
    return;
}

