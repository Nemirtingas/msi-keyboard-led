#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#if defined(WIN32) || defined(_WIN32) || defined(__MINGW32__)
	#define __WINDOWS_32__
#elif defined(WIN64) || defined(_WIN64) || defined(__MINGW64__)
	#define __WINDOWS_64__
#endif

#if defined(__WINDOWS_32__) || defined(__WINDOWS_64__)
	#define __WINDOWS__
#endif

#if defined(__linux__)
	#define __LINUX__
#endif

#if defined(__WINDOWS__)

#ifdef __EXPORT_SYMBOLS__
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API __declspec(dllimport)
#endif

#define LOCAL_API

#elif defined(__LINUX__)

#ifdef __EXPORT_SYMBOLS__
#define EXPORT_API __attribute__((visibility ("default")))
#else
#define EXPORT_API __attribute__((visibility ("default")))
#endif

#define LOCAL_API __attribute__((visibility ("internal")))
//#define LOCAL_API __attribute__((visibility ("hidden")))

#endif // defined(__LINUX__)

#ifdef __EXPORT_SYMBOLS__
void LOCAL_API shared_library_load(void);
void LOCAL_API shared_library_unload(void);
#endif

#ifdef __cplusplus

#define SINGLETON(x) \
private:\
	x( x const& ) = delete;\
	x( x && ) = delete;\
	x& operator=( x const& ) = delete;\
	x& operator=( x && ) = delete;\
public:\
	static x& Inst()

#include <string>
#include <cstdint>

constexpr uint32_t _hash(const char *input)
{
  return (*input ? static_cast<uint32_t>(*input) + 33 * _hash(input + 1) : 5381);
}

uint32_t _hash(const std::string &input)
{
  return _hash(input.c_str());
}

#define switchstr(x) switch( _hash(x) )
#define casestr(x) case _hash(x)

struct ichar_traits : public std::char_traits<char>
{
    static bool eq(char c1, char c2) { return tolower(c1) == tolower(c2); }
    static bool ne(char c1, char c2) { return tolower(c1) != tolower(c2); }
    static bool lt(char c1, char c2) { return tolower(c1) <  tolower(c2); }
    static int compare(const char* s1, const char* s2, size_t n)
    {
        while( n-- != 0 )
        {
            if( tolower(*s1) < tolower(*s2) ) return -1;
            if( tolower(*s1) > tolower(*s2) ) return 1;
            ++s1; ++s2;
        }
        return 0;
    }
    static const char* find(const char* s, int n, char a)
    {
        while( n-- > 0 && tolower(*s) != tolower(a) )
        {
            ++s;
        }
        return s;
    }
};

typedef std::basic_string<char, ichar_traits> istring;

#endif // __cplusplus

#endif // UTILS_H_INCLUDED
