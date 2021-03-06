// This is vcl/tests/test_stlfwd.cxx
#include <vcl_functional.h>
#include <vcl_string.h> // C++ specific includes first

#if defined(TEST) && TEST == 1
// STL included later - this does not work with vcl/emulation because of "redefinition of default argument"
#include <vcl_stlfwd.h>
#include <vcl_map.h>
#include <vcl_set.h>
#include <vcl_list.h>

#else
#if defined(TEST) && TEST == 2
// stl included first

#include <vcl_map.h>
#include <vcl_set.h>
#include <vcl_list.h>
#include <vcl_stlfwd.h>

#else
// Normal
#include <vcl_stlfwd.h>

#endif
#endif

void f(vcl_map<int, vcl_string,vcl_less<int> >*,
       vcl_set<int,vcl_less<int> >*,
       vcl_list<int>*
      )
{
}


int test_stlfwd_main(int /*argc*/,char* /*argv*/[])
{
  return 0;
}
