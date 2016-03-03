## [Ненормальное программирование]

### Использование кодовой базы проекта Chromium в качестве SDK для разработки кроссплатформенных приложений.

**TODO: Добавить ссылок на официальную документацию по проектам
Chromium, GYP, GN, ninja, GTest, whatever и ссылки на инструкции по выкачиванию и сборке Chromium**

Помимо вполне понятной официальной документации ([Chromium Wiki](https://www.chromium.org/developers/how-tos)), 
существует достаточно статей о том, как получить исходный код и собрать проект Chromium ([например](https://habrahabr.ru/post/165193/)).

Так что я не стану останавливаться на этом, будем считать, что у нас уже есть depot_tools в нашем $PATH (нужны утилиты gn и ninja), исходный код получен и готов к сборке. Сборка всего проекта Chromium нам не понадобится, по крайней мере на первом этапе.

Я же хотел рассказать о том, как на основе этого кода можно создавать приложения на %%C++%%,
способные компилироваться и выполняться на нескольких операционных системах и архитектурах.
Конечно, для этой цели уже существуют библиотеки, такие как ((http://www.qt.io Qt)) и ((www.boost.org/ boost)).
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

Давайте начнем с точки входа нашего приложения и базового конфига для системы сборки.

%%(diff)
diff --git a/sample_app/sample_app.cc b/sample_app/sample_app.cc
new file mode 100644
index 0000000..4cce7f6
--- /dev/null
+++ b/sample_app/sample_app.cc
@@ -0,0 +1,3 @@
+int main() {
+  return 0;
+}

diff --git a/sample_app/BUILD.gn b/sample_app/BUILD.gn
new file mode 100644
index 0000000..af23149
--- /dev/null
+++ b/sample_app/BUILD.gn
@@ -0,0 +1,12 @@
+# SampleApp
+
+executable("sample_app") {
+  output_name = "sample_app"
+  sources = [
+    "sample_app.cc",
+  ]
+}
%%

Chromium использует такие инструменты, как [GYP](https://gyp.gsrc.io) и [GN](https://chromium.googlesource.com/chromium/src/tools/gn/) для генерации [ninja](https://ninja-build.org)-файлов, 
описывающих этапы сборки проекта. GN -- это следующий этап развития генератора 
ninja-файлов, он гораздо быстрее GYP, написан на С++ вместо Python и его синтаксис
более дружелюбен для человека. Так что будем исполтьзовать именно его, хотя на 
данный момент Chromium поддерживает сборку и с GYP тоже.
В своем билд-конфиге sample_app/BUILD.gn задаем имя таргета, итоговое имя исполняемого файла и 
перечислим файлы с исходным кодом.
Выглядит вполне понятно, не так ли?
Хотя все небольшие файлы конфигов выглядят понятно, хоть CMake, хоть Makefile =)

**TODO: Приложить patch для правки src/BUILD.gn чтобы прописать туда наш конфиг.**

После добавления нашего таргета в общий билд-конфиг, мы можем собрать наше приложение.
%%(bash)
$ gn gen --args=is_debug=true out/gn
$ ninja -C out/gn sample_app
%%

Давайте добавим консольный вывод и покажем, что как минимум доступна стандартная библиотека С++.

%%(diff)
diff --git a/sample_app/sample_app.cc b/sample_app/sample_app.cc
index 4cce7f6..a5e741b 100644
--- a/sample_app/sample_app.cc
+++ b/sample_app/sample_app.cc
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
%%

Повторив команду для сборки и запустив приложение, мы должны увидеть наше приветствие:
%%(bash)
$ ninja -C out/gn sample_app
$ ./out/gn/sample_app
Hello from SampleApp!
%%

Одним из базовых классов в любом приложении является строка.
Chromium использует класс строки из библиотеки C++, std::basic_string<>, в 
большей степени используются UTF16 строки (base::string16, это typedef для std::basic_string<char16>) 
и легковесный string-view класс base::StringPiece<>.
Давайте попробуем использовать строки и преобразования между разными кодировками.

%%(diff)
diff --git a/sample_app/sample_app.cc b/sample_app/sample_app.cc
index a5e741b..accc0fa 100644
--- a/sample_app/sample_app.cc
+++ b/sample_app/sample_app.cc
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
%%

Команда сборки не меняется, как и билд конфиг.
Как видите, ничего сложного, за всю работу под капотом отвечает библиотека [ICU](http://site.icu-project.org),
которая доступна нам без каких-либо дополнительных действий.

От строк можно перейти к командной строке приложения и ее разбору.
Пожалуйста, учитывайте, что весь код классов в src/base писался ровно под те нужды,
что были у команды Chromium. Если вам покажется странным, что какого-то функционала нет, 
или наоборот, что написан избыточный код, учитывайте это.

%%(diff)
diff --git a/sample_app/sample_app.cc b/sample_app/sample_app.cc
index accc0fa..d2f9a2a 100644
--- a/sample_app/sample_app.cc
+++ b/sample_app/sample_app.cc
@@ -1,6 +1,9 @@
 #include <iostream>
 #include <string>
 
+#include "base/command_line.h"
+#include "base/files/file_path.h"
+#include "base/logging.h"
 #include "base/strings/utf_string_conversions.h"
 
 namespace {
@@ -13,12 +16,25 @@ void StringsSample() {
             << std::endl;
 }
 
+void CommandLineSample() {
+  using base::CommandLine;
+
+  const CommandLine& command_line = *CommandLine::ForCurrentProcess();
+
+  std::cout << "Application program name is "
+            << command_line.GetProgram().AsUTF8Unsafe() << std::endl;
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
%%

%%(diff)
diff --git a/sample_app/sample_app.cc b/sample_app/sample_app.cc
index d2f9a2a..86c1bb7 100644
--- a/sample_app/sample_app.cc
+++ b/sample_app/sample_app.cc
@@ -19,6 +19,9 @@ void StringsSample() {
 void CommandLineSample() {
   using base::CommandLine;
 
+  DCHECK(CommandLine::ForCurrentProcess())
+      << "Command line for process wasn't initialized.";
+
   const CommandLine& command_line = *CommandLine::ForCurrentProcess();
 
   std::cout << "Application program name is "
%%

Здесь продемонстрированы одновременно классы для работы с командной строкой, абстракцией
для файловых путей и немного с библиотекой логгинга.
Так, вызов [CHECK()](https://code.google.com/p/chromium/codesearch#chromium/src/base/logging.h&q=CHECK&sq=package:chromium&type=cs&l=476) проверит результат вызова [CommandLine::Init](https://code.google.com/p/chromium/codesearch#chromium/src/base/command_line.h&q=CommandLine::Init&sq=package:chromium&type=cs&l=80) и в случае неудачи выведет в лог
строку "Failed to parse a command line argument.". При этом в случае успеха 
`operator <<` для потока логирования не будет вызван и накладных затрат на печать не будет.
Это важно, если такое логирование связано с вызовом нетривиальных функций.
Проверка [DCHECK (debug check)](https://code.google.com/p/chromium/codesearch#chromium/src/base/logging.h&q=DCHECK&sq=package:chromium&type=cs&l=663) будет выполнена только в отладочной сборке и не будет 
влиять на выполнение программы в релизе.

**TODO: Больше написать про LOG() и показать в действии.**

Также видно, что есть довольно жесткое, но полезное правило -- на каждую сущность/класс 
есть один файл, имя которого соответствует файлам с кодом. Так, класс FilePath нужно искать
в заголовочном файле [base/files/file_path.h](https://code.google.com/p/chromium/codesearch#chromium/src/base/files/file_path.h&q=base/files/file_path.cc&sq=package:chromium&type=cs), а его реализация находится в [base/files/file_path.cc](https://code.google.com/p/chromium/codesearch#chromium/src/base/files/file_path.cc&sq=package:chromium&type=cs).
Это очень облегчает навигацию по коду и поиск нужных классов/функций.

Давайте рассмотрим более сложный код, например, для перечисления содержимого текущей директории.

%%(diff)
diff --git a/sample_app/sample_app.cc b/sample_app/sample_app.cc
index 86c1bb7..8bd3321 100644
--- a/sample_app/sample_app.cc
+++ b/sample_app/sample_app.cc
@@ -2,7 +2,9 @@
 #include <string>
 
 #include "base/command_line.h"
+#include "base/files/file_enumerator.h"
 #include "base/files/file_path.h"
+#include "base/files/file_util.h"
 #include "base/logging.h"
 #include "base/strings/utf_string_conversions.h"
 
@@ -28,6 +30,24 @@ void CommandLineSample() {
             << command_line.GetProgram().AsUTF8Unsafe() << std::endl;
 }
 
+void FilesSample() {
+  base::FilePath current_dir;
+  CHECK(base::GetCurrentDirectory(&current_dir));
+
+  std::cout << "Enumerating files and directories in path: "
+            << current_dir.AsUTF8Unsafe() << std::endl;
+
+  base::FileEnumerator file_enumerator(
+      current_dir, false, // not recursive
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
@@ -38,6 +58,7 @@ int main(int argc, const char* argv[]) {
 
   StringsSample();
   CommandLineSample();
+  FilesSample();
 
   return 0;
 }
%%

Как видно, использование класса [base::FileEnumerator](https://code.google.com/p/chromium/codesearch#chromium/src/base/files/file_enumerator.h&q=base::FileEnumerator&sq=package:chromium&type=cs&l=40) не представляет особого труда, и в результате мы смогли получить список файлов в текущей директории:

%%(bash)
$ ninja -C out/gn sample_app
$ (cd sample_app/ && ../out/gn/sample_app)
Hello from SampleApp!
This is a wide string.
This is an UTF8 string.
This is an UTF8 string converted to UTF16 and back.
Application program name is ../out/gn/sample_app
Enumerating files and directories in path: /Users/iceman/work/chromium/src/sample_app
[file] /Users/iceman/work/chromium/src/sample_app/BUILD.gn
[file] /Users/iceman/work/chromium/src/sample_app/sample_app.cc
%%

Обычно программа состоит не только из файла main.cc, так что давайте добавим 
самостоятельный модуль для некоего API в наш проект. Не так важна сейчас 
суть кода в новом модуле, это же демонстрация, можно всегда возвращать true к примеру.
Создадим заголовочный файл и файл с реализацией нашей новой функции:

%%(diff)
diff --git a/sample_app/sample_api.cc b/sample_app/sample_api.cc
new file mode 100644
index 0000000..0f8ac4d
--- /dev/null
+++ b/sample_app/sample_api.cc
@@ -0,0 +1,9 @@
+#include "sample_app/sample_api.h"
+
+namespace sample_api {
+
+bool CallApiFunction() {
+  return true;
+}
+
+}  // namespace sample_api
diff --git a/sample_app/sample_api.h b/sample_app/sample_api.h
new file mode 100644
index 0000000..26b39cb
--- /dev/null
+++ b/sample_app/sample_api.h
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
%%

После этого можно написать юнит-тесты на нашу функцию.
Сделаем это и добавим новые файлы в наш проект.
%%(diff)
diff --git a/sample_app/BUILD.gn b/sample_app/BUILD.gn
index af23149..a37e569 100644
--- a/sample_app/BUILD.gn
+++ b/sample_app/BUILD.gn
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
   ]
 
   deps = [
     "//base",
   ]
 }
+
+test("sample_app_unittests") {
+  sources = [
+    # TODO: Extract these API files as a library.
+    "sample_api.cc",
+    "sample_api.h",
+    "sample_api_unittest.cc",
+  ]
+
+  deps = [
+    "//base/test:run_all_unittests",
+    "//testing/gtest",
+  ]
+}
diff --git a/sample_app/sample_api_unittest.cc b/sample_app/sample_api_unittest.cc
new file mode 100644
index 0000000..910b368
--- /dev/null
+++ b/sample_app/sample_api_unittest.cc
@@ -0,0 +1,15 @@
+#include "sample_app/sample_api.h"
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
%%

Использовать библиотеку GTest довольно несложно, но нужно добавить в проект 
зависимость "//testing/gtest", а для удобства еще и "//base/test:run_all_unittests".
Это избавит нас от необходимости писать код для запуска тестов проекта, 
за это будет отвечать код в src/base/test/run_all_unittests.cc.

Перегенерируем ninja файлы для проекта и соберем наши тесты:
%%(bash)
$ ninja -C out/gn sample_app_unittests
%%

Запустим тесты:
%%(bash)
$ ./out/gn/sample_app_unittests 
IMPORTANT DEBUGGING NOTE: batches of tests are run inside their
own process. For debugging a test inside a debugger, use the
--gtest_filter=<your_test_name> flag along with
--single-process-tests.
Using sharding settings from environment. This is shard 0/1
Using 8 parallel jobs.
[1/1] SampleApi.ApiFunctionTest (0 ms)
SUCCESS: all tests passed.
Tests took 0 seconds.
%%

Отлично, все тесты прошли!
После того, как тесты написаны, а код нашего API добавлен в проект, можно его использовать.
%%(diff)
diff --git a/sample_app/sample_app.cc b/sample_app/sample_app.cc
index 8bd3321..07bf9d2 100644
--- a/sample_app/sample_app.cc
+++ b/sample_app/sample_app.cc
@@ -7,6 +7,7 @@
 #include "base/files/file_util.h"
 #include "base/logging.h"
 #include "base/strings/utf_string_conversions.h"
+#include "sample_app/sample_api.h"
 
 namespace {
 
@@ -48,6 +49,12 @@ void FilesSample() {
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
@@ -59,6 +66,7 @@ int main(int argc, const char* argv[]) {
   StringsSample();
   CommandLineSample();
   FilesSample();
+  UseSampleAPI();
 
   return 0;
 }
%%

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
