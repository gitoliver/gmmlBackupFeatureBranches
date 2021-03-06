#include <sstream>
#include "includes/InputSet/CondensedSequence/assemblyBuilder.hpp"
#include "includes/MolecularMetadata/GLYCAM/glycam06DerivativeAglyconeInfo.hpp"
#include "includes/MolecularMetadata/GLYCAM/glycam06DerivativeChargeAdjustment.hpp"
#include "includes/MolecularMetadata/GLYCAM/glycam06ResidueNameGenerator.hpp" // To get glycam name for ParsedResidue
#include "includes/MolecularModeling/Abstract/Residue.hpp" // For the Residue::Type
#include "includes/MolecularModeling/Selections/selections.hpp"
#include "includes/MolecularModeling/assembly.hpp" // Only to use silly Assembly functions. Should go away. 
#include "includes/MolecularModeling/atom.hpp" // For setting Angles and bond distances
#include "includes/ParameterSet/PrepFileSpace/prepfile.hpp"

//using Abstract::Residue; // For Residue::Type

using CondensedSequence::AssemblyBuilder;
using MolecularModeling::Assembly;

AssemblyBuilder::AssemblyBuilder(std::string inputSequence, std::string prepFilePath, Assembly *inputAssembly) : SequenceManipulator{inputSequence} 
{
	this->ReorderSequence(); // Linkages must be in ascending order for looking up Glycam codes? Fix this dependancy Oliver.
	PrepFileSpace::PrepFile prepFile(prepFilePath);
	this->SetPrepResidueMap(prepFile.GetResidues()); //A mapping between a residue name and its residue object
	this->GenerateResidues(inputAssembly);
	this->EnsureIntegralCharge(inputAssembly->GetTotalCharge());
	return;
}

void AssemblyBuilder::EnsureIntegralCharge(double charge)
{
	std::cout << std::fixed;
	std::cout << "Total charge is: " << std::setprecision(5) << charge << std::endl;
	double difference = std::fabs(charge - (std::round(charge)));
	if (difference > 0.00001 && difference < 0.99999)
	{
		std::stringstream errorMessage; 
		errorMessage << "Non-integral charge (" << charge << "). You cannot run MD with this.\n";
		std::cout << errorMessage.str();
		throw errorMessage.str();
	}
	return;
}

void AssemblyBuilder::GenerateResidues(Assembly *assembly)
{
	std::vector<MolecularModeling::Residue> createdResidues;
	createdResidues.reserve(this->GetParsedResidues().size());
	auto aglycone = this->GetTerminal();
	auto result = this->GetPrepResidueMap()->find(this->GetGlycamResidueName(*aglycone));
	std::cout << "Found prep entry: " << result->first << " for " << aglycone->GetName() << "\n";
	Residue& gmmlParent = assembly->CreateResidue(result->second, aglycone->GetType());
	gmmlParent.AddLabel(aglycone->GetLabel());
	for (auto &child : aglycone->GetChildren())
	{
		this->RecurveGenerateResidues(child, gmmlParent, assembly);	
	}
	std::cout << "Jobs a good un" << std::endl;
	return;
}

void AssemblyBuilder::RecurveGenerateResidues(ParsedResidue* parsedChild, MolecularModeling::Residue& gmmlParent, 
	Assembly* assembly)
{	
	//std::cout << "Recurve Gen Res" << std::endl;
	if (parsedChild->GetType() == ParsedResidue::Type::Deoxy)
	{
		std::cout << "Dealing with deoxy for " << gmmlParent.GetName() << std::endl;
		gmmlParent.MakeDeoxy(parsedChild->GetLink());
		return;
	}
	auto prepEntry = this->GetPrepResidueMap()->find(this->GetGlycamResidueName(*parsedChild));
	if (prepEntry == this->GetPrepResidueMap()->end())
	{
		std::cout << "Could not find prep entry for " << parsedChild->GetName() << " Glycam: " << this->GetGlycamResidueName(*parsedChild) << std::endl; 
	}
	else
	{
		std::cout << "Found prep entry: " << prepEntry->first << " for " << parsedChild->GetName() << "\n";
		Residue& newGmmlChild = assembly->CreateResidue(prepEntry->second, parsedChild->GetType());
		newGmmlChild.AddLabel(parsedChild->GetLabel());
		//newGmmlChild.AddEdge(&gmmlParent, parsedChild->GetLinkageName());
		this->BondResiduesDeduceAtoms(gmmlParent, newGmmlChild, parsedChild->GetLinkageName());
		//this->InitializeInterResidueGeometry(gmmlParent, newGmmlChild);
		//std::cout << "Recurve created " << newGmmlChild.GetLabel() << std::endl;
		for (auto &child : parsedChild->GetChildren())
		{
			this->RecurveGenerateResidues(child, newGmmlChild, assembly);	
		}
	}
	return;
}

