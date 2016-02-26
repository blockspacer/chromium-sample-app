#include <iostream>
#include <string>

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"

namespace {

void StringsSample() {
  std::cout << base::WideToUTF8(L"This is a wide string.") << std::endl;
  std::wcout << base::UTF8ToWide("This is an UTF8 string.") << std::endl;
  std::cout << base::UTF16ToUTF8(base::UTF8ToUTF16(
                   "This is an UTF8 string converted to UTF16 and back."))
            << std::endl;
}

void CommandLineSample() {
  using base::CommandLine;

  DCHECK(CommandLine::ForCurrentProcess())
      << "Command line for process wasn't initialized.";

  const CommandLine& command_line = *CommandLine::ForCurrentProcess();

  std::cout << "Application program name is "
            << command_line.GetProgram().AsUTF8Unsafe() << std::endl;

  if (command_line.HasSwitch("bool-switch")) {
    std::cout << "Detected a boolean switch!" << std::endl;
  }

  std::string string_switch = command_line.GetSwitchValueASCII("string-switch");
  if (!string_switch.empty()) {
    std::cout << "Got a string switch value: " << string_switch << std::endl;
  }
}

}  // namespace

int main(int argc, const char* argv[]) {
  CHECK(base::CommandLine::Init(argc, argv))
      << "Failed to parse a command line argument.";

  std::cout << "Hello from SampleApp!" << std::endl;

  StringsSample();
  CommandLineSample();

  return 0;
}
