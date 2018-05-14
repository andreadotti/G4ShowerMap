#ifndef G4SHOWERMAP_HH
#define G4SHOWERMAP_HH

#include <map>
#include <ostream>
#include <vector>

//If this is defined, use "fake" internals of G4,
//Used for testing w/o G4
#ifdef UNITTESTING
#  include <string>
   using std::string;
#  define G4double double
#  define G4ThreadLocal
#  define G4ParticleDefinition string 
#else  //UNITTESTING
#  include "G4ParticleDefinition.hh"
#endif //UNITTESTING

#include "G4ShowerMapInternals.hh"

// Three entities are defined in this namespace:
// G4TrackData<T>     template class containing information about a specific particle
// TShowerMap<T>      container of G4TrackData<T> instances. User should not use directly
//                    this class, but instead the derived class that implements higher level
//                    utilities and methods.
//                    Requirement for T is to implement a meaning default constructor and 
//                    support the increament operator += (e.g. T=G4double)
// conditions::ptype  functor to select particles based on their species
// Analysis           concrete implementation of a TShiowerMap<G4double>

namespace G4ShowerMap { 

  //struct identifying a track based on its particle type
  template<class T>
  struct G4TrackData {
    G4ParticleDefinition* pdef;
    T data;
  };
  //Helper function to print out 
  template<class T>
  std::ostream& operator<<(std::ostream& os , const G4TrackData<T>& el ) {
#ifdef UNITTESTING
    os<<"("<<*(el.pdef)<<","<<el.data<<")";
#else
    os<<"("<<el.pdef->GetParticleName()<<","<<el.data<<")";
#endif
    return os;
  }

  //Base template class that builds a map of a quantity of type T
  //associatying a particle definition
  //Note that user should use concrete class (see later)
  //  There are two requirements on T: it should have a meaningful 
  //  default constructor T() and it should implement the  T& operator+=(const T&)
  template<class T>
  class TShowerMap : public internal::Container<G4TrackData<T>,int> {
    typedef internal::Container<G4TrackData<T>,int> baseclass;
  public:
    typedef conditions::basecondition<G4TrackData<T> > conditionbase;
    typedef conditions::dummy<G4TrackData<T> > alwaysTrue;

    virtual ~TShowerMap() {}
    //If current selection is valid and condition is met,
    // returns value
    T Data( const conditionbase& cond = alwaysTrue() ) { 
      T result = T();
      if ( baseclass::CurrentValid() ) {
	const typename baseclass::value_type& _data = baseclass::GetData();
	if ( cond(_data) ) result += _data.data;
      }
      return result;
    }

    //Iterate over all siblings, sum values when condition is met
    T SumSiblings( const conditionbase& cond = alwaysTrue() ) {
      T result = T();
      if ( baseclass::CurrentValid() ) {
	const typename baseclass::id_type& cid = baseclass::GetCurrentId();
	//I need to go back to the first child. Precondition to have siblings is to have a parent
	if ( ! baseclass::SelectParent() ) {
	  result += Data(cond); //Just this one
	} else {
	  baseclass::SelectFirstChild(); //this cannot fail, at least there is one, itself
	  result += Data(cond);
	  while (  baseclass::SelectNextSibling() )  {
	    result += Data( cond );
	  }
	}
	baseclass::Select(cid);
      }
      return result;
    }

    //Iterate over all direct children, sum values when conidtions is met
    T SumChildren( const conditionbase& cond = alwaysTrue() ) {
      T result = T();
      if ( baseclass::CurrentValid() ) {
	const typename baseclass::id_type& cid = baseclass::GetCurrentId();
	baseclass::SelectFirstChild();
	result += SumSiblings(cond);
	baseclass::Select(cid);
      }
      return result;
    }
    //Recursively Iterate over all children following descendent, sum values when condition
    //is met
    T SumBranch( const conditionbase& cond = alwaysTrue() ) {
      T result = T();
      SumBranch( result , cond );
      return result;
    }

    //Sum data of all parents up to the root
    T SumParent( const conditionbase& cond = alwaysTrue() ) {
      T result = T();
      if ( baseclass::CurrentValid() ) {
	const typename baseclass::id_type& cid = baseclass::GetCurrentId();
	while ( baseclass::SelectParent() ) result += Data( cond );
	baseclass::Select(cid);
      }
      return result;
    }