// All this stuff should go into Residue. Residue has private Head and child atoms with private getters/setters. Solves this mess.
void AssemblyBuilder::BondResiduesDeduceAtoms(MolecularModeling::Residue& parentResidue, MolecularModeling::Residue& childResidue, std::string linkageLabel)
{
	// This is using the new Node<Residue> functionality and the old AtomNode 
	parentResidue.AddEdge(&childResidue, linkageLabel);
	// Now go figure out how which Atoms to bond to each other in the residues.
	// Rule: Can't ever have a child aglycone or a parent derivative.
	std::string parentAtomName, childAtomName;
	if (parentResidue.GetType() == Residue::Type::Aglycone)
	{ 
		gmml::MolecularMetadata::GLYCAM::Glycam06DerivativeAglyconeConnectionAtomLookup connectionAtomLookup;
		parentAtomName = connectionAtomLookup.GetConnectionAtomForResidue(parentResidue.GetName());
	}
	else if (parentResidue.GetType() == Residue::Type::Sugar)
	{ // Linkage example: childb1-4parent, it's never parentb1-4child 
		size_t linkPosition = 3;
		if (childResidue.GetType() == Residue::Type::Derivative)
		{ // label will be just a single number.
			linkPosition = 0;
		}
		parentAtomName = selection::GetNonCarbonHeavyAtomNumbered(parentResidue.GetAtoms(), linkageLabel.substr(linkPosition));
	}
	Atom* parentAtom = parentResidue.GetAtom(parentAtomName);
	// Now get child atom
	if (childResidue.GetType() == Residue::Type::Derivative)
	{
		gmml::MolecularMetadata::GLYCAM::Glycam06DerivativeAglyconeConnectionAtomLookup connectionAtomLookup;
		childAtomName = connectionAtomLookup.GetConnectionAtomForResidue(childResidue.GetName());	
	}
	else if (childResidue.GetType() == Residue::Type::Sugar)
	{
		auto childLinkageNumber = linkageLabel.substr(1,1);
		childAtomName = "C" + childLinkageNumber;
	}
	Atom* childAtom = childResidue.GetAtom(childAtomName);
	// Now bond the atoms. Needs to change when AtomNode goes away.
	childAtom->GetNode()->AddNodeNeighbor(parentAtom);
	parentAtom->GetNode()->AddNodeNeighbor(childAtom);
	std::cout << "Bonded " << parentResidue.GetName() << "@" << parentAtomName << " to " << childResidue.GetName() << "@" << childAtomName << std::endl;
	// Charge adjustment
	if (childResidue.GetType() == Residue::Type::Derivative)
	{
	//	std::cout << "Charge Adjustment.\n";
		gmml::MolecularMetadata::GLYCAM::Glycam06DerivativeChargeAdjustmentLookupContainer lookup;
		std::string adjustAtomName = lookup.GetAdjustmentAtom(childResidue.GetName());
		adjustAtomName += linkageLabel.substr(0,1); 
		Atom* atomToAdjust = parentResidue.GetAtom(adjustAtomName);
		std::cout << "    Derivative is " << childResidue.GetName() << ". Adjusting charge on " << atomToAdjust->GetName() << "\n";
		std::cout << "    Adjusting by: " << lookup.GetAdjustmentCharge(childResidue.GetName()) << "\n";
		atomToAdjust->SetCharge(atomToAdjust->GetCharge() + lookup.GetAdjustmentCharge(childResidue.GetName()) );
	}
	// Geometry
	//std::cout << "Setting bond distance.\n";
	MolecularModeling::Assembly whyOhGodWhy; // Doing as few changes as possible. These functions should be in a geometryTopology namespace.	
	whyOhGodWhy.SetResidueResidueBondDistance(parentAtom, childAtom);
	// Angle
	//std::cout << "Setting angles.\n";
	const double angle_to_set = 109.4;
	Atom* parentAtomNeighbor;
	for (auto &neighbor : parentAtom->GetNode()->GetNodeNeighbors())
	{ 
		if ( (neighbor->GetName().at(0) != 'H') && (neighbor != childAtom ) )
		{
			parentAtomNeighbor = neighbor;
			//std::cout << "Found neighbor, I'll set the angle now!\n";
			whyOhGodWhy.SetAngle(parentAtomNeighbor, parentAtom, childAtom, angle_to_set);
		}
	}
	return;	
}
	
std::string AssemblyBuilder::GetGlycamResidueName(ParsedResidue &residue)
{
    std::string linkages = "";
    if (residue.GetType() == ParsedResidue::Type::Sugar)
    {
        linkages = residue.GetChildLinkagesForGlycamResidueNaming();
    }
    std::string code = gmml::MolecularMetadata::GLYCAM::Glycam06ResidueNameGenerator(linkages, residue.GetIsomer(), residue.GetResidueName(), 
                                                                            residue.GetRingType(), residue.GetResidueModifier() + residue.GetRingShape(), residue.GetConfiguration() );
    return code;
}
