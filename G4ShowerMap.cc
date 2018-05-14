#include "G4ShowerMap.hh"

G4ShowerMap::Analysis* G4ShowerMap::Analysis::Instance() {
  static G4ThreadLocal Analysis* analysis = 0;
  if ( analysis == 0 ) analysis = new Analysis;
  return analysis;
}

bool G4ShowerMap::Analysis::GetHeads( std::vector<int>& result , const conditions::conditionbase& cond ) {
  //This algorithm should be optimized, for example skipping when I analyse twice the same branch
  bool found = false;
  baseclass::map_type::iterator it = m_map.begin();
  std::map<int,bool> helper;//This is used to speedup algorithm
  while ( it != m_map.end() ) {
    int idx = -1; //Default ids
    baseclass::Select( it->first );//This is for sure valid
    do { 
      int thisid=baseclass::GetCurrentId();
      if ( helper.find(thisid)!=helper.end() ) { //We already know this is a head, skip
	idx = -1;
	break; 
      }
      if ( cond( baseclass::GetData() ) ) { 
	idx = thisid; //This parent matches condition is a candidate
      }
    } while ( baseclass::SelectParent() ); //Go up one level
    //Here I've ascended all tree, check last parent matching condition, if valid add it to the list of results
    if ( idx > -1 ) { 
      result.push_back( idx );
      helper[idx]=true;
      found = true;
    }
    ++it; //Go to next particle
  } 
  return found;
}

bool G4ShowerMap::Analysis::Matches( int id , const conditions::conditionbase& cond ) {
   if ( baseclass::Exists(id) ) {
      baseclass::Select(id);
      const baseclass::value_type& _data = baseclass::GetData();
      if ( cond(_data) ) return true;
  }
  return false; 
}

bool G4ShowerMap::Analysis::GetValue( int id , double& result, const G4ShowerMap::conditions::conditionbase& cond  ) {
  if ( baseclass::Exists(id) ) {
      baseclass::Select(id);
      result = baseclass::Data(cond);
      return cond(baseclass::GetData());
  }
  return false;
}

bool G4ShowerMap::Analysis::Update( int id , G4double value , const conditions::conditionbase& cond ) {
  if ( baseclass::Exists(id) ) {
      baseclass::Select(id);
      if ( cond(baseclass::GetData()) ) {
	baseclass::UpdateCurrent( value );
	return true;
      }
  }
  return false;
}

bool G4ShowerMap::Analysis::ParentMatches( int id , int& parentid , const conditions::conditionbase& cond ) {
    if ( baseclass::Exists(id) ) {
      baseclass::Select(id);
      return baseclass::HasParent( parentid , cond ); //Not at all optimized, goes thought tree twice!
    }
    return false;
}


bool G4ShowerMap::Analysis::GetSumParents( int id , double& result, const G4ShowerMap::conditions::conditionbase& cond ) {
  if ( baseclass::Exists(id) ) {
    baseclass::Select(id);
    result = baseclass::SumParent( cond );
    int cid = 0;
    return baseclass::HasParent( cid , cond ); //Not at all optimized, goes thought tree twice!
  }
  return false;
}

bool G4ShowerMap::Analysis::GetSumSecondaries( int id , double& result , const G4ShowerMap::conditions::conditionbase& cond ) {
  bool retval = false;
  if ( baseclass::Exists(id) ) {
    baseclass::Select(id);
    if ( baseclass::SelectFirstChild() ) {
      do { //Loop on secondaries
	const baseclass::value_type& _data = baseclass::GetData();
	const bool selectme = cond(_data);
	if ( selectme ) {
	  retval = true;
	  result += _data.data;
	}
      } while ( baseclass::SelectNextSibling() );
    }
  }
  return retval;
}

std::pair<int,G4ShowerMap::Analysis::struct_type> G4ShowerMap::Analysis::GetInfo( const baseclass::map_type::const_iterator& it ) { 
  return std::make_pair(it->first,baseclass::GetData(it) ); 
}

void G4ShowerMap::Analysis::AddSecondary( int id, int parent_id, G4ParticleDefinition* pd , G4double value ) {
  baseclass::value_type node = {pd,value};
  baseclass::AddOne( id , parent_id , node );
}

bool G4ShowerMap::Analysis::GetSecondariesIds( int id, std::vector<int>& result, const conditions::conditionbase& cond ) {
  bool retval = false;
  if ( baseclass::Exists(id) ) {
    baseclass::Select(id);
    if ( baseclass::SelectFirstChild() ) {
      {
	do {
	  if ( cond(baseclass::GetData() ) ) { retval=true; result.push_back( baseclass::GetCurrentId() ); }
	} while ( baseclass::SelectNextSibling() );
      }
    }
  }
  return retval;
}
