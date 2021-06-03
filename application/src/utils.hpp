#ifndef UTILS_HPP
#define UTILS_HPP

#include "pa5a.hpp"

EXTERN_C_START

//Obtenir un timestamp UNIX en secondes ou en millisecondes
LONGLONG GetTimestamp(BOOL inMlliseconds);

int gettimeofday(struct timeval* tp, struct timezone* tzp);

//Obtenir une structure SYSTEMTIME depuis un timestamp UNIX
SYSTEMTIME UnixTimestampToSystemTime(LONGLONG timestamp);

//Obtenir un timestamp UNIX depuis une structure SYSTEMTIME
LONGLONG SystemTimeToUnixTimestamp(SYSTEMTIME system_time);

//Convertir une chaine de caractère unicode en UTF8
PCHAR encode_UTF8(LPCWCHAR messageUTF16);

VOID CreateConsole();
EXTERN_C_END

#endif // UTILS_HPP