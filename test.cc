//Example of analysis a shower map using G4ShowerMap
//This is a unit-test i.e. does not depend on G4 and 
//it is not representative of a real use-case
//It exercises the entire higher-level interface and 
//checks the output of each step against the expected
//value. If the test fails the program is interrupted

#ifndef UNITTESTING
#erorr Recompile with -DUNITTESTING option
#endif

#include <iostream>
#include "G4ShowerMap.hh"

//Utility macro to check if test success, if not print a message and abort application
#define TEST( cond , msg ) if (! (cond) ) { std::cout<<"Error at line: "<<__LINE__<<" ::::"<<msg<<std::endl; abort(); }

#include <iostream>
#include <sstream>
#include <cmath>


//For testing define some particles  
namespace {
  G4ParticleDefinition electron = "e-";
  G4ParticleDefinition positron = "e+";
  G4ParticleDefinition proton   = "p";
}

int main(int,char**) {
  std::cout<<"Testing G4ShowerMap, if any test fails a message is issued and programs stops"<<std::endl;

  //Get singleton
  G4ShowerMap::Analysis* instance = G4ShowerMap::Analysis::Instance();

  //Populate the container with some particles.
  //Just some numbers and combinations.
  //This corresponds to the following "shower":
  //   1(e-)
  //     |
  //     +-> 2(e-)
  //           |
  //           +->3(e+)
  //           +->4(p)
  //           |    |
  //           |    +->6(p)
  //           |    +->7(e-)
  //           +->5(p)
  //           |    |
  //           |    +->8(e+)
  //           +->9(p)
  //
  //                     Id, parent, type,      a value
  instance->AddSecondary( 1 , 0 ,    &electron , 0.1 );
  instance->AddSecondary( 2 , 1 ,    &electron , 0.2 );
  instance->AddSecondary( 3 , 2 ,    &positron , 0.3 );
  instance->AddSecondary( 4 , 2 ,    &proton, 0.4 );
  instance->AddSecondary( 5 , 2 ,    &proton, 0.5 );
  instance->AddSecondary( 6 , 4 ,    &proton, 0.6 );
  instance->AddSecondary( 7 , 4 ,    &electron, 0.7 );
  instance->AddSecondary( 8 , 5 ,    &positron, 0.8 );
  instance->AddSecondary( 9 , 2 ,    &proton, 0.9 );

  //Dump the shower info to screen
  std::cout<<"Test Shower map is:\n"<<*instance<<std::endl;

  //Start testing interfaces.
  //First basic interfaces

  //Check if an id exists and select it as current to be analysed
  TEST( instance->Exists(3) , "Cannot find 3");
  instance->Select(3);

  //Should be valid and correct
  TEST( instance->CurrentValid()==1, "Not valid current");
  TEST( instance->GetCurrentId()==3, "Not correct selection");
  TEST( instance->Data()==0.3,"Not valid data");
  TEST( instance->SumChildren()==0,"Not valid sum children");
  TEST( instance->CurrentValid()==1, "Not valid current");
  TEST( instance->GetCurrentId()==3, "Not correct selection");
  TEST( fabs(instance->SumSiblings()-2.1)<0.000001, "Not valid SumSiblings");
  TEST( instance->CurrentValid()==1, "Not valid current");
  TEST( instance->GetCurrentId()==3, "Not correct selection");

  instance->Select( 4 );
  TEST( instance->CurrentValid()==1, "Not valid current");
  TEST( instance->GetCurrentId()==4, "Not correct selection");
  TEST( instance->Data()==0.4,"Not valid data");
  TEST( instance->CurrentValid()==1, "Not valid current");
  TEST( instance->GetCurrentId()==4, "Not correct selection");
  TEST( fabs(instance->SumChildren()-1.3)<0.0000001,"Not valid sum children");
  TEST( instance->CurrentValid()==1, "Not valid current");
  TEST( instance->GetCurrentId()==4, "Not correct selection");
  TEST( instance->SumSiblings()==2.1, "Not valid SumSiblings");
  TEST( instance->CurrentValid()==1, "Not valid current");
  TEST( instance->GetCurrentId()==4, "Not correct selection");
  TEST( fabs(instance->SumBranch()-1.7)<0.0000001, "Not correct branch sum");

  instance->Select(1);
  TEST( fabs(instance->SumBranch()-4.5)<0.0000001,"Not correct branch sum");
  instance->Select(9);
  TEST( fabs(instance->SumBranch()-0.9)<0.0000001,"Not correct branch sum");
  instance->Select(8);
  TEST( fabs(instance->SumParent()-0.8)<0.00000001,"Not correct sum parent");

  //Now testing higher-level interfaces, specify ID of particle to be analysed

  //create a filter that returns true only if particle type is electron
  G4ShowerMap::conditions::ptype elefilter(&electron);
  instance->Select(1);
  //Check sum of descendent electrons
  TEST( fabs(instance->SumBranch(elefilter)-1.)<0.000001,"Not corect sum branch w/ filter");

  //Check that current particle has an electron as ancestor
  int id = -1;
  TEST( ! instance->HasParent( id , elefilter ) , "No parent type");
  instance->Select(8);
  TEST( instance->HasParent( id , elefilter ) , "No parent type");
  TEST( id==2 , "Wrong parent id");

  //Update value 
  instance->Select(4);
  instance->UpdateCurrent( 4.34 );
  TEST( fabs(instance->Data()-4.34)<0.000001,"Not updated correctly");

  //Check type of a particle
  TEST(instance->Matches(2,elefilter),"Not recongnized as electron");
  TEST(!instance->Matches(3,elefilter),"Recongnized as electron");
  //Check if a parent is an electron and get this parent id
  int pid = 0;
  bool result = instance->ParentMatches(3,pid,elefilter);
  TEST( result && pid==2 ,"Parent not recognized as electron");
  result = instance->ParentMatches(8,pid,elefilter);
  TEST( result && pid==2 , "Parent not recongnized as electron");
  G4ShowerMap::conditions::ptype posifilter(&positron);
  pid = -1;
  //This should fail
  result = instance->ParentMatches(8,pid,posifilter);
  TEST( !result && pid==-1 , "Parent recognizes as positron");

  //Retrieve data associated to id w/ and w/o filter/
  //
  double value = 0.;
  result = instance->GetValue( 4 , value );
  TEST( result && abs(value-4.34)<0.0000001,"Not correct value");
  result = instance->GetValue( 2 , value , elefilter);
  TEST( result && abs(value-0.2)<0.0000001,"Not correct value");
  value = 0.;
  result = instance->GetValue( 4, value, elefilter);
  TEST( !result && value==0,"Not correct value");

  //Transverse parents' tree
  result = instance->GetSumParents( 4 , value );
  TEST( result && abs(value-0.3)<0.0000001,"Not correct value");
  result = instance->GetSumParents(6 , value , elefilter);
  TEST( result && abs(value-0.3)<0.0000001,"Not correct value");
  value = 0;
  result = instance->GetSumParents( 4 , value , posifilter );
  TEST( !result && value==0 , "Not correct value");

  //Get sum of secondaries
  result = instance->GetSumSecondaries( 4 , value );
  TEST( result && abs(value-1.3)<0.00000001,"Not correct vlaue for children 1");
  value = 0;
  result = instance->GetSumSecondaries( 4, value , elefilter );
  TEST( result && abs(value-0.7)<0.0000001,"Not correct value for children 2 ");
  value = -1;
  result = instance->GetSumSecondaries( 4, value, posifilter );
  TEST( !result && value == -1 , "Not correct value for children 3");

  //Retrieve IDs of secondaries
  std::vector<int> ids;
  result = instance->GetSecondariesIds(4,ids);
  TEST( result && ids.size()==2 && ids[0]==6&&ids[1]==7 , "Wrong secondaries");
  ids.clear();
  G4ShowerMap::conditions::ptype pfilter(&proton);
  result = instance->GetSecondariesIds(2,ids,pfilter);
  TEST( result && ids.size()==3 &&ids[0]==4&&ids[1]==5&&ids[2]==9,"Wrong secondaries");
  ids.clear();
  result = instance->GetSecondariesIds(5,ids,pfilter);
  TEST( !result && ids.size()==0 , "Wrong secondaries");

  //Get the "heads" e.g. the most ancient ancestor of a partial shower that does *not* anymore have
  //a further ancestor matching the condition. With forceaccept, we get the very head of shower
  std::vector<int> heads;
  result = instance->GetHeads( heads, G4ShowerMap::forceaccept() );
  TEST( result && heads.size()==1,"Not correct size of heads");
  TEST( heads[0]==1,"Not correct value of heads");
  heads.clear();
  result = instance->GetHeads( heads, pfilter );
  TEST( result && heads.size()==3,"Not correct size of heads");
  TEST( heads[0]==4&&heads[1]==5&&heads[2]==9,"Not correct value of heads");

  //Check use of iterators, The type(iterator)=std::pair<int,G4TrackData<G4double> >*
  TEST(instance->First()->first==1,"Not correct first element");
  TEST((--instance->End())->first==9,"Not correct last element");
  G4ShowerMap::Analysis::const_iterator it = instance->First();

  //Reset content of 4 (so later it's easier to check content via iterators)
  result = instance->Update( 4, 0.4);
  TEST( result , "Wrong update");
  result = instance->Update( 4 , 1000. , elefilter);
  TEST( !result , "Wrong update");
 
  int idx=1;
  for ( ; it != instance->End() ; ++it ) {
    TEST( it->first==idx ,"Wrong iterator");
    std::pair<int,G4ShowerMap::Analysis::struct_type> info = G4ShowerMap::Analysis::GetInfo( it );
    TEST( info.first==idx, "Wrong info from iterator");
    TEST( abs(info.second.data-idx/10)<0.0000001 , "Wrong info from iterator");
    ++idx;
  }

  //Empty map (e.g. prepare for new event)
  TEST( instance->Size()==9,"Wrong size of container");
  instance->Clear();
  TEST( instance->Size()==0, "Wrong size of container");
  std::cout<<"Nothing between arrows:->"<<*instance<<"<-"<<std::endl;
  std::cout<<"END"<<std::endl;
  return 0;
}