    //Returns true if a parent matches the condition, the id of the first matching
    //parent is available throught the parameter id
    bool HasParent( typename baseclass::id_type& id ,  const conditionbase& cond = alwaysTrue() ) {
      bool result = false;
      if ( baseclass::CurrentValid() ) {
	const typename baseclass::id_type& cid = baseclass::GetCurrentId();
	while ( baseclass::SelectParent() ) { 
	  result = cond(baseclass::GetData());
	  if ( result ) { id = baseclass::GetCurrentId(); break; }
	}
	baseclass::Select(cid);
      }
      return result;
    }

    //Change the value
    void UpdateCurrent( const T& val ) {
      typename baseclass::value_type _data = baseclass::GetData();
      _data.data = val;
      baseclass::UpdateCurrentValue( _data );
    }
  protected:    
    //Recursively Iterate over all children following descendent, sum values when condition
    //is met
    void SumBranch( T& result , const conditionbase& cond = alwaysTrue() ) {
      if ( baseclass::CurrentValid() ) {
	const typename baseclass::value_type& _data = baseclass::GetData();
	const typename baseclass::id_type& cid = baseclass::GetCurrentId();
	//std::cout<<cid<<std::endl;
	if ( baseclass::SelectFirstChild() ) {
	  do { 
	    SumBranch( result, cond );
	  } while ( baseclass::SelectNextSibling() );
	} 
	//Get back to the correct level and update result
	baseclass::Select(cid);
	result += Data(cond);
      }
    }
    TShowerMap() {}
  private:
    //disable copy constructor and assignement operators
    TShowerMap(const TShowerMap<T>& rhs);
    TShowerMap<T>& operator=(const TShowerMap<T>& rhs);
  };

  //A functor to select particles based on 
  //the particle type
  namespace conditions {
    struct ptype : public basecondition< G4TrackData<G4double> > {
    private:
      G4ParticleDefinition* m_reference;
    public:
      ptype( G4ParticleDefinition* pd ) : m_reference(pd) {}
      bool operator()( const G4TrackData<G4double>& d ) const { return (d.pdef == m_reference);} 
    };
    typedef basecondition<G4TrackData<G4double> > conditionbase; 
  }
  //Define an helper that always returns true
  typedef conditions::dummy<G4TrackData<G4double> > forceaccept;


  //Concrete class implementing singleton pattern and using a G4double
  //to store the quantity
  //Here higher level methods are defined to help in selecting 
  class Analysis : public TShowerMap<G4double> {
    typedef TShowerMap<G4double> baseclass;
  public:
    typedef baseclass::value_type struct_type; //G4TrackData<G4double>
    static Analysis* Instance();
    //Clear map content.
    void Clear() { baseclass::Clear(); }
    //Add a secondary. If parent_id is zero, this is a primary
    void AddSecondary( int id , int parent_id , G4ParticleDefinition* pd , G4double value );
    //Update values of a particle with given id
    bool Update( int id , G4double value , const conditions::conditionbase& cond = forceaccept() );

    //All these methods return true if and condition is met, otherwise false. If id does not 
    //exist also return false
    //An optional condition cab be passed, by default condition always matches
    bool Matches( int id , const conditions::conditionbase& cond = forceaccept() );
    //A perent up in hierarchy matches
    bool ParentMatches( int id , int& parentid , const conditions::conditionbase& cond = forceaccept() );
    //The value associated with id
    bool GetValue( int id , double& result , const conditions::conditionbase& cond = forceaccept() );
    //Sum of values of parents up matching condion
    bool GetSumParents( int id , double& result , const conditions::conditionbase& cond = forceaccept() );
    //Sum of values of direct secondaries
    bool GetSumSecondaries( int id , double& result , const conditions::conditionbase& cond = forceaccept() );
    //All ids of the secondaries matching condition
    bool GetSecondariesIds( int id, std::vector<int>& result, const conditions::conditionbase& cond = forceaccept() );

    //Retrieve heads ids: e.g. the most ancient ancestor of a partial shower that does *not* anymore have
    //a further ancestor matching the condition.
    //Returns false if none is found
    bool GetHeads(  std::vector<int>& result , const conditions::conditionbase& cond );

    //Returns iterator to first element
    typedef  baseclass::map_type::const_iterator const_iterator;
    const_iterator First() const { return m_map.begin(); }
    //Returns iterator to past-last element
    const_iterator End() const { return m_map.end(); }
    //Returns a pair with id and G4TrackData structure for the current iterator.
    static std::pair<int,struct_type > GetInfo( const const_iterator& it );
    size_t Size() const { return m_map.size(); }
    
  };
  
} //End G4ShowerMap namespace

#endif //G4SHOWERMAP_HH
