/* add.c

   Demonstrates creating a DLL with an exported function in a flexible and
   elegant way.
*/

#include "library.h"

int ADDCALL Add(int a, int b)
{
    return (a + b);
}