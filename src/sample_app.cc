#include <iostream>
#include <string>

#include "base/command_line.h"
#include "base/files/file_enumerator.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "sample_app/src/sample_api.h"

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

void LoggingSample() {
  logging::LoggingSettings settings;

  // Set log to STDERR on POSIX or to OutputDebugString on Windows.
  settings.logging_dest = logging::LOG_TO_SYSTEM_DEBUG_LOG;
  CHECK(logging::InitLogging(settings));

  // Log messages visible by default.
  LOG(INFO) << "This is INFO log message.";
  LOG(WARNING) << "This is WARNING log message.";

  // Verbose log messages, disabled by default.
  VLOG(1) << "This is a log message with verbosity == 1";
  VLOG(2) << "This is a log message with verbosity == 2";

  // Verbose messages, can be enabled only in debug build.
  DVLOG(1) << "This is a DEBUG log message with verbosity == 1";
  DVLOG(2) << "This is a DEBUG log message with verbosity == 2";

  // FATAL log message will terminate our app.
  if (base::CommandLine::ForCurrentProcess()->HasSwitch("log-fatal")) {
    LOG(FATAL) << "Program will terminate now!";
  }
}

void FilesSample() {
  base::FilePath current_dir;
  CHECK(base::GetCurrentDirectory(&current_dir));

  std::cout << "Enumerating files and directories in path: "
            << current_dir.AsUTF8Unsafe() << std::endl;

  base::FileEnumerator file_enumerator(
      current_dir, false,
      base::FileEnumerator::FILES | base::FileEnumerator::DIRECTORIES);
  for (base::FilePath name = file_enumerator.Next(); !name.empty();
       name = file_enumerator.Next()) {
    std::cout << (file_enumerator.GetInfo().IsDirectory()
        ? "[dir ] "
        : "[file] ") << name.AsUTF8Unsafe() << std::endl;
  }
}

void UseSampleAPI() {
  if (sample_api::CallApiFunction()) {
    std::cout << "Magick!" << std::endl;
  }
}

}  // namespace

int main(int argc, const char* argv[]) {
  CHECK(base::CommandLine::Init(argc, argv))
      << "Failed to parse a command line argument.";

  std::cout << "Hello from SampleApp!" << std::endl;

  StringsSample();
  CommandLineSample();
  LoggingSample();
  FilesSample();
  UseSampleAPI();

  return 0;
}
