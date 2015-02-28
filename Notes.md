

Serial Numbers
Information on monome grid serial numbers can be found on the community forum post at http://monome.org/community/discussion/18459
"series" (non-vari-bright) have the prefix "m64-" or "m128-" or "m256-" and the old 40h protocol starts with "m40h"
newest vari-bright "mext" protocol have just "m" followed by seven digits.

Strings
Strings are encoded inside the monome in Unicode
See http://playground.arduino.cc/Code/UTF-8
You can handle Unicode http://www.joelonsoftware.com/articles/Unicode.html
In C++ code can declare strings as wchar_t ("wide char") instead of char and use the wcs functions instead of the str functions (for example wcscat and wcslen instead of strcat and strlen). To create a literal UCS-2 string in C code you just put an L before it as so: L"Hello".