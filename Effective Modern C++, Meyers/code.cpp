/*
================================
 Effective Modern C++. 
 42 specific ways to improve your use of C++11 and C++14.
 
 Scott Meyers
 2014 
================================
*/

/*
================
 Deducing Types
 
 There are three types of type deduction in C++11: one for templates, 
 one for auto and one for decltype.
================
*/


// 1. UNDERSTAND TEMPLATE TYPE DEDUCTION

// For:

template<typename T>
void f(ParamType param);

f(expr);

int x = 27;
const int cx = x;
const int& rx = x;

// There are 3 scenarios:

// ParamType is a pointer or reference type
// In this case simply remove the reference or pointer part.

template<typename T>
void f(T& param); // param is a reference

f(x);	// T is int, param is int&
f(cx);	// T is const int, param is const int&
f(rx);	// T is const int, param is const int&

// if const is added to param, T no longer needs it, i.e.

template<typename>
void f(const T& param); // param is reference to const

f(x);	// T is int, param is const int&
f(cx);	// T is int, param is const int&

// ParamType is a universal reference

template<typename T>
void f(T&& param);	// param is a universal reference

f(x);	// x is lvalue, T is int& param is int&
f(cx);	// cx is lvalue, T is const int&, param is const int&
f(rx);	// rx is lvalue, T is const int&, param is const int&
f(27);	// 27 is rvalue, T is int, param is int&&

// ParamType is neither of the above

template<typename T>
void f(T param);

// This is copy by value, so T and param are generally the same.
// Here they are all int. Constness and volatileness are removed.

// Border cases include arrays and function pointers which decay to 
// raw pointers or references.


// 2. UNDERSTAND auto TYPE DEDUCTION

// auto type deduction works like template type deduction where
// auto is like T and variable is like ParamType. So additional modifiers
// are added to auto as they would be added to T. E.g.

auto x = 27;
const auto cx = x;
const auto& rx = x;

auto&& uref1 = x;	// int&
auto&& uref2 = cx;	// const int&
auto&& uref3 = 27;	// int&&

// auto assumes std::initializer_list with braced initializer (unlike templates)

auto x = {3, 4, 7};	// auto is initializer_list<int>

// Additionally in C++14 auto can be used for function return types and lambda parameters.


// 3. UNDERSTAND decltype

// decltype usually deducts the exact type, e.g.

const int i = 0;	// decltype(i) is const int
bool f(const Widget& w);	// decltype(w) is const Widget&, decltype(f) is bool(const Widget& w)
vector<int> v;	// decltype(v) is vector<int>, decltype(v[0]) is int&

// The primary use for decltype in C++11 is declaring function template return types
// which depend on the parameters.

template<typename Container, typename Index>
auto authAndAccess(Container&& c, Index i)->decltype(std::forward<Container>(c)[i])
{
	authenticateUser();
	return std::forward<Container>(c)[i];
}


// 4. KNOW HOW TO VIEW DEDUCED TYPES

// IDE Editors, compiler error messages, Boost TypeIndex library.
// All of which may be inaccurate.


/*
================
 auto
================
*/


// 5. PREFER auto TO EXPLICIT TYPE DECLARATIONS

// Requires less typing (e.g. iterators)

// Has to be initialized.

int x1;		// Potentially uninitialized
auto x2;	// Compiler error

// Can represent types known only to compilers

auto piggerPtrFunc = [](const std::unique_ptr<Widget>& p1, const std::unique_ptr<Widget>& p2) { return *p1 < *p2; }

// Ensures the right type

auto sz = v.size();	// unsigned could be 32-bit on 64 bit machine

// Easier enumeration

for (const auto& p : m) { ... }

// Good example:

std::unordered_map<std::string, int> m;

for (const std::pair<std::string, int>& p : m) { ... } // Here the std::string is really const in the container so the std::pair returned
																		// will be a copy created by compilers (inefficient)
																		

// USE EXPLICITLY TYPED INITIALIZER IDIOM WHEN auto DEDUCES UNDESIRED TYPES

// Sometimes auto fails to deduce the right type, e.g.
std::vector<bool> features(const Widget& w);
Widget w;
auto someFeature = features(w)[5];	// Now this returns std::vector<bool>::reference (a proxy class) and not bool 
													// (because vector<bool> uses a bitfield internally and C++ forbids references to bits)													
													// Another example of a proxy class can be Sum<Matrix, Matrix>
													
// To fix the problem use static_cast


/*
================
 Moving to Modern C++
================
*/


// DISTINGUISH BETWEEN () AND {} WHEN CREATING OBJECTS

// Braced initialization is the most widely usable initialization syntax which prevents narrowing conversions and is immune to C++ most vexing parse

Widget w1();	// Most vexing parse - declares a function
Widget w2{};	// Calls widget's ctor with no arguments

// During ctor overload resolution braced initializers are matched to std::initializer_list

// Choice between the two can make significant difference, e.g.

std::vector<int> v1(10, 42);	// vector with 10 elements
std::vector<int> v2{10, 42};	// vector with 2 elements


// PREFER nullptr to 0 and NULL

// Pretty obvious really, helps with overloads resolution


