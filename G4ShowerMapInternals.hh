#ifndef G4SHOWERMAPINTERNALS_HH
#define G4SHOWERMAPINTERNALS_HH

#include <map>
#include <ostream>

//Namespace for G4 application use
namespace G4ShowerMap { 

  //This contains general data structures
  //not depending on G4
  namespace internal {  

    //Forward declarations: the container class
    template <class T,class ID> class Container;

    /*A Node in our data structure
      A node is an element with some relations:
      It has a reference to a parent, it has a
      reference to the first child and to its level next sibpling
          Parent -> ...
            ^
            |
            v
        FirstChild -> NextSibling -> NextSibling -> NULL
            |             ^              ^  
            |             |              |
            v             v              v
          NULL           ...            ...
     A Node is identified by an ID and it contains DATA.
     Note Nodes cannot be accessed/created directly but
     only via a container */
    template <class T,class ID=int>
    struct Node {
      typedef T value_type;
      typedef ID id_type;
      friend class Container<value_type,id_type>;
      friend std::ostream& operator<<(std::ostream& os , const Node<T,ID>& e) {
	os << "id: "<<e.m_id <<" = "<< e.m_data 
	   << " ; parent id: "<< (e.p_parent?e.p_parent->m_id:0)
	   <<" ; First Child id: "<< (e.p_firstChild?e.p_firstChild->m_id:0)
	   << " ; Next Sibling id: "<< (e.p_nextSibling?e.p_nextSibling->m_id:0);
	return os;
      }
    private:
      // Constructors
      Node () : p_parent(0), p_firstChild(0) , p_nextSibling(0) {}
      Node (ID id , T data) : m_id(id), m_data(data), p_parent(0), p_firstChild(0) , p_nextSibling(0) { }
      Node (ID id , T data, Node<T,ID>* parent) : m_id(id) , m_data(data), p_parent(parent), p_firstChild(0) , p_nextSibling(0) {
	if ( p_parent->p_firstChild == 0 ) { p_parent->p_firstChild = this; }
	else {
	  Node<T,ID>* next = p_parent->p_firstChild;
	  do { 
	    if ( next->p_nextSibling == 0 ) { 
	      next->p_nextSibling = this;
	      break;
	    }
	    next = next->p_nextSibling;
	  } while(next);
	}
      }
      
      //Data
      ID m_id;
      T m_data;
      Node<T,ID>* p_parent;
      Node<T,ID>* p_firstChild;
      Node<T,ID>* p_nextSibling;
      
      //Disable copy and assignement
      Node(const Node<T,ID>& rhs);
      Node<T,ID> operator=(const Node<T,ID>& rhs);
    };
    

    /* Container class
       It's a collection of Nodes<T,ID>.
       Nodes information can be accessed via IDs and the structure can be navigated
     */
    template <class T, class ID=int>
    class Container {
      friend std::ostream& operator<<(std::ostream& os , const Container<T,ID>& ct) {
	for ( typename Container<T,ID>::map_type::const_iterator it = ct.m_map.begin() ; 
	      it != ct.m_map.end() ; ++it ) os<<*(it->second)<<"\n";
	return os;
      }
    public:
      typedef std::map<ID,Node<T,ID>* > map_type;
      typedef typename Node<T,ID>::value_type value_type;
      typedef typename Node<T,ID>::id_type id_type;
      
      //Manipulate container
      void AddOne( id_type id , id_type parent , const value_type& data ) {
	typename map_type::const_iterator parentIt = m_map.find(parent);
	if ( parentIt != m_map.end() ) { 
	  m_map[id] = new Node<T,ID>(id,data,parentIt->second);
	}
	else {
	  m_map[id] = new Node<T,ID>(id,data);
	}
      }
      //Empty container and delete nodes
      void Clear() { 
	typename map_type::iterator it = m_map.begin();
	while ( it!=m_map.end() ) {
	  Node<T,ID>* e = it->second;
	  m_map.erase(it);
	  delete e;
	  it = m_map.begin();
	}
      }
      virtual ~Container() { Clear(); }
      
      //Analyse container
      bool Exists( const id_type& id ) const { return (m_map.find(id) != m_map.end()); }
      void Select( const id_type& id ) { p_current = m_map.find(id)->second; }
      const value_type& GetData() const { return p_current->m_data; }
      const id_type& GetCurrentId() const { return p_current->m_id; }
      bool SelectParent() { return p_current = p_current->p_parent; }
      bool SelectFirstChild() { return p_current = p_current->p_firstChild; }
      bool SelectNextSibling() { return p_current = p_current->p_nextSibling; }
      bool CurrentValid() const { return (p_current != 0); }
      void UpdateCurrentValue( const value_type& newval ) { p_current->m_data = newval; }
    protected:
      static T GetData( const typename map_type::const_iterator& it ) { return it->second->m_data; }
      std::map<ID,Node<T,ID>* > m_map;
      Node<T,ID>* p_current;
    };

  } // End namespace internal

  //Conditions are functors that allow to select a Node in a container
  namespace conditions {
    template <class T>
    struct basecondition {
      virtual bool operator()(const T&) const =0;
    };
    
    template <class T>
    struct dummy : public basecondition<T> {
      bool operator()(const T&) const { return true; }
    };
  }

}//End Namespace G4ShowerMap

#endif //G4SHOWERMAPINTERNALS_HH
