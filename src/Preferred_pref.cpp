/**
 * @file	Preferred_pref.cpp
 * @class	Preferred
 * @brief	Implementation of the recursive function `pref`
 * @author	Maurizio Zucchelli
 * @version	1
 * @date	2013-06-25
 */

#include "Preferred.h"

/**
 * @brief	Main resolutive function.
 * @details	Fills the labels of the class with the preferred extensions found.
 * @todo	Everything
 *
 * @param[in]	theAF	The Argument Framework to be considered
 * @param[in]	theC	The subset of arguments to consider
 */
void Preferred::pref( AF* theAF, SetArguments* theC )
{
	if ( debug )
		cerr << "Entering pref\n";

	this->cleanlabs();
	this->af = theAF;
	this->C = theC;

	if ( stages ) {
		cerr << "AF initialized. Showing structure" << endl
			 << this->af->toString() << endl;
	}

	SetArguments e = SetArguments(),
				 I = SetArguments();
	// Avoid calling Grounded if useless
	if ( theC->cardinality() <= 1 && *theC == *(theAF->get_arguments()) )
	{
		if ( debug )
			cerr << "\tNo need to call Grounded.\n";

		// Empty: no extensions => null iteration
		// Singlet: still have to iterate over I
		for ( SetArgumentsIterator it = theC->begin(); it != theC->end(); ++it )
			e.add_Argument( *it );
	}
	else
		Grounded( &e, &I );

	if ( stages )
		cerr << "\te: " << e << endl << "\tI: " << I << endl; 

	// Convert Grounded's output into a Labelling
	Labelling first = Labelling();
	for ( SetArgumentsIterator it = e.begin(); it != e.end(); ++it )
		first.add_label( *it, Labelling::lab_in );
	// TODO: Label I as undec and everything else as out?

	if ( stages )
		cerr << "\tFirst Labelling: " << *(first.inargs()) << endl;

	// Add the Labelling to the solutions
	this->labellings.push_back( first );

	if ( I.empty() )
	{
		if ( debug )
			cerr << "\tNo unlabelled arguments left. Returning.\n";

		return;
	}

	// Generate G restricted to I
	AF* restricted = new AF();
	this->af->restrictTo( &I, restricted );
	this->af = restricted;

	if ( stages )
		cerr << "\tNew AF ( restricted ):\n" << this->af->toString() << endl;

	// Structure used in SCCSSEQ
	this->initDFSAF();

	if ( debug )
		cerr << "\tDFSAF structure initialized\n";

	// Calculate the Strongly Connected Components
	list<SetArguments*> S = SCCSSEQ();

	if ( stages )
		for ( list<SetArguments*>::iterator it = S.begin(); it != S.end(); ++it )
			cerr << "\tSCC: " << **it << endl;

	for ( list<SetArguments*>::iterator aSCC = S.begin(); aSCC != S.end(); ++aSCC )
	{
		vector<Labelling> newLabellings = vector<Labelling>();

		for ( vector<Labelling>::iterator aLabelling = this->labellings.begin();
				aLabelling != this->labellings.end(); ++aLabelling )
		{
			Preferred p = Preferred();
			SetArguments O = SetArguments();
			I = SetArguments();							// I already exists..

			boundcond( *aSCC, (*aLabelling).inargs(), &O, &I );

			if ( stages )
				cerr << "\tO: " << O << endl << "\tI: " << I << endl;

			if ( O.empty() )
			{
				if ( debug )
					cerr << "\tGoing to call prefSAT.\n";

				AF restricted = AF();
				this->af->restrictTo( *aSCC, &restricted );

				// Avoid calling prefSAT if the two parameters are the same
				// and the size is 2 or less:
				// 	0:	out = { {} }
				//	1:	out = { {singlet} }
				//	2:	out = { {singlet}, {singlet} }
				if ( (*aSCC)->cardinality() <= 2 && *( restricted.get_arguments() ) == I )
				{
					if ( debug )
						cerr << "\t\tNo need to call prefSAT.\n";

					// Empty: no extensions => null iteration
					// Singlet: still have to iterate over I
					// Two element: Mutually exclusive arguments
					for ( SetArgumentsIterator it = (*aSCC)->begin();
							it != (*aSCC)->end(); ++it )
					{
						Labelling* temp = new Labelling();
						temp->add_label( *it, Labelling::lab_in );
						p.labellings.push_back( *temp );
					}
				}
				else
				{
					// prefSAT problem: the nodes in I are different from
					// the nodes in the restricted AF.
					// Temporary solution: rebuild I.
					I.adaptTo( &restricted );

					if ( debug )
						cerr << "\t\tCalling prefSAT.\n";

					p.prefSAT( &restricted, &I );
				}
			}
			else
			{
				if ( debug )
					cerr << "\tGoing to call pref.\n";

				SetArguments restriction = SetArguments();
				(*aSCC)->setminus( &O, &restriction );

				if ( restriction.cardinality() <= 1 && restriction == I )
				{
					if ( debug )
						cerr << "\t\tNo need to call pref.\n";

					// Empty: no extensions => null iteration
					// Singlet: still have to iterate over I
					for ( SetArgumentsIterator it = I.begin();
							it != I.end(); ++it )
					{
						Labelling* temp = new Labelling();
						temp->add_label( *it, Labelling::lab_in );
						p.labellings.push_back( *temp );
					}
				}
				else
				{
					if ( debug )
						cerr << "\t\tCalling pref.\n";

					AF restricted = AF();
					this->af->restrictTo( &restriction, &restricted );
					
					// The same as above for prefSAT.
					I.adaptTo( &restricted );

					p.pref( &restricted, &I );
				}
			}

			// prefSAT doesn't put newline at the end of its output...
			if ( debug )
				cerr << endl;

			// Create the new Labellings
			// by merging the current Labelling with every Labelling found
			for ( Preferred::iterator EStar = p.begin(); EStar != p.end(); ++EStar )
			{
				if ( debug )
					cerr << "\t\tFound " << *((*EStar).inargs()) << endl;

				// Rebuild the Labelling using the original Arguments
				// Not doing so could cause problems to next boundconds
				(*EStar).adaptTo( theAF );

				(*aLabelling).clone( &(*EStar) );

				if ( debug )
					cerr << "\t\t\tCreated: " << *((*EStar).inargs()) << endl;

				// Add it to the labellings found
				newLabellings.push_back( *EStar );
			}
		}
		
		// The generated Labellings are the new Labellings
		this->labellings.assign( newLabellings.begin(), newLabellings.end() );
	}
}