// PREFER ALIAS DECLARATIONS TO typedefs

using FP = void(*)(int, const std::string&);

// Easier to read
// Deal with nested structures
// Support templatization


// PREFER SCOPED ENUMS TO UNSCOPED ENUMS

enum class Color  : std::uint8_t { black, white, red };	// note enum type declaration plus std cross-platform types
auto white = false;	// works (no scope name conflict, as would be with normal enum)

// Can only be converted with a cast
// Can be forward declared


// PREFER DELETED FUNCTIONS TO PRIVATE UNDEFINED ONES

// Useful to implement uncopyable objects as well as to remove unwanted overloads (compile-time error vs link-time error)

bool isLucky(int number);
bool isLucky(char) = delete;	// Normally would convert to int
bool isLucky(bool) = delete;	// As above
bool isLucky(double) = delete;

// Aslo useful for pointer types, where a no real pointer may be passed (void or char/string)
template<>
void processPointer<void>(void*) = delete;	// This could not be dereferenced, incremented etc.
void processPointer<void>(char*) = delete;	// Typically a string

// Any function can be deleted


// DECLARE OVERRIDING FUNCTIONS override


class Base
{
public:
	virtual void mf1() const;
	virtual void mf2(int x);
	virtual void mf3() &;	// invoke mf3() if Base is an lvalue
	void mf4() const;
}

class Derived : public Base
{
public:
	virtual void mf1() ;
	virtual void mf2(unsigned int x);
	virtual void mf3() &&;	// invoke mf3() if Derived is an rvalue (e.g. makeDerived().mf3())
	void mf4() const;
}

// None of the functions will actually work as virtual, using override will generate compiler error

class Derived : public Base
{
public:
	virtual void mf1() override;
	virtual void mf2(unsigned int x) override;
	virtual void mf3() && override;	// invoke mf3() if Derived is an rvalue (e.g. makeDerived().mf3())
	void mf4() const override;
}

// Note that rvalue and lvalue objects can be treated differently


// PREFER const_iterators TO iterators

// C++11 make it easier to use const iterators with new cbegin() and cend() methods, e.g.
auto cit = std::find(values.cbegin(), values.cend(), 42);

// The values cannot be modified, but other operations like insert can still be used.

// A more generic approach is to create a non-member methods which can take any container
template <class C>
auto cbegin(const C& container)->decltype(std::begin(container))
{
	return std::begin(container);
}


// DECLARE FUNCTIONS noexcept IF THEY WON't EMIT EXCEPTIONS

// noexcept is more optimizable by the compiler and can be used conditionally
template <class T, size_t N>
void swap(T (&a)[N], T (&b)[N]) noexcept(noexcept(swap(*a, *b)));

// noexcept is a part of function interface (clients may depend on it)
// most functions are exception neutral rather than noexcept (i.e. they don't generate exceptions by may forward from others)


// 15. USE constexpr WHENEVER POSSIBLE

class Point{
public:
	constexpr Point(double x = 0, y = 0) noexcept : m_x(x), m_y(y) {}
	
	constexpr double xValue() const noexcept { return x; }
	constexpr double yValue() const noexcept { return y; }
	
private:
	double m_x, m_y;	
}

constexpr Point p1(1.2, 3.4);
constexpr Point p2(5.6, 7.8);

constexpr Point midpoint(const Point& p1, const Point& p2) noexcept
{
	return { (p1.xValue() + p2.xValue()) / 2	, (p1.yValue() + p2.yValue()) / 2 }
}

constexpr auto mid = midpoint(p1, p2);

// constexpr objects are const and initialized with values known during compilation


// MAKE const MEMBER FUNCTIONS THREAD SAFE

// Approach include using std::atomic and std::mutex

class Widget
{
public:
		// ...
		
		int magicValue() const
		{
			std::lock_guard<std::mutex> guard(m_mutex);
			
			if (cacheValid) return cachedValue;
			else
			{
					auto val1 = expensiveComputation1();
					auto val2 = expensiveComputation2();
					cachedValue = val1 + val2;
					cacheValid = true;
					return cachedValue;				
			}
		}
			
private:
	mutable std::mutex m_mutex;
	mutable int cachedValue;
	mutable bool cacheValid { false }; // note bracket initialization of default value
}


// UNDERSTAND SPECIAL MEMBER FUNCTION GENERATION

// When in doubt, use default keyword
// Move operations are generated only for classes lacking explicitly declared move operations, copy operations and a destructor
// Copy constructor is generated only for classes lacking an explicitly declared copy constructor and is deleted if a move operation is declared
// Member function templates never suppress generation of a special member functions, e.g.

class Widget
{
	template<typename T>
	Wisget(const T& rhs);	// construct Widget from anything, the default constructor will still be generated
}

// One reason to use default is that adding something to the class, may delete methods previously generated (e.g. logging destruction)



/*
================
 Smart Pointers
 
 Raw pointers are bad for multiple reasons. Use smart pointers instead.
================
*/

// 18. USE std::unique_ptr for exclusive ownership resource management

// Lightweight move only type
// Can use custom deleters, lambdas become handy

