#include <iostream>
#include <string>

#include "base/strings/utf_string_conversions.h"

namespace {

void StringsSample() {
  std::cout << base::WideToUTF8(L"This is a wide string.") << std::endl;
  std::wcout << base::UTF8ToWide("This is an UTF8 string.") << std::endl;
  std::cout << base::UTF16ToUTF8(base::UTF8ToUTF16(
                   "This is an UTF8 string converted to UTF16 and back."))
            << std::endl;
}

}  // namespace

int main(int argc, const char* argv[]) {
  std::cout << "Hello from SampleApp!" << std::endl;

  StringsSample();

  return 0;
}
