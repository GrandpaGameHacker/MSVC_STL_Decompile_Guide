/*
=============== A SMALL GUIDE TO DECOMPILING AND RECOGNIZING MSVC STL TYPES (x86) =============================
This documents how some common MSVC stl types look like in memory
and how they look in decompilation
results may vary depending on decompiler
and instruction set and compiler version/STL version
I will write a guide on x64 at some point.

first off When dealing with templated types like
vector, map, list etc. its important to remember
in your decompilation to create a new type for each templated type
since modern decompilers do not support templates

so say std::string vector would be

struct std::vector_string{
	std::string* start, *end, *max;
};

if the vector template uses a pointer like <MyClass*>
then we use double pointers
struct vector_pMyClass
{
	MyClass **start, **end, **max;
};

secondly, reading the STL template code is helpful
https://github.com/microsoft/STL/blob/main/stl/inc/
*/

 // the union variable pstr is used when a string is too long to store in local string (lstr)
struct string
{
	union
	{
		char* pstr;
		char lstr[16];
	}
	DWORD size, max; // max is always set to sizeof(lstr)-1
};

/*
Hints for a string in decompilation
v1[0] = 0;
v1[4] = 0;
v1[5] = 15; // maximum characters before it gets allocated to heap

or if decompiler does not detect them being one variable

v34 = 0;
v25 = 0;
v26 = 15;

hardcoded strings often get constructed like this
sub_4017E0(v24, "String", 6); // allocate string "String" with size 6
empty strings will use a pointer to zeroed memory in a data section.
sub_4017E0(v24, DWORD_XXXXXXXX, 0);
*/

template<typename T> // vector is a very simple container
struct vector<T>
{
	T *start, *end, *max;
};

/*hints for a vector type are
v1[0] = 0;
v1[1] = 0;
v1[2] = 0;
// and in called functions/local functions
  v3 = (v1 - *v2) / sizeof(T); //gets current size of vector ie vector.size();
*/

template<typename K, typename V>
struct map_node<K, V> //STL uses a red black tree implementation
{
	map_node *left, *right, *parent;
	bool bIsFirstNode, bColor;
	K key;
	V value;
};

struct map
{
	map_node* head;
	DWORD size;
};

/*hints for a map type are

v1[0] = 0; // head node pointer
v1[1] = 0;
v4 = operator new(sizeof(map_node));
or
v34 = 0;
v35 = 0;

// Then create first node
v4 = operator new(sizeof(map_node));
*v4 = v4; //set the nodes to be pointing to this node (left node)
*(v4 + 4) = v4; // right node
*(v4 + 8) = v4; // parent node
*(v4 + 12) = 257; //0x0101 setting bIsFirstNode and bColor to true
v1[0] = v4; // head_node ptr is set to the first node;
*/


template <typename T>
struct list_node //doubly linked list
{
	list_node* forward;
	list_node* back;
	T value;
};

struct list
{
	list_node* head;
	size_t size;
};

/* hints for list type are
  if ( v49[1] == 357913941 ) // 357913941 is the max elements for a std::list<float> in 32bit
    std::_Xlength_error("list too long"); // call to Xlength_error like this is a sure sign

v29 = operator new(sizeof(list_node));
  v29[2] = 1; // add a new list item with value 1
  ++v49[1]; // increase list size
*/


// bitset is a little complex but it uses ulong/ulonglong under the hood
// its also partly based on a basic_string type but we wont worry about that.
template<typename T, size_t nBits>
struct bitset
{
	unsigned long bits[nBits/sizeof(unsigned long)/8]
};

// or

struct bitset
{
	unsigned long long bits[nBits/sizeof(unsigned long long)/8]
};

/*the template chooses long or long long based on the size of the bitset (nBits)
 for optimization reasons.

 hints for bitset include 
_Xout_of_range("invalid bitset position");
_Xout_of_range("invalid bitset<N> position"); // older msvc
_Xoverflow_error("bitset overflow");
_Xinvalid_argument("invalid bitset char");

if you find a function that takes a bitset as an argument
you can grab the size from code like this

 for ( i = a2; v4; i = v4 )
  {
    if ( v5 >= 256 ) // maximum bits
      break;
    if ( (v4 & 1) != 0 )
    {
      v6 = 0;
      v7 = &v2[2 * (v5 >> 6)];
*/