auto delWidget = [](Widget* pWidget)
{
	makeLogEntry(pWidget);
	delete pWidget;
}

template<typename... Ts>
std::unique_ptr<Widget, decltype(delWidget)> makeWidget(Ts&&... params)
{
	std::unique_ptr<Widget, decltype(delWidget)> pWgt(nullptr, delWidget);
	
	if (...)
	{
		pWgt.reset(new Widget(std::forward<Ts>(params)...));
	}
	else
	{
		pWgt.reset(new OtherWidget(std::forward<Ts>(params)...));
	}
	
	return pWgt;
}

// unique_ptr can be implicitly converted to shared_ptr


// 19. USE std::shared_ptr FOR SHARED OWNERSHIP RESOURCE MANAGEMENT

// Unlike unique_ptr shared_ptr takes a custom deleter as an argument to the contructor and it is not a part of the type

// std::shared_ptr is two pointers in size, one to the object and one to the control block which contains reference count, weak count 
// and stuff like custom deleter and allocator


// 19. USE std::weak_pptr FOR std::shared_ptr LIKE POINTERS THAT CAN DANGLE

// Can be useful for caching e.g.

std::shared_ptr<const Widget> fastLoadWidget(WidgetId id)
{
	static std::unordered_map<WidgetId, std::weak_ptr<const Widget>> cache;
	
	auto objPtr = cache[id].lock();	// Creates shared_ptr, executed atomically
	
	if (!objPtr)
	{
		objPtr = loadWidget(id);
		cache[id] = objPtr;
	}
	
	return objPtr;
}	// NOTE: this would still need equality-comparison functions and getting rid of expired Widgets which are no longer in use

// Can also be used for observer lists


// PREFER std::make_unique AND std::make_shared TO DIRECT USE OF NEW

// This mainly provide speed and size benefit and better exception safety
// Can't be used with custom deleters and custom memory management
// There's also allocate_shared to be used with custom allocators


// WHEN USING THE Pimpl IDIOM, DEFINE SPECIAL MEMBER FUNCTIONS IN THE IMPLEMENTATION FILE

// widget.h
class Widget 
{
public:
	Widget();
	~Widget();
	
private:
	struct Impl;
	std::unique_ptr<Impl> pImpl;
};

// widget.cpp
struct Widget::Impl
{
	// ...
}

Widget::Widget() :
	pImpl(std::make_unique<Impl>())
{}

// To make it work define destrucor and copy and move (unique_ptr) in cpp

Widget::~Widget() = default;

Widget::Widget(Widget&& rhs) = default;
Widget& Widget::operator=(Widget& rhs) = default;

//Create deep copy
Widget::Widget(const Widget& rhs) :
	pImpl(std::make_unique<Impl>(*rhs.pImpl))
{}

Widget& Widget::operator=(const Widget& rhs)
{
	*pImpl = *rhs.pImpl;
	return this;
}


/*
================
 Rvalue References, Move Semantics, and Perfect Forwarding
 
 Move Semantics make it possible for the compilers to replace expensive copy operations with cheaper (usually) move operations.
 Perfect forwarding makes it possible to create function template that take arbitraty arguments and forward them to the target funcitons. 
 
 TODO: review this entire chapter
================
*/


// UNDERSTAND std::move AND std::forward

// Both std::move and std::forward are casts. The first being unconditional and the latter one being conditional.


// DISTINGUISH UNIVERSAL REFERENCES FROM RVALUE REFERENCES


// USE std::move ON RVALUE REFERENCES, std::forward ON UNIVERSAL REFERENCES


// AVOID OVERLOADING ON UNIVERSAL REFERENCES


// FAMILIARIZE YOURSELF WITH ALTERNATIVES TO OVERLOADING ON UNIVERSAL REFERENCES


// UNDERSTAND REFERENCE COLLAPSING


// ASSUME THAT MOVE OPERATIONS ARE NOT PRESENT, NOT CHEAP, AND NOT USED


// FAMILIARIZE YOURSELF WITH PERFECT FORWARDING FAILURE CASES


/*
================
 Lambda Expressions
 
 Lambdas are very convenient tool to use in C++11 which make many things much easier.
 A lambda expression is and expression following [].
 A closure is the runtime object created by the lambda. Depending on the capture mode closures hold
 copies or references to captured data.
 A closure class is a class from which the closure is instantiated.
================
*/

// 31. AVOID DEFAULT CAPTURE MODES

// Default by reference capture can lead to dangling references


// 32. USE INIT CAPTURE TO MOVE OBJECTS INTO CLOSURES

// C++14 only

// Possible to emulate in C++11 via hand-written classes or bind


// 33. USE decltype ON auto&& PARAMETERS TO std::forward THEM

// C++14 only

// 34. PREFER LAMBDAS TO std::bind


/*
================
 The concurrency API
 
 TODO
================
*/

/*
================
 Tweaks 
================
*/

// 41. CONSIDER PASS BY VALUE FOR COPYABLE PARAMETERS THAT ARE CHEAP TO MOVE AND ALWAYS COPIED


// 42. CONSIDER EMPLACEMENT INSTEAD OF INSERTION

