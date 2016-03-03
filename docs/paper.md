## [Ненормальное программирование]

### Использование кодовой базы проекта Chromium в качестве SDK для разработки кроссплатформенных приложений.

Помимо вполне понятной официальной документации ([Chromium Wiki](https://www.chromium.org/developers/how-tos)), 
существует достаточно статей о том, как получить исходный код и собрать проект Chromium ([например](https://habrahabr.ru/post/165193/)).

Я же хотел рассказать о том, как на основе этого кода можно создавать приложения на C++,
способные компилироваться и выполняться на нескольких операционных системах и архитектурах.
Конечно, для этой цели уже существуют библиотеки, такие как [Qt](http://www.qt.io) и [boost](www.boost.org/).
Но именно поэтому данная статья относится к разделу 'ненормальное программирование', 
никто всерьез не рассматривает код Chromium как основу для кроссплатформенного приложения.

Однако, если задуматься, то становится очевидно, что это вполне возможно, и даже не очень сложно.
Ведь в проекте Chromium есть система сборки, с помощью которой собирается как сам проект, так и все 
нужные зависимости. Такие библиотеки, как [boringssl](https://boringssl.googlesource.com/boringssl/), [ffmpeg](https://www.ffmpeg.org), [freetype2](http://www.freetype.org), [hunspell](https://hunspell.github.io), [ICU](http://site.icu-project.org), [jsoncpp](https://github.com/open-source-parsers/jsoncpp), 
[libjpeg](http://libjpeg.sourceforge.net), libxml, [openh264](http://www.openh264.org), [protobuf](https://developers.google.com/protocol-buffers/), [gtest](https://github.com/google/googletest), [sqlite](https://www.sqlite.org) и конечно, [v8](https://developers.google.com/v8/) поставляются, обновляются и несложно подключаются для использования.
Командой Chromium написаны такие компоненты, как логирование, строки и интернационализация,
работа с ресурсами приложения (строки, изображения, бинарные данные), работа с сетью и файлами,
графика, в том числе 3D, IPC, UI фреймворк для нескольких платформ, и много чего еще.
Все это покрыто большим (хотя и не стопроцентным) количеством тестов, включая performance тесты.

Еще раз повторю, я не пытаюсь доказать, что это лучше любой знакомой вам библиотеки, 
быстрее, удобнее, или просто очистит вашу карму. Нет, это просто эксперимент и 
в некоторой степени рассказ о том, из чего состоит такой сложный и большой проект,
как современный браузер.

Так что я решил показать, как можно создать небольшое приложение,
демонстрирующее работу с некоторыми сущностями из [базовой библиотеки Chromium](https://code.google.com/p/chromium/codesearch#chromium/src/base/).
Если данный материал покажется интересным, можно будет подробнее разобрать работу с
сетью, графикой, UI и прочим. Это не справка по имеющемуся API в Chromium, скорее демонстрация
того, как работать с базовыми вещами, которые нужны практически в любой программе.
Нужно учитывать то, что кодовая база постоянно меняется, какие-то части больше подвержены 
измемениям, какие то меньше. Это все же совсем не фиксированное публичное API.

Не стану останавливаться на том, как скачать код и настроить окружение, все это подробно описано в статьях по приведеным ссылкам. Будем считать, что у нас уже есть depot_tools в нашем $PATH (нужны утилиты gn и ninja), исходный код получен и готов к сборке в директории chromium/. Сборка всего проекта Chromium нам не понадобится, по крайней мере на первом этапе.

Создадим в chromium/src директории нашего приложения, sample_app и sample_app/src.
В sample_app/src будет размещен код приложения, а все команды я буду приводить относительно текущей директории chromium/src/sample_app.

Чтобы получить сразу весь код приложения из статьи, можно склонировать репозиторий
https://github.com/dreamer-dead/chromium-sample-app.git

```bash
$ pwd
/Users/username/chromium/src

$ git clone https://github.com/dreamer-dead/chromium-sample-app.git sample_app

$ cd sample_app/
```

Давайте начнем с точки входа нашего приложения и базового конфига для системы сборки.

```diff
diff --git a/src/sample_app.cc b/src/sample_app.cc
new file mode 100644
index 0000000..4cce7f6
--- /dev/null
+++ b/src/sample_app.cc
@@ -0,0 +1,3 @@
+int main() {
+  return 0;
+}

diff --git a/src/BUILD.gn b/src/BUILD.gn
new file mode 100644
index 0000000..b2fbea5
--- /dev/null
+++ b/src/BUILD.gn
@@ -0,0 +1,8 @@
+# SampleApp
+
+executable("sample_app") {
+  output_name = "sample_app"
+  sources = [
+    "sample_app.cc",
+  ]
+}
```

Chromium использует такие инструменты, как [GYP](https://gyp.gsrc.io) и [GN](https://chromium.googlesource.com/chromium/src/tools/gn/) для генерации [ninja](https://ninja-build.org)-файлов, 
описывающих этапы сборки проекта. GN -- это следующий этап развития генератора 
ninja-файлов, он гораздо быстрее GYP, написан на С++ вместо Python и его синтаксис
более дружелюбен для человека. Так что будем использовать именно его, хотя на 
данный момент Chromium поддерживает сборку и с GYP тоже.
В своем билд-конфиге sample_app/src/BUILD.gn задаем имя таргета, итоговое имя исполняемого файла и 
перечислим файлы с исходным кодом.
Выглядит вполне понятно, не так ли?
Хотя все небольшие файлы конфигов выглядят понятно, хоть CMake, хоть Makefile =)

Для того, чтобы GN увидел конфиг нашего проекта, нужно сослаться на него в корневом файле 
chromium/src/BUILD.gn, наложив такой патч
```diff
diff --git a/BUILD.gn b/BUILD.gn
index ce698a7..67f8219 100644
--- a/BUILD.gn
+++ b/BUILD.gn
@@ -886,3 +886,7 @@ group("chromium_builder_perf") {
     }
   }
 }
+
+group("sample_app") {
+  deps = [ "//sample_app/src:sample_app" ]
+}
```

Если был сделан чекаут репозитория, то для этого можно выполнить
```bash
$ (cd .. && git apply sample_app/src/root_BUILD_gn.patch)
```

После добавления нашего таргета в общий билд-конфиг, мы можем собрать наше приложение.
```bash
$ gn gen --args=is_debug=true --root=../ ../out/gn
$ ninja -C ../out/gn sample_app
```

Таким образом, мы указали, что собираем отладочную сборку, генерируем ninja-файлы сборки в 
директории chromium/src/out/gn/ и корневой билд-конфиг располагается в chromium/src/

Давайте добавим консольный вывод в наше приложение и покажем, что как минимум нам доступна стандартная библиотека С++.

```diff
diff --git a/src/sample_app.cc b/src/sample_app.cc
index 4cce7f6..a5e741b 100644
--- a/src/sample_app.cc
+++ b/src/sample_app.cc
@@ -1,3 +1,8 @@
-int main() {
+#include <iostream>
+#include <string>
+
+int main(int argc, const char* argv[]) {
+  std::cout << "Hello from SampleApp!" << std::endl;
+
   return 0;
 }
```

Повторив команду для сборки и запустив приложение, мы должны увидеть наше приветствие:
```bash
$ ninja -C ../out/gn sample_app
$ ../out/gn/sample_app
Hello from SampleApp!
```

Одним из базовых классов в любом приложении является строка.
Chromium использует класс строки из библиотеки C++, std::basic_string<>, в 
большей степени используются UTF16 строки ([base::string16](https://code.google.com/p/chromium/codesearch#chromium/src/base/strings/string16.h&q=base::string16&sq=package:chromium&type=cs&l=135), это typedef для std::basic_string<char16>) и легковесный string-view класс [base::StringPiece](https://code.google.com/p/chromium/codesearch#chromium/src/base/strings/string_piece.h&sq=package:chromium&type=cs&l=163).
Давайте попробуем использовать строки и преобразования между разными кодировками.

```diff
diff --git a/src/BUILD.gn b/src/BUILD.gn
index b2fbea5..9f6d5c3 100644
--- a/src/BUILD.gn
+++ b/src/BUILD.gn
@@ -5,4 +5,8 @@ executable("sample_app") {
   sources = [
     "sample_app.cc",
   ]
+
+  deps = [
+    "//base"
+  ]
 }

diff --git a/src/sample_app.cc b/src/sample_app.cc
index a5e741b..accc0fa 100644
--- a/src/sample_app.cc
+++ b/src/sample_app.cc
@@ -1,8 +1,24 @@
 #include <iostream>
 #include <string>
 
+#include "base/strings/utf_string_conversions.h"
+
+namespace {
+
+void StringsSample() {
+  std::cout << base::WideToUTF8(L"This is a wide string.") << std::endl;
+  std::wcout << base::UTF8ToWide("This is an UTF8 string.") << std::endl;
+  std::cout << base::UTF16ToUTF8(base::UTF8ToUTF16(
+                   "This is an UTF8 string converted to UTF16 and back."))
+            << std::endl;
+}
+
+}  // namespace
+
 int main(int argc, const char* argv[]) {
   std::cout << "Hello from SampleApp!" << std::endl;
 
+  StringsSample();
+
   return 0;
 }
```

Мы добавили нужную нам теперь зависимость от таргета //base в BUILD.gn и смогли использовать нужные функции.
Как видите, ничего сложного, за всю работу под капотом отвечает библиотека [ICU](http://site.icu-project.org),
которая доступна нам без каких-либо дополнительных действий.
Команда для сборки не меняется,
```bash
$ ninja -C ../out/gn sample_app
```
Ninja автоматически перестроит файлы при изменении .gn конфига.

От строк можно перейти к командной строке приложения и ее разбору.
Пожалуйста, учитывайте, что весь код классов в src/base писался ровно под те нужды,
что были у команды Chromium. Если вам покажется странным, что какого-то функционала нет, 
или наоборот, что написан избыточный код, учитывайте это.

```diff
diff --git a/src/sample_app.cc b/src/sample_app.cc
index accc0fa..acfbb79 100644
--- a/src/sample_app.cc
+++ b/src/sample_app.cc
@@ -1,6 +1,9 @@
 #include <iostream>
 #include <string>
 
+#include "base/command_line.h"
+#include "base/files/file_path.h"
+#include "base/logging.h"
 #include "base/strings/utf_string_conversions.h"
 
 namespace {
@@ -13,12 +16,37 @@ void StringsSample() {
             << std::endl;
 }
 
+void CommandLineSample() {
+  using base::CommandLine;
+
+  DCHECK(CommandLine::ForCurrentProcess())
+      << "Command line for process wasn't initialized.";
+
+  const CommandLine& command_line = *CommandLine::ForCurrentProcess();
+
+  std::cout << "Application program name is "
+            << command_line.GetProgram().AsUTF8Unsafe() << std::endl;
+
+  if (command_line.HasSwitch("bool-switch")) {
+    std::cout << "Detected a boolean switch!" << std::endl;
+  }
+
+  std::string string_switch = command_line.GetSwitchValueASCII("string-switch");
+  if (!string_switch.empty()) {
+    std::cout << "Got a string switch value: " << string_switch << std::endl;
+  }
+}
+
 }  // namespace
 
 int main(int argc, const char* argv[]) {
+  CHECK(base::CommandLine::Init(argc, argv))
+      << "Failed to parse a command line argument.";
+
   std::cout << "Hello from SampleApp!" << std::endl;
 
   StringsSample();
+  CommandLineSample();
 
   return 0;
 }

```

Теперь можем запустить собранную программу с ключами и посмотреть на вывод:
```bash
$ ../out/gn/sample_app --bool-switch --string-switch=SOME_VALUE
Hello from SampleApp!
This is a wide string.
This is an UTF8 string.
This is an UTF8 string converted to UTF16 and back.
Application program name is ../out/gn/sample_app
Detected a boolean switch!
Got a string switch value: SOME_VALUE
```

Здесь продемонстрированы одновременно классы для работы с командной строкой, абстракцией
для файловых путей и немного с библиотекой логгинга.
Так, вызов [CHECK()](https://code.google.com/p/chromium/codesearch#chromium/src/base/logging.h&q=CHECK&sq=package:chromium&type=cs&l=476) проверит результат вызова [CommandLine::Init](https://code.google.com/p/chromium/codesearch#chromium/src/base/command_line.h&q=CommandLine::Init&sq=package:chromium&type=cs&l=80) и в случае неудачи выведет в лог строку "Failed to parse a command line argument." и завершит приложение.
При этом в случае успеха `operator <<` для потока логирования не будет вызван и накладных затрат на печать не будет. Это важно, если такое логирование связано с вызовом нетривиальных функций.

Проверка [DCHECK (debug check)](https://code.google.com/p/chromium/codesearch#chromium/src/base/logging.h&q=DCHECK&sq=package:chromium&type=cs&l=663) будет выполнена только в отладочной сборке и не будет 
влиять на выполнение программы в релизе.

Продолжая говорить о логе программы, рассмотрим следующий код, включающий и использующий логирование

```diff
diff --git a/src/sample_app.cc b/src/sample_app.cc
index acfbb79..76aa0c6 100644
--- a/src/sample_app.cc
+++ b/src/sample_app.cc
@@ -37,6 +37,31 @@ void CommandLineSample() {
   }
 }
 
+void LoggingSample() {
+  logging::LoggingSettings settings;
+
+  // Set log to STDERR on POSIX or to OutputDebugString on Windows.
+  settings.logging_dest = logging::LOG_TO_SYSTEM_DEBUG_LOG;
+  CHECK(logging::InitLogging(settings));
+
+  // Log messages visible by default.
+  LOG(INFO) << "This is INFO log message.";
+  LOG(WARNING) << "This is WARNING log message.";
+
+  // Verbose log messages, disabled by default.
+  VLOG(1) << "This is a log message with verbosity == 1";
+  VLOG(2) << "This is a log message with verbosity == 2";
+
+  // Verbose messages, can be enabled only in debug build.
+  DVLOG(1) << "This is a DEBUG log message with verbosity == 1";
+  DVLOG(2) << "This is a DEBUG log message with verbosity == 2";
+
+  // FATAL log message will terminate our app.
+  if (base::CommandLine::ForCurrentProcess()->HasSwitch("log-fatal")) {
+    LOG(FATAL) << "Program will terminate now!";
+  }
+}
+
 }  // namespace
 
 int main(int argc, const char* argv[]) {
@@ -47,6 +72,7 @@ int main(int argc, const char* argv[]) {
 
   StringsSample();
   CommandLineSample();
+  LoggingSample();
 
   return 0;
 }
```

Здесь мы во-первых, инициализируем подсистему логирования для записи в STDERR, а затем 
выводим сообщения в лог с разными уровнями.
Последнее же сообщение с уровнем FATAL завершит выполнение программы, и выведет стек-трейс, если сможет.
Проверим работу программы с заданным уровнем логирования (приведен вывод на Mac OS X):

```bash
$ ../out/gn/sample_app --v=2 --log-fatal
Hello from SampleApp!
This is a wide string.
This is an UTF8 string.
This is an UTF8 string converted to UTF16 and back.
Application program name is ../out/gn/sample_app
[0303/202541:INFO:sample_app.cc(51)] This is INFO log message.
[0303/202541:WARNING:sample_app.cc(52)] This is WARNING log message.
[0303/202541:VERBOSE1:sample_app.cc(55)] This is a log message with verbosity == 1
[0303/202541:VERBOSE2:sample_app.cc(56)] This is a log message with verbosity == 2
[0303/202541:VERBOSE1:sample_app.cc(59)] This is a DEBUG log message with verbosity == 1
[0303/202541:VERBOSE2:sample_app.cc(60)] This is a DEBUG log message with verbosity == 2
[0303/202541:FATAL:sample_app.cc(64)] Program will terminate now!
0   sample_app                          0x000000010f276def _ZN4base5debug10StackTraceC2Ev + 47
1   sample_app                          0x000000010f276f93 _ZN4base5debug10StackTraceC1Ev + 35
2   sample_app                          0x000000010f2b53a0 _ZN7logging10LogMessageD2Ev + 80
3   sample_app                          0x000000010f2b2c43 _ZN7logging10LogMessageD1Ev + 35
4   sample_app                          0x000000010f235072 _ZN12_GLOBAL__N_113LoggingSampleEv + 1346
5   sample_app                          0x000000010f2342e0 main + 288
6   sample_app                          0x000000010f2341b4 start + 52
7   ???                                 0x0000000000000003 0x0 + 3

Trace/BPT trap: 5

$ ../out/gn/sample_app
Hello from SampleApp!
This is a wide string.
This is an UTF8 string.
This is an UTF8 string converted to UTF16 and back.
Application program name is ../out/gn/sample_app
[0303/203145:INFO:sample_app.cc(51)] This is INFO log message.
[0303/203145:WARNING:sample_app.cc(52)] This is WARNING log message.
```

Также видно, что есть довольно жесткое, но полезное правило -- на каждую сущность/класс 
есть один файл, имя которого соответствует файлам с кодом. Так, класс FilePath нужно искать
в заголовочном файле [base/files/file_path.h](https://code.google.com/p/chromium/codesearch#chromium/src/base/files/file_path.h&q=base/files/file_path.cc&sq=package:chromium&type=cs), а его реализация находится в [base/files/file_path.cc](https://code.google.com/p/chromium/codesearch#chromium/src/base/files/file_path.cc&sq=package:chromium&type=cs).
Это очень облегчает навигацию по коду и поиск нужных классов/функций.

Давайте рассмотрим более сложный код, например, для перечисления содержимого текущей директории.

```diff
diff --git a/src/sample_app.cc b/src/sample_app.cc
index 76aa0c6..69aa8a5 100644
--- a/src/sample_app.cc
+++ b/src/sample_app.cc
@@ -2,7 +2,9 @@
 #include <string>
 
 #include "base/command_line.h"
+#include "base/files/file_enumerator.h"
 #include "base/files/file_path.h"
+#include "base/files/file_util.h"
 #include "base/logging.h"
 #include "base/strings/utf_string_conversions.h"
 
@@ -62,6 +64,24 @@ void LoggingSample() {
   }
 }
 
+void FilesSample() {
+  base::FilePath current_dir;
+  CHECK(base::GetCurrentDirectory(&current_dir));
+
+  std::cout << "Enumerating files and directories in path: "
+            << current_dir.AsUTF8Unsafe() << std::endl;
+
+  base::FileEnumerator file_enumerator(
+      current_dir, false,
+      base::FileEnumerator::FILES | base::FileEnumerator::DIRECTORIES);
+  for (base::FilePath name = file_enumerator.Next(); !name.empty();
+       name = file_enumerator.Next()) {
+    std::cout << (file_enumerator.GetInfo().IsDirectory()
+        ? "[dir ] "
+        : "[file] ") << name.AsUTF8Unsafe() << std::endl;
+  }
+}
+
 }  // namespace
 
 int main(int argc, const char* argv[]) {
@@ -73,6 +93,7 @@ int main(int argc, const char* argv[]) {
   StringsSample();
   CommandLineSample();
   LoggingSample();
+  FilesSample();
 
   return 0;
 }
```

Как видно, использование класса [base::FileEnumerator](https://code.google.com/p/chromium/codesearch#chromium/src/base/files/file_enumerator.h&q=base::FileEnumerator&sq=package:chromium&type=cs&l=40) не представляет особого труда, и в результате мы смогли получить список файлов в текущей директории:

```bash
$ ninja -C ../out/gn sample_app
$ (cd src/ && ../../out/gn/sample_app)
Hello from SampleApp!
This is a wide string.
This is an UTF8 string.
This is an UTF8 string converted to UTF16 and back.
Application program name is ../../out/gn/sample_app
[0303/203629:INFO:sample_app.cc(51)] This is INFO log message.
[0303/203629:WARNING:sample_app.cc(52)] This is WARNING log message.
Enumerating files and directories in path: /Users/username/chromium/src/sample_app/src
[file] /Users/username/chromium/src/sample_app/src/BUILD.gn
[file] /Users/username/chromium/src/sample_app/src/sample_app.cc
```

Обычно программа состоит не только из файла main.cc, так что давайте добавим 
самостоятельный модуль для некоего API в наш проект. Не так важна сейчас 
суть кода в новом модуле, это же демонстрация, можно всегда возвращать true к примеру.
Создадим заголовочный файл и файл с реализацией нашей новой функции:

```diff
diff --git a/src/sample_api.cc b/src/sample_api.cc
new file mode 100644
index 0000000..ba0cf1a
--- /dev/null
+++ b/src/sample_api.cc
@@ -0,0 +1,9 @@
+#include "sample_app/src/sample_api.h"
+
+namespace sample_api {
+
+bool CallApiFunction() {
+  return true;
+}
+
+}  // namespace sample_api

diff --git a/src/sample_api.h b/src/sample_api.h
new file mode 100644
index 0000000..26b39cb
--- /dev/null
+++ b/src/sample_api.h
@@ -0,0 +1,11 @@
+#ifndef SAMPLE_APP_SAMPLE_API_H_
+#define SAMPLE_APP_SAMPLE_API_H_
+
+namespace sample_api {
+
+// Do some black magic.
+bool CallApiFunction();
+
+}  // namespace sample_api
+
+#endif  // SAMPLE_APP_SAMPLE_API_H_
```

После этого можно написать юнит-тесты на нашу функцию.
Сделаем это и добавим новые файлы в наш проект.
```diff
diff --git a/src/BUILD.gn b/src/BUILD.gn
index 9f6d5c3..2c392ef 100644
--- a/src/BUILD.gn
+++ b/src/BUILD.gn
@@ -1,12 +1,30 @@
 # SampleApp
 
+import("//testing/test.gni")
+
 executable("sample_app") {
   output_name = "sample_app"
   sources = [
     "sample_app.cc",
+    "sample_api.cc",
+    "sample_api.h",
+  ]
+
+  deps = [
+    "//base",
+  ]
+}
+
+test("sample_app_unittests") {
+  sources = [
+    # TODO: Extract these API files as a library.
+    "sample_api.cc",
+    "sample_api.h",
+    "sample_api_unittest.cc",
   ]
 
   deps = [
-    "//base"
+    "//base/test:run_all_unittests",
+    "//testing/gtest",
   ]
 }

diff --git a/src/sample_api_unittest.cc b/src/sample_api_unittest.cc
new file mode 100644
index 0000000..535ef91
--- /dev/null
+++ b/src/sample_api_unittest.cc
@@ -0,0 +1,15 @@
+#include "sample_app/src/sample_api.h"
+
+#include "testing/gtest/include/gtest/gtest.h"
+
+namespace sample_api {
+
+namespace {
+
+TEST(SampleApi, ApiFunctionTest) {
+  EXPECT_TRUE(CallApiFunction());
+}
+
+}  // namespace
+
+}  // namespace sample_api
```

Использовать библиотеку GTest довольно несложно, но нужно добавить в проект 
зависимость "//testing/gtest", а для удобства еще и "//base/test:run_all_unittests".
Это избавит нас от необходимости писать код для запуска тестов проекта, 
за это будет отвечать код в src/base/test/run_all_unittests.cc.

Перегенерируем ninja файлы для проекта и соберем наши тесты:
```bash
$ ninja -C ../out/gn sample_app_unittests
```

Запустим тесты:
```bash
$ ../out/gn/sample_app_unittests 
IMPORTANT DEBUGGING NOTE: batches of tests are run inside their
own process. For debugging a test inside a debugger, use the
--gtest_filter=<your_test_name> flag along with
--single-process-tests.
Using sharding settings from environment. This is shard 0/1
Using 8 parallel jobs.
[1/1] SampleApi.ApiFunctionTest (0 ms)
SUCCESS: all tests passed.
Tests took 0 seconds.
```

Отлично, все тесты прошли!
После того, как тесты написаны, а код нашего API добавлен в проект, можно его использовать.
```diff
diff --git a/src/sample_app.cc b/src/sample_app.cc
index 69aa8a5..f4ef100 100644
--- a/src/sample_app.cc
+++ b/src/sample_app.cc
@@ -7,6 +7,7 @@
 #include "base/files/file_util.h"
 #include "base/logging.h"
 #include "base/strings/utf_string_conversions.h"
+#include "sample_app/src/sample_api.h"
 
 namespace {
 
@@ -82,6 +83,12 @@ void FilesSample() {
   }
 }
 
+void UseSampleAPI() {
+  if (sample_api::CallApiFunction()) {
+    std::cout << "Magick!" << std::endl;
+  }
+}
+
 }  // namespace
 
 int main(int argc, const char* argv[]) {
@@ -94,6 +101,7 @@ int main(int argc, const char* argv[]) {
   CommandLineSample();
   LoggingSample();
   FilesSample();
+  UseSampleAPI();
 
   return 0;
 }
```

Вот так просто.
В итоге наша программа может работать с разными кодировками, с файловой системой,
выполняет разбор своей командной строки, способна рапортовать в лог об ошибках и 
использует новый код с test coverage приближающийся к 100%.
При этом билд-конфиг проекта очень маленький и читабельный, а компилироваться и исполняться
код может на разных платформах, и без единого #ifdef в нашем коде.
Разве не здорово?

Конечно, чтобы использовать весь арсенал классов, имеющихся в проекте, нужно будет прочитать много 
кода и документации (что иногда одно и то же), или спрашивать в рассылках, потому что никаких особенных 
обучающих материалов, справочников по API и готовых примеров нет.
Более того, кодовая база живая и постоянно меняется, хоть и в лучшую сторону.
Так что повторюсь, все что вы прочли, должно использоваться на свой страх и риск =)
